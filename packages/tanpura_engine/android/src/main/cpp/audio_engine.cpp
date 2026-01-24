#include "audio_engine.h"
#include <android/log.h>
#include <cmath>
#include <cstring>
#include <cstdlib>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "TanpuraEngine", __VA_ARGS__)

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
    LOGI("playing");
    playing = true;
}

void AudioEngine::pause()
{
    if (!engineRunning)
        return;
    LOGI("paused");
    playing = false;
}

bool AudioEngine::isPlaying() const
{
    return playing;
}

void AudioEngine::load_tanpura_sample(const std::vector<float> &samples)
{
    tanpura_sample.load(samples);
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

    playback_rate = kFirstStringRatios[firstStringIndex];
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

    for (int i = 0; i < numFrames; i++)
    {
        float s = tanpura_sample.process(playback_rate);

        // 1️⃣ Jawari-style soft saturation
        s = tanhf(s * 2.5f);

        // 2️⃣ Slow amplitude breathing (ADD HERE 👇)
        breath_phase += 0.00015f; // very slow
        if (breath_phase > 2.0f * M_PI)
            breath_phase -= 2.0f * M_PI;

        float breath = 0.97f + 0.03f * sinf(breath_phase);
        s *= breath;

        // 3️⃣ Proper gain staging
        s *= 1.4f * masterVolume.load();

        // 4️⃣ Stereo width
        float left = s * (1.0f - stereo_width * 0.3f);
        float right = s * (1.0f + stereo_width * 0.3f);

        output[i * 2] = left;
        output[i * 2 + 1] = right;
    }

    return oboe::DataCallbackResult::Continue;
}
