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
        pluckIntervalSec = intervalSec;
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
    // Slow micro-detune (~1–2 sec)
    // --------------------------------------------------------
    detuneCounter += numFrames;
    if (detuneCounter > sampleRate * 1.5f)
    {
        for (int s = 0; s < kNumStrings; s++)
        {
            detuneOffset[s] = randomFloat(-0.0025f, 0.0025f);
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
            stringTimeOffset[s] = randomFloat(-3.0f, 3.0f); // samples
        }
        timingDriftCounter = 0;
    }

    // // --------------------------------------------------------
    // // Musical clock (energy refresh)
    // // --------------------------------------------------------
    // framesSincePluck += numFrames;
    // int framesPerRefresh = static_cast<int>(pluckIntervalSec * sampleRate);

    // if (framesSincePluck >= framesPerRefresh)
    // {

    //     stringEnvelope[activeString] += 0.2f;
    //     if (stringEnvelope[activeString] > 1.0f)
    //         stringEnvelope[activeString] = 1.0f;

    //     activeString = (activeString + 1) % kNumStrings;
    //     framesSincePluck = 0;
    // }

    const float twoPi = 2.0f * M_PI;
    // --------------------------------------------------------
    // DSP loop
    // --------------------------------------------------------
    for (int i = 0; i < numFrames; i++)
    {

        float left = 0.0f;
        float right = 0.0f;

        for (int s = 0; s < kNumStrings; s++)
        {

            // Envelope decay (never dies)
            stringEnvelope[s] *= decayRate;
            if (stringEnvelope[s] < sustainLevel)
                stringEnvelope[s] = sustainLevel;

            // ---------- CONTINUOUS APERIODIC ENERGY ----------
            float energy = randomFloat(0.00005f, 0.00015f);
            stringEnvelope[s] += energy;

            if (stringEnvelope[s] > 1.0f)
                stringEnvelope[s] = 1.0f;

            // Phase increment (with micro-detune)
            float phaseInc =
                twoPi *
                (stringFreq[s] * (1.0f + detuneOffset[s])) /
                sampleRate;

            float sample =
                sinf(stringPhase[s]) * 0.6f +
                sinf(2.0f * stringPhase[s]) * 0.25f +
                sinf(3.0f * stringPhase[s]) * 0.15f;

            sample *= stringEnvelope[s];

            // Stereo pan (constant-power)
            float pan = stringPan[s];
            float lGain = sqrtf(0.5f * (1.0f - pan));
            float rGain = sqrtf(0.5f * (1.0f + pan));

            left += sample * lGain;
            right += sample * rGain;

            // Phase advance with micro timing offset
            stringPhase[s] +=
                phaseInc + (stringTimeOffset[s] * phaseInc * 0.001f);

            if (stringPhase[s] > twoPi)
                stringPhase[s] -= twoPi;
        }

        // Normalize
        float gain = masterVolume.load();
        left *= 0.25f * gain;
        right *= 0.25f * gain;

        // Write interleaved stereo
        output[i * 2] = left;
        output[i * 2 + 1] = right;
    }

    return oboe::DataCallbackResult::Continue;
}
