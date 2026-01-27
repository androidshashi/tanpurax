#include "audio_engine.h"
#include <android/log.h>
#include <cmath>
#include <cstring>
#include <cstdlib>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "TanpuraEngine", __VA_ARGS__)

// ------------------------------------------------------------
// Utility
// ------------------------------------------------------------
static float randomFloat(float min, float max)
{
    return min + (float(rand()) / float(RAND_MAX)) * (max - min);
}

// ------------------------------------------------------------
// First string ratios (relative to Sa)
// ------------------------------------------------------------
static constexpr float kFirstStringRatios[] = {
    1.0000f,  // Sa
    1.05946f, // Re komal
    1.12246f, // Re shuddh
    1.18921f, // Ga komal
    1.25992f, // Ga shuddh
    1.33484f, // Ma shuddh
    1.41421f, // Ma tivra
    1.49830f, // Pa
    1.58740f, // Dha komal
    1.68179f, // Dha shuddh
    1.78180f, // Ni komal
    1.88775f  // Ni shuddh
};

static constexpr int kNumFirstStrings =
    sizeof(kFirstStringRatios) / sizeof(float);

// ------------------------------------------------------------
// Engine lifecycle
// ------------------------------------------------------------
bool AudioEngine::initialize()
{

    if (engineRunning)
    {
        LOGI("AudioEngine already initialized");
        return true;
    }

    LOGI("AudioEngine initialized");

    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Output)
        ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
        ->setSharingMode(oboe::SharingMode::Exclusive)
        ->setChannelCount(oboe::ChannelCount::Stereo)
        ->setFormat(oboe::AudioFormat::Float)
        ->setCallback(this);
    if (builder.openStream(stream) != oboe::Result::OK)
    {
        LOGI("Failed to open audio stream");
        return false;
    }

    sampleRate = static_cast<float>(stream->getSampleRate());
    stream->requestStart();

    engineRunning = true;
    playing = false;

    return true;
}

void AudioEngine::release()
{

    if (!engineRunning)
        return;

    LOGI("AudioEngine released");

    playing = false;

    if (stream)
    {
        stream->requestStop();
        stream->close();
        stream.reset();
    }

    engineRunning = false;
}

bool AudioEngine::isEngineRunning() const
{
    return engineRunning;
}

// ------------------------------------------------------------
// Playback
// ------------------------------------------------------------
void AudioEngine::play()
{
    if (!engineRunning)
        return;
    playing = true;
}

void AudioEngine::pause()
{
    playing = false;
}

bool AudioEngine::isPlaying() const
{
    return playing;
}

// ------------------------------------------------------------
// Parameters
// ------------------------------------------------------------
void AudioEngine::setTempo(float intervalSec)
{
    if (intervalSec > 0.0f)
    {
        pluckIntervalSec.store(intervalSec);
    }
}

/// @brief Set the master volume
/// @param volume
void AudioEngine::setVolume(float volume)
{
    if (volume < 0.0f)
        volume = 0.0f;
    if (volume > 1.0f)
        volume = 1.0f;
    masterVolume.store(volume);
}

/// @brief Set the first string (Sa) tuning from predefined ratios
/// @param firstStringIndex
void AudioEngine::setFirstString(int firstStringIndex)
{

    if (firstStringIndex < 0)
        firstStringIndex = 0;
    else if (firstStringIndex >= kNumFirstStrings)
        firstStringIndex = kNumFirstStrings - 1;

    constexpr float baseSaFreq = 146.83f; // Sa reference

    float baseFreq =
        baseSaFreq * kFirstStringRatios[firstStringIndex];

    // Typical tanpura layout: Sa – Sa – Pa – Sa
    stringFreq[0] = baseFreq;
    stringFreq[1] = baseFreq;
    stringFreq[2] = baseFreq * 1.5f; // Pa
    stringFreq[3] = baseFreq;

    for (int i = 0; i < kNumStrings; i++)
    {
        stringPhase[i] = 0.0f;
    }
}

// ------------------------------------------------------------
// Soft clipper for clean saturation at high volumes
// ------------------------------------------------------------
static inline float softClip(float x)
{
    // Attempt maximum clean output with tanh saturation
    // Allows 1.5x overdrive before clipping starts
    if (x > 1.5f)
        return 1.0f;
    if (x < -1.5f)
        return -1.0f;
    return tanhf(x * 0.8f) * 1.15f;
}

// ------------------------------------------------------------
// Jivari simulation (bridge buzzing characteristic of tanpura)
// ------------------------------------------------------------
static inline float jivari(float sample, float phase, float intensity)
{
    // Simulate the characteristic "buzzing" from the curved bridge
    // Creates subtle harmonic distortion when string amplitude is high
    float buzz = sample * (1.0f + intensity * fabsf(sinf(phase * 3.0f)));
    // Add subtle waveshaping for that metallic quality
    float shaped = sample + intensity * 0.3f * sample * sample * (sample > 0 ? 1.0f : -1.0f);
    return buzz * 0.7f + shaped * 0.3f;
}

// ------------------------------------------------------------
// Audio callback (REALISTIC TANPURA)
// ------------------------------------------------------------
oboe::DataCallbackResult AudioEngine::onAudioReady(
    oboe::AudioStream *,
    void *audioData,
    int32_t numFrames)
{
    auto *output = static_cast<float *>(audioData);

    // Silence if engine not running or playback paused (STEREO)
    if (!engineRunning || !playing)
    {
        memset(audioData, 0, sizeof(float) * numFrames * 2);
        return oboe::DataCallbackResult::Continue;
    }

    // --------------------------------------------------------
    // Tempo-based sequential string plucking
    // --------------------------------------------------------
    const float intervalSec = pluckIntervalSec.load();
    const int pluckIntervalFrames = static_cast<int>(intervalSec * sampleRate);

    framesSincePluck += numFrames;
    if (framesSincePluck >= pluckIntervalFrames)
    {
        // Pluck the next string in sequence (Sa → Sa → Pa → Sa)
        stringEnvelope[activeString] = 1.0f; // Full attack on pluck

        // Add slight random variation to pluck timing for natural feel
        framesSincePluck = static_cast<int>(randomFloat(-0.02f, 0.02f) * sampleRate);

        // Move to next string
        activeString = (activeString + 1) % kNumStrings;
    }

    // --------------------------------------------------------
    // Slow micro-detune (~1–2 sec)
    // --------------------------------------------------------
    detuneCounter += numFrames;
    if (detuneCounter > sampleRate * 1.5f)
    {
        for (int s = 0; s < kNumStrings; s++)
        {
            detuneOffset[s] = randomFloat(-0.003f, 0.003f);
        }
        detuneCounter = 0;
    }

    // --------------------------------------------------------
    // Slow micro timing drift (~2–3 sec)
    // --------------------------------------------------------
    timingDriftCounter += numFrames;
    if (timingDriftCounter > sampleRate * 2.5f)
    {
        for (int s = 0; s < kNumStrings; s++)
        {
            stringTimeOffset[s] = randomFloat(-3.0f, 3.0f);
        }
        timingDriftCounter = 0;
    }

    const float twoPi = 2.0f * M_PI;
    const float gain = masterVolume.load();

    // --------------------------------------------------------
    // DSP loop
    // --------------------------------------------------------
    for (int i = 0; i < numFrames; i++)
    {
        float left = 0.0f;
        float right = 0.0f;

        for (int s = 0; s < kNumStrings; s++)
        {
            // -------- Envelope decay with sustain floor --------
            stringEnvelope[s] *= decayRate;
            if (stringEnvelope[s] < sustainLevel)
                stringEnvelope[s] = sustainLevel;

            // Phase increment (with micro-detune)
            float freq = stringFreq[s] * (1.0f + detuneOffset[s]);
            float phaseInc = twoPi * freq / sampleRate;

            // -------- Rich harmonic content (7 harmonics) --------
            // Tanpura has strong odd harmonics due to plucked string
            float phase = stringPhase[s];
            float sample =
                sinf(phase) * 0.45f +              // fundamental
                sinf(2.0f * phase) * 0.22f +       // 2nd harmonic
                sinf(3.0f * phase) * 0.15f +       // 3rd harmonic
                sinf(4.0f * phase) * 0.08f +       // 4th harmonic
                sinf(5.0f * phase) * 0.05f +       // 5th harmonic
                sinf(6.0f * phase) * 0.03f +       // 6th harmonic
                sinf(7.0f * phase) * 0.02f;        // 7th harmonic

            // -------- Jivari effect (bridge buzzing) --------
            // Intensity varies with envelope (stronger when louder)
            float jivariIntensity = 0.15f + 0.1f * stringEnvelope[s];
            sample = jivari(sample, phase, jivariIntensity);

            // Apply envelope
            sample *= stringEnvelope[s];

            // -------- Stereo pan (constant-power) --------
            float pan = stringPan[s];
            float lGain = sqrtf(0.5f * (1.0f - pan));
            float rGain = sqrtf(0.5f * (1.0f + pan));

            left += sample * lGain;
            right += sample * rGain;

            // Phase advance with micro timing offset
            stringPhase[s] += phaseInc + (stringTimeOffset[s] * phaseInc * 0.001f);
            if (stringPhase[s] > twoPi)
                stringPhase[s] -= twoPi;
        }

        // -------- Final mix with higher output level --------
        // Increased from 0.25 to 0.6 for louder output
        float outputGain = 0.6f * gain;
        left *= outputGain;
        right *= outputGain;

        // -------- Soft clip for clean limiting --------
        left = softClip(left);
        right = softClip(right);

        // Write interleaved stereo
        output[i * 2] = left;
        output[i * 2 + 1] = right;
    }

    return oboe::DataCallbackResult::Continue;
}
