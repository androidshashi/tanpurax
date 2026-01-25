#include "audio_engine.h"
#include <android/log.h>
#include <cmath>
#include <cstring>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "TanpuraEngine", __VA_ARGS__)

// ------------------------------------------------------------
// Scale ratios (absolute pitch: C → B)
// ------------------------------------------------------------
static constexpr float kScaleRatios[12] = {
    1.0000f,  // C
    1.05946f, // C#
    1.12246f, // D
    1.18921f, // D#
    1.25992f, // E
    1.33484f, // F
    1.41421f, // F#
    1.49830f, // G
    1.58740f, // G#
    1.68179f, // A
    1.78180f, // A#
    1.88775f  // B
};

// ------------------------------------------------------------
// First string swara ratios (relative to Sa)
// ------------------------------------------------------------
static constexpr float kFirstStringRatios[12] = {
    1.0000f,  // Sa
    1.05946f, // Re♭
    1.12246f, // Re
    1.18921f, // Ga♭
    1.25992f, // Ga
    1.33484f, // Ma
    1.41421f, // Ma#
    1.49830f, // Pa
    1.58740f, // Dha♭
    1.68179f, // Dha
    1.78180f, // Ni♭
    1.88775f  // Ni
};

// ------------------------------------------------------------
// Engine lifecycle
// ------------------------------------------------------------
bool AudioEngine::initialize()
{
    if (engineRunning)
        return true;

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

    sampleRate = stream->getSampleRate();
    stream->requestStart();

    engineRunning = true;
    playing = false;

    LOGI("AudioEngine initialized (%d Hz)", sampleRate);
    return true;
}

void AudioEngine::release()
{
    if (!engineRunning)
        return;

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
    playing = false;
}

bool AudioEngine::isPlaying() const
{
    return playing;
}

// ------------------------------------------------------------
// Load WAV into all strings
// ------------------------------------------------------------
void AudioEngine::load_tanpura_sample(const std::vector<float> &samples)
{
    for (int i = 0; i < kNumStrings; i++)
        strings[i].load(samples);
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

// Absolute pitch (C, C#, D, ...)
void AudioEngine::setScale(int scaleIndex)
{
    if (scaleIndex < 0)
        scaleIndex = 0;
    if (scaleIndex > 11)
        scaleIndex = 11;

    scale_rate = kScaleRatios[scaleIndex];
    base_rate = scale_rate * first_string_rate;
}

// Swara role (Sa, Re♭, Re, ..., Ni)
void AudioEngine::setFirstString(int swaraIndex)
{
    if (swaraIndex < 0)
        swaraIndex = 0;
    if (swaraIndex > 11)
        swaraIndex = 11;

    first_string_rate = kFirstStringRatios[swaraIndex];
    base_rate = scale_rate * first_string_rate;
}

void AudioEngine::setOctave(int octave)
{
    // octave: -1 = male, 0 = normal, +1 = female
    octave = std::max(-1, std::min(1, octave));
    octave_rate = powf(2.0f, octave);
    base_rate = scale_rate * first_string_rate * octave_rate;
}

// ------------------------------------------------------------
// Audio callback
// ------------------------------------------------------------
oboe::DataCallbackResult AudioEngine::onAudioReady(
    oboe::AudioStream *,
    void *audioData,
    int32_t numFrames)
{
    float *output = static_cast<float *>(audioData);

    if (!engineRunning || !playing)
    {
        memset(output, 0, sizeof(float) * numFrames * 2);
        return oboe::DataCallbackResult::Continue;
    }

    for (int i = 0; i < numFrames; i++)
    {
        float mix = 0.0f;

        // ---- Sa–Pa–Sa–Sa layering ----
        for (int s = 0; s < kNumStrings; s++)
        {
            float rate =
                base_rate *
                string_rate[s] *
                (1.0f + string_detune[s]);

            mix += strings[s].process(rate) * string_gain[s];
        }

        // Jawari presence
        mix = tanhf(mix * 2.2f);

        // Gentle breathing
        breath_phase += 0.00015f;
        if (breath_phase > 2.0f * M_PI)
            breath_phase -= 2.0f * M_PI;

        mix *= (0.97f + 0.03f * sinf(breath_phase));

        // Final gain
        mix *= 1.25f * masterVolume.load();

        // Stereo width
        float left = 0.0f;
        float right = 0.0f;

        for (int s = 0; s < kNumStrings; s++)
        {
            float pan = string_pan[s] * stereo_width;
            float lg = sqrtf(0.5f * (1.0f - pan));
            float rg = sqrtf(0.5f * (1.0f + pan));

            left += mix * lg * 0.25f;
            right += mix * rg * 0.25f;
        }

        output[i * 2] = left;
        output[i * 2 + 1] = right;
    }

    return oboe::DataCallbackResult::Continue;
}
