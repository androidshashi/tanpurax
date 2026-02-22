#include "audio_engine.h"
#include <android/log.h>
#include <fstream>
#include <vector>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "TanpuraEngine", __VA_ARGS__)

// ============================================================================
// TANPURA DSP ENGINE – Exact mirror of tanpura-engine.js
// ============================================================================

// Scale frequencies: C3..B3  (matches JS <select id="pitch"> options)
static constexpr float kScaleFreq[] = {
    130.81f, 138.59f, 146.83f, 155.56f, 164.81f, 174.61f,
    185.00f, 196.00f, 207.65f, 220.00f, 233.08f, 246.94f
};
static constexpr int kNumScales = 12;

static constexpr float kOctaveMult[] = { 0.5f, 1.0f, 2.0f };
static constexpr int   kNumOctaves   = 3;

// First-string ratios RELATIVE TO Sa – mirroring JS firstStringRatio exactly.
//
// JS options:  Pa=0.75,  Ma=0.6666,  Ni=0.9375
//
// These are chromatic ratios divided by 2 so the PA string plays BELOW Sa
// (same octave register as the JS engine).  Mapping:
//   idx 0  Sa        = 1.0 / 2      = 0.50000
//   idx 1  re♭       = 1.05946 / 2  = 0.52973
//   idx 2  re        = 1.12246 / 2  = 0.56123
//   idx 3  ga♭       = 1.18921 / 2  = 0.59461
//   idx 4  ga        = 1.25992 / 2  = 0.62996
//   idx 5  Ma  ←──── = 0.66742  ≈  JS 0.6666  ✓
//   idx 6  Ma♯       = 1.41421 / 2  = 0.70711
//   idx 7  Pa  ←──── = 0.74915  ≈  JS 0.75    ✓
//   idx 8  dha♭      = 1.58740 / 2  = 0.79370
//   idx 9  dha       = 1.68179 / 2  = 0.84090
//   idx 10 Ni♭       = 1.78180 / 2  = 0.89090
//   idx 11 Ni  ←──── = 0.94388  ≈  JS 0.9375  ✓
static constexpr float kFirstStringRatio[] = {
    0.50000f,   // sa
    0.52973f,   // reKomal
    0.56123f,   // reShuddh
    0.59461f,   // gaKomal
    0.62996f,   // gaShuddh
    0.66742f,   // maShuddh   ← JS Ma (4th)
    0.70711f,   // maTivra
    0.74915f,   // pa         ← JS Pa (5th)
    0.79370f,   // dhaKomal
    0.84090f,   // dhaShuddh
    0.89090f,   // niKomal
    0.94388f,   // niShuddh   ← JS Ni (7th)
};
static constexpr int kNumFirstStrings = 12;

// ============================================================================
// StringVoice – implementation
// ============================================================================

void StringVoice::trigger(double freq, double gainMod, float pan,
                          bool highShelf, bool lowShelf,
                          float shelfFreq, float shelfGainDb, double sr)
{
    sampleRate = sr;
    phaseInc   = kTwoPi * freq / sr;

    // Fresh oscillator phases on each pluck (JS: new OscillatorNode per pluck)
    sinePhase = 0;
    sawPhase  = 0;

    // ------ Javari bandpass filter ------
    // JS: javariFilter.type = 'bandpass', Q = 3.0
    //     frequency: setValueAtTime(freq*6, time)
    //                exponentialRampToValueAtTime(freq, time + duration/2)
    //     duration/2 = 12/2 = 6 seconds
    javariFreq       = freq * 6.0;
    javariTargetFreq = freq;
    // Per-64-sample compound sweep multiplier:
    //   per-sample = (freq / (freq*6))^(1 / (6*sr)) = (1/6)^(1/(6*sr))
    //   per 64 samples = (1/6)^(64 / (6*sr))
    double totalSamples = 6.0 * sr;
    javariMult64   = pow(1.0 / 6.0, 64.0 / totalSamples);
    javariCount    = 0;
    javariSweeping = true;
    javariFilter.setBandpass((float)javariFreq, 3.0f, (float)sr);
    javariFilter.reset();

    // ------ Per-string EQ ------
    // JS: highshelf/lowshelf per string; SA1/SA2 use peaking+0dB = passthrough
    useEq = highShelf || lowShelf;
    if (highShelf)     eqFilter.setHighShelf(shelfFreq, shelfGainDb, (float)sr);
    else if (lowShelf) eqFilter.setLowShelf(shelfFreq,  shelfGainDb, (float)sr);
    eqFilter.reset();

    // ------ Stereo pan (constant-power, mirrors Web Audio StereoPannerNode) ------
    // Web Audio panner maps pan ∈ [−1, 1] → angle ∈ [0, π/2]
    float angle = (float)((pan + 1.0) * kPi / 4.0);
    panL = cosf(angle);
    panR = sinf(angle);

    // ------ Envelope ------
    // JS: env.gain.setValueAtTime(0, time)
    //     env.gain.linearRampToValueAtTime(0.5 * gainMod, time + 0.8)
    //     env.gain.exponentialRampToValueAtTime(0.001, time + 12.0)
    peakGain   = 0.5 * gainMod;
    attackStep = peakGain / (0.8 * sr);                        // linear ramp
    decayMult  = pow(0.001 / peakGain, 1.0 / (11.2 * sr));    // exp decay (12 − 0.8 = 11.2 s)

    envGain  = 0;
    envState = ATTACK;
}

void StringVoice::processSample(float& outL, float& outR)
{
    if (envState == IDLE) return;

    // Recompute javari coefficients every 64 samples (avoids per-sample sin/cos)
    if (javariSweeping) {
        if (++javariCount >= 64) {
            javariCount = 0;
            javariFreq *= javariMult64;
            if (javariFreq <= javariTargetFreq) {
                javariFreq     = javariTargetFreq;
                javariSweeping = false;
            }
            javariFilter.setBandpass((float)javariFreq, 3.0f, (float)sampleRate);
        }
    }

    // --- Oscillators ---
    // osc1: sine   osc2: sawtooth (−1..1, same as Web Audio 'sawtooth')
    float sine = sinf((float)sinePhase);
    float saw  = (float)(2.0 * sawPhase / kTwoPi - 1.0);

    sinePhase += phaseInc;
    if (sinePhase >= kTwoPi) sinePhase -= kTwoPi;
    sawPhase  += phaseInc;
    if (sawPhase  >= kTwoPi) sawPhase  -= kTwoPi;

    // --- JS signal graph: osc1 → eqFilter,  osc2 → javariFilter → eqFilter ---
    // Both signals are summed at eqFilter's input.
    float javariOut = javariFilter.process(saw);
    float mixed     = sine + javariOut;

    // --- EQ (passthrough for SA1/SA2 since useEq=false) ---
    if (useEq) mixed = eqFilter.process(mixed);

    // --- Envelope ---
    if (envState == ATTACK) {
        envGain += attackStep;
        if (envGain >= peakGain) { envGain = peakGain; envState = DECAY; }
    } else {
        envGain *= decayMult;
        if (envGain < 0.0001) { envGain = 0; envState = IDLE; return; }
    }

    float s = mixed * (float)envGain;
    outL += s * panL;
    outR += s * panR;
}

// ============================================================================
// Helper: trigger string i with JS-mirrored parameters
// ============================================================================
static void doTrigger(int i, StringVoice* v, const float* freq, double sr)
{
    // Exact mirror of JS playString() per-string configuration:
    //   i=0 PA:   highshelf 3000 Hz +5 dB   pan=−0.6  gainMod=0.9
    //   i=1 SA1:  no EQ                      pan=−0.2  gainMod=1.0
    //   i=2 SA2:  no EQ                      pan=+0.2  gainMod=1.0
    //   i=3 BASS: lowshelf  150 Hz  +4 dB   pan=+0.6  gainMod=1.2
    switch (i) {
        case 0: v[0].trigger(freq[0], 0.9, -0.6f, true,  false, 3000.f, 5.f, sr); break;
        case 1: v[1].trigger(freq[1], 1.0, -0.2f, false, false,    0.f, 0.f, sr); break;
        case 2: v[2].trigger(freq[2], 1.0,  0.2f, false, false,    0.f, 0.f, sr); break;
        case 3: v[3].trigger(freq[3], 1.2,  0.6f, false, true,  150.f,  4.f, sr); break;
    }
}

// ============================================================================
// AudioEngine – lifecycle
// ============================================================================

bool AudioEngine::initialize()
{
    if (engineRunning) return true;
    LOGI("Initializing Tanpura Engine (exact JS mirror, oscillator model)");

    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Output)
        ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
        ->setSharingMode(oboe::SharingMode::Exclusive)
        ->setChannelCount(oboe::ChannelCount::Stereo)
        ->setFormat(oboe::AudioFormat::Float)
        ->setCallback(this);

    if (builder.openStream(stream) != oboe::Result::OK) {
        LOGI("Failed to open audio stream");
        return false;
    }

    sampleRate = static_cast<float>(stream->getSampleRate());
    initEngine();

    stream->requestStart();
    engineRunning = true;
    playing       = false;
    LOGI("Engine ready at %.0f Hz", sampleRate);
    return true;
}

void AudioEngine::initEngine()
{
    compressor.init(sampleRate);
    for (int i = 0; i < 4; i++) voices[i].reset();
    updateFrequencies();
    framesSincePluck = 0;
    currentString    = 0;
}

void AudioEngine::release()
{
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

void AudioEngine::play()
{
    if (engineRunning) {
        currentString = 0;
        // Pre-arm so first string fires on the very first audio frame
        framesSincePluck = static_cast<int>(pluckIntervalSec.load() * sampleRate);
        playing = true;
    }
}

void AudioEngine::pause() { playing = false; }
bool AudioEngine::isPlaying() const { return playing; }

// ============================================================================
// Parameters
// ============================================================================

void AudioEngine::setTempo(float sec)    { if (sec > 0) pluckIntervalSec.store(sec); }
void AudioEngine::setVolume(float v)     { masterVolume.store(fmaxf(0.f, fminf(1.f, v))); }
void AudioEngine::setJivari(float v)     { jivariLevel.store(fmaxf(0.f, fminf(1.f, v))); }
int   AudioEngine::getOctave()  const    { return currentOctave.load(); }
float AudioEngine::getJivari()  const    { return jivariLevel.load(); }

void AudioEngine::updateFrequencies()
{
    int scale  = fmaxf(0, fminf(currentScale.load(),  kNumScales     - 1));
    int octave = fmaxf(0, fminf(currentOctave.load(), kNumOctaves    - 1));
    int fs     = fmaxf(0, fminf(currentFirstString,   kNumFirstStrings - 1));

    float sa = kScaleFreq[scale] * kOctaveMult[octave];

    // Mirror JS playString() string frequencies:
    //   i=0 PA:   root * firstStringRatio  (ratio now below-root, matches JS 0.75/0.6666/0.9375)
    //   i=1 SA1:  root * 1.001
    //   i=2 SA2:  root * 0.999
    //   i=3 BASS: root * 0.5
    stringFreq[0] = sa * kFirstStringRatio[fs];
    stringFreq[1] = sa * 1.001f;
    stringFreq[2] = sa * 0.999f;
    stringFreq[3] = sa * 0.5f;

    LOGI("Frequencies → PA=%.2f  SA1=%.2f  SA2=%.2f  BASS=%.2f",
         stringFreq[0], stringFreq[1], stringFreq[2], stringFreq[3]);
}

void AudioEngine::setFirstString(int idx)
{
    currentFirstString = fmaxf(0, fminf(idx, kNumFirstStrings - 1));
    updateFrequencies();
}

void AudioEngine::setScale(int idx)
{
    currentScale.store(fmaxf(0, fminf(idx, kNumScales - 1)));
    updateFrequencies();
}

void AudioEngine::setOctave(int idx)
{
    currentOctave.store(fmaxf(0, fminf(idx, kNumOctaves - 1)));
    updateFrequencies();
}

// ============================================================================
// Audio Callback – exact mirror of JS schedule() + playString()
// ============================================================================

oboe::DataCallbackResult AudioEngine::onAudioReady(
    oboe::AudioStream*, void* audioData, int32_t numFrames)
{
    auto* out = static_cast<float*>(audioData);

    if (!engineRunning || !playing) {
        memset(audioData, 0, sizeof(float) * numFrames * 2);
        return oboe::DataCallbackResult::Continue;
    }

    // JS: gap = 1.3 / tempo  →  pluckIntervalSec stores that value directly
    const int   pluckInt = static_cast<int>(pluckIntervalSec.load() * sampleRate);
    // JS: masterGain.gain.value = 0.8  (default masterVolume = 0.8)
    const float gain     = masterVolume.load();

    for (int i = 0; i < numFrames; i++) {

        // ===== 4-string sequential pluck cycle =====
        // JS: schedule() → idx = (idx + 1) % 4, gap = 1.3/tempo
        if (++framesSincePluck >= pluckInt) {
            framesSincePluck = 0;
            doTrigger(currentString, voices, stringFreq, sampleRate);
            currentString = (currentString + 1) % 4;
        }

        // ===== Sum all 4 voices =====
        float L = 0.f, R = 0.f;
        for (int s = 0; s < 4; s++) voices[s].processSample(L, R);

        // ===== Apply masterGain (JS: masterGain.gain = 0.8) =====
        L *= gain;
        R *= gain;

        // ===== DynamicsCompressor (JS: threshold=−8, ratio=4, attack=0.01, release=0.1) =====
        compressor.process(L, R);

        out[i * 2]     = L;
        out[i * 2 + 1] = R;
    }

    return oboe::DataCallbackResult::Continue;
}

// ============================================================================
// WAV Export – same synthesis pipeline, offline render
// ============================================================================

bool AudioEngine::exportToWav(const char* path, float duration)
{
    LOGI("Exporting: %s  %.1f s", path, duration);

    const int sr     = 48000;
    const int frames = static_cast<int>(duration * sr);

    StringVoice           expVoices[4];
    FeedforwardCompressor expComp;
    expComp.init(sr);

    const int   pluckInt = static_cast<int>(pluckIntervalSec.load() * (float)sr);
    const float gain     = masterVolume.load();

    int fsSincePluck = pluckInt;    // trigger first string immediately
    int curStr       = 0;

    std::vector<int16_t> pcm(frames * 2);

    for (int i = 0; i < frames; i++) {
        if (++fsSincePluck >= pluckInt) {
            fsSincePluck = 0;
            doTrigger(curStr, expVoices, stringFreq, sr);
            curStr = (curStr + 1) % 4;
        }

        float L = 0.f, R = 0.f;
        for (int s = 0; s < 4; s++) expVoices[s].processSample(L, R);

        L *= gain;
        R *= gain;
        expComp.process(L, R);

        // Clamp to [−1, 1] before quantising to int16
        pcm[i * 2]     = (int16_t)(fmaxf(-1.f, fminf(1.f, L)) * 32767.f);
        pcm[i * 2 + 1] = (int16_t)(fmaxf(-1.f, fminf(1.f, R)) * 32767.f);
    }

    // ---- WAV header ----
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;

    const int dataSize = frames * 2 * 2;
    const int fileSize = 36 + dataSize;
    const int byteRate = sr * 4;
    int16_t audioFmt=1, channels=2, blockAlign=4, bitsPerSample=16;
    int     chunkSize = 16;

    f.write("RIFF",4);              f.write((char*)&fileSize,4);
    f.write("WAVE",4);
    f.write("fmt ",4);              f.write((char*)&chunkSize,4);
    f.write((char*)&audioFmt,2);    f.write((char*)&channels,2);
    f.write((char*)&sr,4);          f.write((char*)&byteRate,4);
    f.write((char*)&blockAlign,2);  f.write((char*)&bitsPerSample,2);
    f.write("data",4);              f.write((char*)&dataSize,4);
    f.write((char*)pcm.data(), dataSize);
    f.close();

    LOGI("Export done: %d frames, stereo 48 kHz 16-bit", frames);
    return true;
}
