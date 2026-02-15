#include "audio_engine.h"
#include <android/log.h>
#include <fstream>
#include <vector>
#include <random>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "TanpuraEngine", __VA_ARGS__)

// ============================================================================
// TANPURA DSP ENGINE - Final Verified Implementation
// ============================================================================

// Scale frequencies (Middle octave C3-B3)
static constexpr float kScaleFreq[] = {
    130.81f, 138.59f, 146.83f, 155.56f, 164.81f, 174.61f,
    185.00f, 196.00f, 207.65f, 220.00f, 233.08f, 246.94f
};
static constexpr int kNumScales = 12;

static constexpr float kOctaveMult[] = {0.5f, 1.0f, 2.0f};
static constexpr int kNumOctaves = 3;

// First string ratios (relative to Sa)
static constexpr float kFirstStringRatio[] = {
    1.0f, 1.05946f, 1.12246f, 1.18921f, 1.25992f, 1.33484f,
    1.41421f, 1.49830f, 1.58740f, 1.68179f, 1.78180f, 1.88775f
};
static constexpr int kNumFirstStrings = 12;

// Random generator for tempo fuzz (human-like timing)
static std::mt19937 rng(42);
static std::uniform_int_distribution<int> fuzzDist(48, 240);

// ============================================================================
// Soft limiter for output
// ============================================================================
static inline float softLimit(float x) {
    return tanhf(x * 0.8f) * 1.2f;
}

// ============================================================================
// Engine Initialization
// ============================================================================
bool AudioEngine::initialize() {
    if (engineRunning) return true;

    LOGI("Initializing Final Verified Tanpura DSP Engine");

    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Output)
        ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
        ->setSharingMode(oboe::SharingMode::Exclusive)
        ->setChannelCount(oboe::ChannelCount::Stereo)
        ->setFormat(oboe::AudioFormat::Float)
        ->setCallback(this);

    if (builder.openStream(stream) != oboe::Result::OK) {
        LOGI("Failed to open stream");
        return false;
    }

    sampleRate = static_cast<float>(stream->getSampleRate());

    // Initialize everything
    initEngine();

    stream->requestStart();
    engineRunning = true;
    playing = false;

    LOGI("Tanpura engine ready at %.0f Hz (40 modes, P^1.6 jivari)", sampleRate);
    return true;
}

void AudioEngine::initEngine() {
    // Initialize body resonator
    bodyResonator.init(sampleRate);

    // Initialize excitation generators
    for (int s = 0; s < 4; s++) {
        excitation[s].setSampleRate(sampleRate);
        excitation[s].reset();
    }

    // Update frequencies and initialize DSP
    updateFrequencies();
}

void AudioEngine::release() {
    if (!engineRunning) return;
    playing = false;
    if (stream) {
        stream->requestStop();
        stream->close();
        stream.reset();
    }
    engineRunning = false;
    LOGI("Engine released");
}

bool AudioEngine::isEngineRunning() const { return engineRunning; }
void AudioEngine::play() { if (engineRunning) playing = true; }
void AudioEngine::pause() { playing = false; }
bool AudioEngine::isPlaying() const { return playing; }

// ============================================================================
// Parameters
// ============================================================================
void AudioEngine::setTempo(float sec) {
    if (sec > 0) pluckIntervalSec.store(sec);
}

void AudioEngine::setVolume(float v) {
    masterVolume.store(fmaxf(0, fminf(1, v)));
}

void AudioEngine::setJivari(float v) {
    jivariLevel.store(fmaxf(0, fminf(1, v)));
}

void AudioEngine::updateFrequencies() {
    int scale = fmaxf(0, fminf(currentScale.load(), kNumScales - 1));
    int octave = fmaxf(0, fminf(currentOctave.load(), kNumOctaves - 1));

    float baseSa = kScaleFreq[scale] * kOctaveMult[octave];
    float first = baseSa * kFirstStringRatio[currentFirstString];

    // Tanpura layout: Pa/Ma, Sa, Sa (detuned), Kharaj
    stringFreq[0] = first;
    stringFreq[1] = baseSa;
    stringFreq[2] = baseSa * 1.003f;  // Slight detune for natural beating
    stringFreq[3] = baseSa * 0.5f;    // Kharaj (octave below)

    // Initialize DSP with new frequencies
    dsp.initWithFrequencies(stringFreq[0], stringFreq[1], stringFreq[2], stringFreq[3], sampleRate);
    dsp.reset();

    LOGI("Frequencies: %.1f, %.1f, %.1f, %.1f (40 modes each)",
         stringFreq[0], stringFreq[1], stringFreq[2], stringFreq[3]);
}

void AudioEngine::setFirstString(int idx) {
    currentFirstString = fmaxf(0, fminf(idx, kNumFirstStrings - 1));
    isSaPaMode = (currentFirstString >= 7);
    updateFrequencies();
}

void AudioEngine::setScale(int idx) {
    currentScale.store(fmaxf(0, fminf(idx, kNumScales - 1)));
    updateFrequencies();
}

void AudioEngine::setOctave(int idx) {
    currentOctave.store(fmaxf(0, fminf(idx, kNumOctaves - 1)));
    updateFrequencies();
}

int AudioEngine::getOctave() const { return currentOctave.load(); }
float AudioEngine::getJivari() const { return jivariLevel.load(); }

// ============================================================================
// Audio Callback - Main DSP Processing
// ============================================================================
oboe::DataCallbackResult AudioEngine::onAudioReady(
    oboe::AudioStream*, void* audioData, int32_t numFrames) {

    auto* out = static_cast<float*>(audioData);

    if (!engineRunning || !playing) {
        memset(audioData, 0, sizeof(float) * numFrames * 2);
        return oboe::DataCallbackResult::Continue;
    }

    float interval = pluckIntervalSec.load();
    int pluckInterval = static_cast<int>(interval * sampleRate);
    float gain = masterVolume.load();

    // Map jivari level (0-1) to strength (0.5-2.0)
    float jivari = 0.5f + jivariLevel.load() * 1.5f;
    dsp.setJivari(jivari);

    for (int i = 0; i < numFrames; i++) {
        // ===== PLUCK SCHEDULING (6-beat Darda cycle) =====
        framesSincePluck++;
        if (framesSincePluck >= pluckInterval + fuzzDist(rng)) {
            framesSincePluck = 0;

            // Map beat to string (-1 = rest)
            int stringToPluck = -1;
            switch (currentBeat) {
                case 0: stringToPluck = 0; break;  // Pa/Ma
                case 1: stringToPluck = -1; break; // Rest
                case 2: stringToPluck = 1; break;  // Sa
                case 3: stringToPluck = 2; break;  // Sa (detuned)
                case 4: stringToPluck = 3; break;  // Kharaj
                case 5: stringToPluck = -1; break; // Rest
            }

            if (stringToPluck >= 0) {
                // Trigger raised cosine excitation
                // Duration: 3ms, Position: 0.25 (quarter from bridge)
                excitation[stringToPluck].trigger(1.0, 3.0, 0.25);
            }

            currentBeat = (currentBeat + 1) % 6;
        }

        // ===== APPLY EXCITATION TO DSP =====
        for (int s = 0; s < 4; s++) {
            if (excitation[s].isActive()) {
                double force = excitation[s].generate();
                dsp.pluckString(s, force, excitation[s].getPosition());
            }
        }

        // ===== PROCESS DSP (all 4 strings with shared bridge) =====
        float sample = dsp.processSample();

        // ===== BODY RESONANCE (Miraj gourd) =====
        sample = bodyResonator.process(sample);

        // ===== STEREO OUTPUT =====
        // Simple stereo widening based on string panning
        float left = sample;
        float right = sample;

        // ===== OUTPUT WITH SOFT LIMITING =====
        float outputGain = 4.0f * gain;
        left = softLimit(left * outputGain);
        right = softLimit(right * outputGain);

        out[i * 2] = left;
        out[i * 2 + 1] = right;
    }

    return oboe::DataCallbackResult::Continue;
}

// ============================================================================
// WAV Export
// ============================================================================
bool AudioEngine::exportToWav(const char* path, float duration) {
    LOGI("Exporting: %s, %.1fs", path, duration);

    const int sr = 48000;
    const int frames = static_cast<int>(duration * sr);
    const int samples = frames * 2;

    std::vector<float> buf(samples);

    // Create local DSP instance for export
    TanpuraDSP exportDsp;
    exportDsp.initWithFrequencies(stringFreq[0], stringFreq[1], stringFreq[2], stringFreq[3], sr);
    exportDsp.reset();

    // Local excitation
    RaisedCosineExcitation exportExc[4];
    for (int s = 0; s < 4; s++) {
        exportExc[s].setSampleRate(sr);
        exportExc[s].reset();
    }

    // Local body resonator
    MirajBodyResonator exportBody;
    exportBody.init(sr);

    float interval = pluckIntervalSec.load();
    int pluckInt = static_cast<int>(interval * sr);
    float gain = masterVolume.load();
    float jivari = 0.5f + jivariLevel.load() * 1.5f;

    exportDsp.setJivari(jivari);

    int framesSince = 0;
    int beat = 0;

    for (int i = 0; i < frames; i++) {
        // Pluck scheduling
        framesSince++;
        if (framesSince >= pluckInt + fuzzDist(rng)) {
            framesSince = 0;

            int stringToPluck = -1;
            switch (beat) {
                case 0: stringToPluck = 0; break;
                case 1: stringToPluck = -1; break;
                case 2: stringToPluck = 1; break;
                case 3: stringToPluck = 2; break;
                case 4: stringToPluck = 3; break;
                case 5: stringToPluck = -1; break;
            }

            if (stringToPluck >= 0) {
                exportExc[stringToPluck].trigger(1.0, 3.0, 0.25);
            }

            beat = (beat + 1) % 6;
        }

        // Apply excitation
        for (int s = 0; s < 4; s++) {
            if (exportExc[s].isActive()) {
                double force = exportExc[s].generate();
                exportDsp.pluckString(s, force, exportExc[s].getPosition());
            }
        }

        // Process DSP
        float sample = exportDsp.processSample();
        sample = exportBody.process(sample);

        // Output
        float g = 4.0f * gain;
        float out = softLimit(sample * g);

        buf[i * 2] = out;
        buf[i * 2 + 1] = out;
    }

    // Write WAV
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;

    std::vector<int16_t> pcm(samples);
    for (int i = 0; i < samples; i++) {
        pcm[i] = static_cast<int16_t>(fmaxf(-1, fminf(1, buf[i])) * 32767);
    }

    int dataSize = samples * 2;
    int fileSize = 36 + dataSize;

    f.write("RIFF", 4);
    f.write((char*)&fileSize, 4);
    f.write("WAVE", 4);
    f.write("fmt ", 4);
    int fmt = 16; f.write((char*)&fmt, 4);
    int16_t af = 1; f.write((char*)&af, 2);
    int16_t ch = 2; f.write((char*)&ch, 2);
    f.write((char*)&sr, 4);
    int br = sr * 4; f.write((char*)&br, 4);
    int16_t ba = 4; f.write((char*)&ba, 2);
    int16_t bp = 16; f.write((char*)&bp, 2);
    f.write("data", 4);
    f.write((char*)&dataSize, 4);
    f.write((char*)pcm.data(), dataSize);

    f.close();
    LOGI("Export done: %d frames", frames);
    return true;
}
