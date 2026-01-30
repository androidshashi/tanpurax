#include "audio_engine.h"
#include <android/log.h>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <vector>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "TanpuraEngine", __VA_ARGS__)

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

static constexpr int kNumScales = sizeof(kScaleFrequencies) / sizeof(float);

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

static constexpr int kNumFirstStrings = sizeof(kFirstStringRatios) / sizeof(float);

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

void AudioEngine::setVolume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    masterVolume.store(volume);
}

void AudioEngine::updateStringFrequencies()
{
    int scaleIdx = currentScale.load();
    if (scaleIdx < 0) scaleIdx = 0;
    if (scaleIdx >= kNumScales) scaleIdx = kNumScales - 1;

    float baseSaFreq = kScaleFrequencies[scaleIdx];
    float firstStringFreq = baseSaFreq * kFirstStringRatios[currentFirstString];

    // Apply octave multiplier
    // octave1 (0) = 0.25x, octave2 (1) = 0.5x, octave3 (2) = 1.0x, octave4 (3) = 2.0x, octave5 (4) = 4.0x
    static const float kOctaveMultipliers[] = {0.25f, 0.5f, 1.0f, 2.0f, 4.0f};
    int octaveIdx = currentOctave;
    if (octaveIdx < 0) octaveIdx = 0;
    if (octaveIdx >= 5) octaveIdx = 4;
    float octaveMultiplier = kOctaveMultipliers[octaveIdx];

    // Standard 4-string tanpura layout:
    // String 0: First string (Pa or Ma) - provides melodic color
    // String 1: Sa (middle octave)
    // String 2: Sa (middle octave) - slight detune for beating
    // String 3: Sa (low octave) - bass drone
    stringFreq[0] = firstStringFreq * octaveMultiplier;
    stringFreq[1] = baseSaFreq * octaveMultiplier;
    stringFreq[2] = baseSaFreq * octaveMultiplier;
    stringFreq[3] = baseSaFreq * 0.5f * octaveMultiplier;  // Low octave kharaj

    LOGI("String frequencies (octave %d, %.2fx): First=%.1f, Sa=%.1f, Sa=%.1f, SaLow=%.1f",
         octaveIdx + 1, octaveMultiplier, stringFreq[0], stringFreq[1], stringFreq[2], stringFreq[3]);

    for (int i = 0; i < kNumStrings; i++)
    {
        stringPhase[i] = 0.0f;
    }
}

void AudioEngine::setFirstString(int firstStringIndex)
{
    if (firstStringIndex < 0)
        firstStringIndex = 0;
    else if (firstStringIndex >= kNumFirstStrings)
        firstStringIndex = kNumFirstStrings - 1;

    currentFirstString = firstStringIndex;
    isSaPaMode = (firstStringIndex >= 7);

    LOGI("First string set to %d, mode: %s", firstStringIndex, isSaPaMode ? "Sa-Pa" : "Sa-Ma");

    updateStringFrequencies();
}

void AudioEngine::setScale(int scaleIndex)
{
    if (scaleIndex < 0)
        scaleIndex = 0;
    else if (scaleIndex >= kNumScales)
        scaleIndex = kNumScales - 1;

    currentScale.store(scaleIndex);
    updateStringFrequencies();
}

void AudioEngine::setOctave(int octaveIndex)
{
    if (octaveIndex < 0)
        octaveIndex = 0;
    else if (octaveIndex >= 5)
        octaveIndex = 4;

    currentOctave = octaveIndex;
    LOGI("Octave set to %d", octaveIndex);
    updateStringFrequencies();
}

// ------------------------------------------------------------
// JIVARI BRIDGE SIMULATION
// The jivari (curved bridge) is what gives tanpura its buzz
// String grazes the bridge creating harmonic-rich buzz
// ------------------------------------------------------------
static inline float jivari(float sample, float envelope)
{
    // Jivari creates asymmetric clipping - string buzzes against bridge
    // More buzz when string has more energy (higher envelope)
    float buzzAmount = 0.3f + 0.4f * envelope;

    // Asymmetric soft clipping simulates string hitting bridge
    float x = sample * (1.0f + buzzAmount);
    if (x > 0.8f)
    {
        // Positive half - string lifts off bridge, cleaner
        x = 0.8f + 0.2f * tanhf((x - 0.8f) * 3.0f);
    }
    else if (x < -0.5f)
    {
        // Negative half - string hits bridge, creates buzz
        x = -0.5f - 0.3f * tanhf((-x - 0.5f) * 5.0f);
        // Add some grit/buzz on the negative swing
        x += 0.1f * sinf(x * 15.0f) * envelope;
    }

    return x;
}

// ------------------------------------------------------------
// Generate tanpura string with rich harmonics
// Based on SaMa_A analysis: H1=32%, H2=100%, H3=30%, H4=65%, etc.
// ------------------------------------------------------------
static inline float generateString(float phase, float envelope)
{
    // Rich harmonic content based on authentic tanpura analysis
    float sample =
        sinf(phase) * 0.32f +               // H1: 32%
        sinf(2.0f * phase) * 1.0f +         // H2: 100% dominant
        sinf(3.0f * phase) * 0.30f +        // H3: 30%
        sinf(4.0f * phase) * 0.65f +        // H4: 65% strong!
        sinf(5.0f * phase) * 0.21f +        // H5: 21%
        sinf(6.0f * phase) * 0.29f +        // H6: 29%
        sinf(7.0f * phase) * 0.10f +        // H7: 10%
        sinf(8.0f * phase) * 0.12f;         // H8: 12%

    // Apply jivari bridge effect - this creates the buzz!
    sample = jivari(sample, envelope);

    return sample;
}

// ------------------------------------------------------------
// Soft output limiter
// ------------------------------------------------------------
static inline float softLimit(float x)
{
    return tanhf(x * 0.7f) * 1.3f;
}

// ------------------------------------------------------------
// WAV file export
// ------------------------------------------------------------
bool AudioEngine::exportToWav(const char* filePath, float durationSec)
{
    LOGI("Exporting WAV to: %s, duration: %.1f sec", filePath, durationSec);

    const int exportSampleRate = 48000;
    const int numChannels = 2;
    const int totalFrames = static_cast<int>(durationSec * exportSampleRate);
    const int totalSamples = totalFrames * numChannels;

    std::vector<float> buffer(totalSamples);

    float exportPhase[4] = {0, 0, 0, 0};
    float exportEnvelope[4] = {0.6f, 0.6f, 0.6f, 0.6f};  // Start at sustain level
    bool exportRising[4] = {false, false, false, false};
    int exportFramesSincePluck = 0;
    int exportActiveString = 0;

    const float twoPi = 2.0f * M_PI;
    const float intervalSec = pluckIntervalSec.load();
    const int pluckIntervalFrames = static_cast<int>(intervalSec * exportSampleRate);
    const float gain = masterVolume.load();

    // TANPURA ENVELOPE - Very gentle, no sharp attack
    const float attackRate = 0.0003f;    // VERY slow attack
    const float decayRate = 0.999965f;   // Slow decay
    const float sustainLevel = 0.55f;    // HIGH sustain
    const float peakLevel = 0.85f;       // Subtle swell

    for (int i = 0; i < totalFrames; i++)
    {
        exportFramesSincePluck++;
        if (exportFramesSincePluck >= pluckIntervalFrames)
        {
            exportRising[exportActiveString] = true;
            exportFramesSincePluck = 0;
            exportActiveString = (exportActiveString + 1) % kNumStrings;
        }

        float left = 0.0f;
        float right = 0.0f;

        for (int s = 0; s < kNumStrings; s++)
        {
            // Gentle envelope - no sharp attack
            if (exportRising[s])
            {
                exportEnvelope[s] += attackRate;
                if (exportEnvelope[s] >= peakLevel)
                {
                    exportEnvelope[s] = peakLevel;
                    exportRising[s] = false;
                }
            }
            else
            {
                exportEnvelope[s] *= decayRate;
                if (exportEnvelope[s] < sustainLevel)
                    exportEnvelope[s] = sustainLevel;
            }

            float freq = stringFreq[s] * (1.0f + stringDetune[s]);
            float phaseInc = twoPi * freq / exportSampleRate;

            float sample = generateString(exportPhase[s], exportEnvelope[s]);
            sample *= exportEnvelope[s] * stringVolume[s];

            float pan = stringPan[s];
            left += sample * sqrtf(0.5f * (1.0f - pan));
            right += sample * sqrtf(0.5f * (1.0f + pan));

            exportPhase[s] += phaseInc;
            if (exportPhase[s] > twoPi)
                exportPhase[s] -= twoPi;
        }

        float outputGain = 0.4f * gain;
        left = softLimit(left * outputGain);
        right = softLimit(right * outputGain);

        buffer[i * 2] = left;
        buffer[i * 2 + 1] = right;
    }

    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        LOGI("Failed to open file for writing");
        return false;
    }

    std::vector<int16_t> pcmBuffer(totalSamples);
    for (int i = 0; i < totalSamples; i++)
    {
        float sample = buffer[i];
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        pcmBuffer[i] = static_cast<int16_t>(sample * 32767.0f);
    }

    int dataSize = totalSamples * sizeof(int16_t);
    int fileSize = 36 + dataSize;

    file.write("RIFF", 4);
    file.write(reinterpret_cast<char*>(&fileSize), 4);
    file.write("WAVE", 4);

    file.write("fmt ", 4);
    int fmtSize = 16;
    file.write(reinterpret_cast<char*>(&fmtSize), 4);
    int16_t audioFormat = 1;
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

    file.write("data", 4);
    file.write(reinterpret_cast<char*>(&dataSize), 4);
    file.write(reinterpret_cast<char*>(pcmBuffer.data()), dataSize);

    file.close();
    LOGI("WAV export complete: %d frames", totalFrames);
    return true;
}

// ------------------------------------------------------------
// Audio callback - AUTHENTIC TANPURA WITH JIVARI
// ------------------------------------------------------------
oboe::DataCallbackResult AudioEngine::onAudioReady(
    oboe::AudioStream *,
    void *audioData,
    int32_t numFrames)
{
    auto *output = static_cast<float *>(audioData);

    if (!engineRunning || !playing)
    {
        memset(audioData, 0, sizeof(float) * numFrames * 2);
        return oboe::DataCallbackResult::Continue;
    }

    const float intervalSec = pluckIntervalSec.load();
    const int pluckIntervalFrames = static_cast<int>(intervalSec * sampleRate);

    framesSincePluck += numFrames;
    if (framesSincePluck >= pluckIntervalFrames)
    {
        stringRising[activeString] = true;
        stringAge[activeString] = 0;
        framesSincePluck = 0;
        activeString = (activeString + 1) % kNumStrings;
    }

    const float twoPi = 2.0f * M_PI;
    const float gain = masterVolume.load();

    // TANPURA ENVELOPE - Very gentle, no sharp attack
    // Real tanpura has continuous drone with subtle swells
    // Attack is VERY slow - no click/pluck sound
    const float attackRate = 0.0003f;    // VERY slow attack - 3000+ samples
    const float decayRate = 0.999965f;   // Slow decay
    const float sustainLevel = 0.55f;    // HIGH sustain - continuous drone
    const float peakLevel = 0.85f;       // Don't go to full 1.0 - subtle swell

    for (int i = 0; i < numFrames; i++)
    {
        float left = 0.0f;
        float right = 0.0f;

        for (int s = 0; s < kNumStrings; s++)
        {
            // Gentle envelope - NO sharp attack
            if (stringRising[s])
            {
                // Very slow rise - creates gentle swell, not pluck
                stringEnvelope[s] += attackRate;
                if (stringEnvelope[s] >= peakLevel)
                {
                    stringEnvelope[s] = peakLevel;
                    stringRising[s] = false;
                }
            }
            else
            {
                // Slow decay to high sustain floor
                stringEnvelope[s] *= decayRate;
                if (stringEnvelope[s] < sustainLevel)
                    stringEnvelope[s] = sustainLevel;
            }

            stringAge[s]++;

            // Frequency with slight detune for beating between strings
            float freq = stringFreq[s] * (1.0f + stringDetune[s]);
            float phaseInc = twoPi * freq / sampleRate;

            // Generate string with jivari buzz
            float sample = generateString(stringPhase[s], stringEnvelope[s]);
            sample *= stringEnvelope[s] * stringVolume[s];

            // Stereo pan
            float pan = stringPan[s];
            left += sample * sqrtf(0.5f * (1.0f - pan));
            right += sample * sqrtf(0.5f * (1.0f + pan));

            stringPhase[s] += phaseInc;
            if (stringPhase[s] > twoPi)
                stringPhase[s] -= twoPi;
        }

        // Output with soft limiting
        float outputGain = 0.4f * gain;
        left = softLimit(left * outputGain);
        right = softLimit(right * outputGain);

        output[i * 2] = left;
        output[i * 2 + 1] = right;
    }

    return oboe::DataCallbackResult::Continue;
}
