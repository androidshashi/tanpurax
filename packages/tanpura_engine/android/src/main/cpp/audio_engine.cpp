#include "audio_engine.h"
#include <android/log.h>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <vector>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "TanpuraEngine", __VA_ARGS__)

// ------------------------------------------------------------
// Utility
// ------------------------------------------------------------
static float randomFloat(float min, float max)
{
    return min + (float(rand()) / float(RAND_MAX)) * (max - min);
}

// ------------------------------------------------------------
// Scale frequencies (base Sa frequency for each Western note)
// ------------------------------------------------------------
static constexpr float kScaleFrequencies[] = {
    130.81f, // C3
    138.59f, // C#3
    146.83f, // D3 (default)
    155.56f, // D#3
    164.81f, // E3
    174.61f, // F3
    185.00f, // F#3
    196.00f, // G3
    207.65f, // G#3
    220.00f, // A3
    233.08f, // A#3
    246.94f  // B3
};

static constexpr int kNumScales =
    sizeof(kScaleFrequencies) / sizeof(float);

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

/// @brief Recalculate string frequencies based on current scale and first string
void AudioEngine::updateStringFrequencies()
{
    int scaleIdx = currentScale.load();
    if (scaleIdx < 0) scaleIdx = 0;
    if (scaleIdx >= kNumScales) scaleIdx = kNumScales - 1;

    float baseSaFreq = kScaleFrequencies[scaleIdx];
    float baseFreq = baseSaFreq * kFirstStringRatios[currentFirstString];

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

/// @brief Set the first string (Sa) tuning from predefined ratios
/// @param firstStringIndex
void AudioEngine::setFirstString(int firstStringIndex)
{
    if (firstStringIndex < 0)
        firstStringIndex = 0;
    else if (firstStringIndex >= kNumFirstStrings)
        firstStringIndex = kNumFirstStrings - 1;

    currentFirstString = firstStringIndex;
    updateStringFrequencies();
}

/// @brief Set the scale/pitch (base frequency of Sa)
/// @param scaleIndex
void AudioEngine::setScale(int scaleIndex)
{
    if (scaleIndex < 0)
        scaleIndex = 0;
    else if (scaleIndex >= kNumScales)
        scaleIndex = kNumScales - 1;

    currentScale.store(scaleIndex);
    updateStringFrequencies();
}

// ------------------------------------------------------------
// WAV file export (for debugging/sharing audio)
// ------------------------------------------------------------
bool AudioEngine::exportToWav(const char* filePath, float durationSec)
{
    LOGI("Exporting WAV to: %s, duration: %.1f sec", filePath, durationSec);

    const int exportSampleRate = 48000;
    const int numChannels = 2;
    const int totalFrames = static_cast<int>(durationSec * exportSampleRate);
    const int totalSamples = totalFrames * numChannels;

    // Allocate buffer
    std::vector<float> buffer(totalSamples);

    // Reset state for clean export
    float exportPhase[4] = {0, 0, 0, 0};
    float exportEnvelope[4] = {0, 0, 0, 0};
    bool exportRising[4] = {false, false, false, false};
    int exportFramesSincePluck = 0;
    int exportActiveString = 0;
    float exportDetune[4] = {0, 0, 0, 0};

    const float twoPi = 2.0f * M_PI;
    const float intervalSec = pluckIntervalSec.load();
    const int pluckIntervalFrames = static_cast<int>(intervalSec * exportSampleRate);
    const float gain = masterVolume.load();

    // Generate audio
    for (int i = 0; i < totalFrames; i++)
    {
        // Plucking logic
        exportFramesSincePluck++;
        if (exportFramesSincePluck >= pluckIntervalFrames)
        {
            exportRising[exportActiveString] = true;
            exportFramesSincePluck = 0;
            exportActiveString = (exportActiveString + 1) % kNumStrings;
        }

        // Micro detune every ~1.5 sec
        if (i % (exportSampleRate * 3 / 2) == 0)
        {
            for (int s = 0; s < kNumStrings; s++)
            {
                exportDetune[s] = randomFloat(-0.002f, 0.002f);
            }
        }

        float left = 0.0f;
        float right = 0.0f;

        for (int s = 0; s < kNumStrings; s++)
        {
            // Envelope - use same smooth parameters as real-time playback
            const float exportAttackRate = 0.00008f;   // Matches header
            const float exportDecayRate = 0.999985f;   // Matches header
            const float exportSustainLevel = 0.65f;    // Matches header

            if (exportRising[s])
            {
                exportEnvelope[s] += exportAttackRate;
                if (exportEnvelope[s] >= 1.0f)
                {
                    exportEnvelope[s] = 1.0f;
                    exportRising[s] = false;
                }
            }
            else
            {
                exportEnvelope[s] *= exportDecayRate;
                if (exportEnvelope[s] < exportSustainLevel)
                    exportEnvelope[s] = exportSustainLevel;
            }

            // Oscillator
            float freq = stringFreq[s] * (1.0f + exportDetune[s]);
            float phaseInc = twoPi * freq / exportSampleRate;

            float phase = exportPhase[s];
            float brightness = 0.5f + 0.5f * exportEnvelope[s];

            // Rich 12-harmonic content for authentic tanpura sound
            float sample =
                sinf(phase) * 0.30f +
                sinf(2.0f * phase) * 0.25f * brightness +
                sinf(3.0f * phase) * 0.20f * brightness +
                sinf(4.0f * phase) * 0.15f * brightness +
                sinf(5.0f * phase) * 0.12f * brightness +
                sinf(6.0f * phase) * 0.10f * brightness +
                sinf(7.0f * phase) * 0.08f * brightness +
                sinf(8.0f * phase) * 0.06f * brightness +
                sinf(9.0f * phase) * 0.05f * brightness +
                sinf(10.0f * phase) * 0.04f * brightness +
                sinf(11.0f * phase) * 0.03f * brightness +
                sinf(12.0f * phase) * 0.025f * brightness;

            // Jivari
            float jivariInt = 0.12f + 0.15f * exportEnvelope[s];
            float buzz = sample * (1.0f + jivariInt * fabsf(sinf(phase * 3.0f)));
            float shaped = sample + jivariInt * 0.3f * sample * sample * (sample > 0 ? 1.0f : -1.0f);
            sample = buzz * 0.7f + shaped * 0.3f;

            sample *= exportEnvelope[s];

            // Pan
            float pan = stringPan[s];
            float lGain = sqrtf(0.5f * (1.0f - pan));
            float rGain = sqrtf(0.5f * (1.0f + pan));

            left += sample * lGain;
            right += sample * rGain;

            exportPhase[s] += phaseInc;
            if (exportPhase[s] > twoPi)
                exportPhase[s] -= twoPi;
        }

        // Output gain and soft clip
        float outputGain = 0.6f * gain;
        left *= outputGain;
        right *= outputGain;

        if (left > 1.5f) left = 1.0f;
        else if (left < -1.5f) left = -1.0f;
        else left = tanhf(left * 0.8f) * 1.15f;

        if (right > 1.5f) right = 1.0f;
        else if (right < -1.5f) right = -1.0f;
        else right = tanhf(right * 0.8f) * 1.15f;

        buffer[i * 2] = left;
        buffer[i * 2 + 1] = right;
    }

    // Write WAV file
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        LOGI("Failed to open file for writing");
        return false;
    }

    // Convert to 16-bit PCM
    std::vector<int16_t> pcmBuffer(totalSamples);
    for (int i = 0; i < totalSamples; i++)
    {
        float sample = buffer[i];
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        pcmBuffer[i] = static_cast<int16_t>(sample * 32767.0f);
    }

    // WAV header
    int dataSize = totalSamples * sizeof(int16_t);
    int fileSize = 36 + dataSize;

    file.write("RIFF", 4);
    file.write(reinterpret_cast<char*>(&fileSize), 4);
    file.write("WAVE", 4);

    // fmt chunk
    file.write("fmt ", 4);
    int fmtSize = 16;
    file.write(reinterpret_cast<char*>(&fmtSize), 4);
    int16_t audioFormat = 1; // PCM
    file.write(reinterpret_cast<char*>(&audioFormat), 2);
    int16_t channels = numChannels;
    file.write(reinterpret_cast<char*>(&channels), 2);
    file.write(reinterpret_cast<const char*>(&exportSampleRate), 4);
    int byteRate = exportSampleRate * numChannels * 2;
    file.write(reinterpret_cast<char*>(&byteRate), 4);
    int16_t blockAlign = numChannels * 2;
    file.write(reinterpret_cast<char*>(&blockAlign), 2);
    int16_t bitsPerSample = 16;
    file.write(reinterpret_cast<char*>(&bitsPerSample), 2);

    // data chunk
    file.write("data", 4);
    file.write(reinterpret_cast<char*>(&dataSize), 4);
    file.write(reinterpret_cast<char*>(pcmBuffer.data()), dataSize);

    file.close();
    LOGI("WAV export complete: %d frames", totalFrames);
    return true;
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
    // Sequential string plucking (Sa → Sa → Pa → Sa cycle)
    // --------------------------------------------------------
    const float intervalSec = pluckIntervalSec.load();
    const int pluckIntervalFrames = static_cast<int>(intervalSec * sampleRate);

    framesSincePluck += numFrames;
    if (framesSincePluck >= pluckIntervalFrames)
    {
        // Trigger the next string with smooth attack
        stringRising[activeString] = true;
        stringAge[activeString] = 0;

        // Small random timing variation for natural feel
        framesSincePluck = static_cast<int>(randomFloat(-0.015f, 0.015f) * sampleRate);

        // Move to next string in sequence
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
            detuneOffset[s] = randomFloat(-0.002f, 0.002f);
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
            stringTimeOffset[s] = randomFloat(-2.0f, 2.0f);
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
            // -------- Smooth envelope with attack and decay --------
            if (stringRising[s])
            {
                // Smooth attack phase
                stringEnvelope[s] += attackRate;
                if (stringEnvelope[s] >= 1.0f)
                {
                    stringEnvelope[s] = 1.0f;
                    stringRising[s] = false;
                }
            }
            else
            {
                // Slow decay with sustain floor
                stringEnvelope[s] *= decayRate;
                if (stringEnvelope[s] < sustainLevel)
                    stringEnvelope[s] = sustainLevel;
            }

            stringAge[s]++;

            // Phase increment (with micro-detune)
            float freq = stringFreq[s] * (1.0f + detuneOffset[s]);
            float phaseInc = twoPi * freq / sampleRate;

            // -------- Rich harmonic content (12 harmonics for authentic jivari) --------
            float phase = stringPhase[s];

            // Harmonic balance changes with envelope (brighter on attack)
            float brightness = 0.5f + 0.5f * stringEnvelope[s];

            // Real tanpura has very rich harmonics due to jivari (curved bridge)
            // Harmonics decay slower than typical string instruments
            float sample =
                sinf(phase) * 0.30f +                        // Fundamental
                sinf(2.0f * phase) * 0.25f * brightness +    // 2nd - strong
                sinf(3.0f * phase) * 0.20f * brightness +    // 3rd - strong
                sinf(4.0f * phase) * 0.15f * brightness +    // 4th
                sinf(5.0f * phase) * 0.12f * brightness +    // 5th
                sinf(6.0f * phase) * 0.10f * brightness +    // 6th
                sinf(7.0f * phase) * 0.08f * brightness +    // 7th
                sinf(8.0f * phase) * 0.06f * brightness +    // 8th
                sinf(9.0f * phase) * 0.05f * brightness +    // 9th
                sinf(10.0f * phase) * 0.04f * brightness +   // 10th
                sinf(11.0f * phase) * 0.03f * brightness +   // 11th
                sinf(12.0f * phase) * 0.025f * brightness;   // 12th

            // -------- Jivari effect (bridge buzzing) --------
            // Stronger jivari when string is louder (freshly plucked)
            float jivariIntensity = 0.12f + 0.15f * stringEnvelope[s];
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
