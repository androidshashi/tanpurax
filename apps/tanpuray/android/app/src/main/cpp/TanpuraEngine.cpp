// android/app/src/main/cpp/TanpuraEngine.cpp
#include "TanpuraEngine.h"
#include <android/log.h>
#include <cmath>

#define LOG_TAG "TanpuraEngine"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

TanpuraEngine::TanpuraEngine()
    : pitch(261.63), bufferSize(2048),
      lpf1(0.0), lpf2(0.0), dcBlockerX(0.0), dcBlockerY(0.0)
{
    tempBuffer = new float[bufferSize];
    initializeStrings();
}

TanpuraEngine::~TanpuraEngine()
{
    stop();
    for (auto *str : strings)
    {
        delete str;
    }
    delete[] tempBuffer;
}

void TanpuraEngine::initializeStrings()
{
    for (auto *str : strings)
    {
        delete str;
    }
    strings.clear();

    const double PI = 3.14159265358979323846;

    // 7 strings with detuning and phase offsets
    strings.push_back(new TanpuraString(pitch * 1.498, 44100, 0.0));
    strings.push_back(new TanpuraString(pitch * 1.502, 44100, PI * 0.3));
    strings.push_back(new TanpuraString(pitch * 0.997, 44100, 0.0));
    strings.push_back(new TanpuraString(pitch * 1.000, 44100, PI * 0.4));
    strings.push_back(new TanpuraString(pitch * 1.003, 44100, PI * 0.7));
    strings.push_back(new TanpuraString(pitch * 0.497, 44100, 0.0));
    strings.push_back(new TanpuraString(pitch * 0.503, 44100, PI * 0.5));
}

bool TanpuraEngine::start()
{
    oboe::AudioStreamBuilder builder;

    oboe::Result result = builder.setDirection(oboe::Direction::Output)
                              ->setPerformanceMode(oboe::PerformanceMode::None) // Changed from LowLatency
                              ->setSharingMode(oboe::SharingMode::Shared)       // Changed from Exclusive
                              ->setFormat(oboe::AudioFormat::Float)
                              ->setChannelCount(oboe::ChannelCount::Stereo)
                              ->setSampleRate(44100)
                              ->setFramesPerDataCallback(512) // Set buffer size - larger = smoother
                              ->setDataCallback(this)
                              ->openStream(stream);

    if (result != oboe::Result::OK)
    {
        LOGE("Failed to create stream: %s", oboe::convertToText(result));
        return false;
    }

    LOGD("Stream created: %d Hz, buffer capacity: %d, frames per callback: %d",
         stream->getSampleRate(),
         stream->getBufferCapacityInFrames(),
         stream->getFramesPerDataCallback());

    // Set larger buffer size for smoother playback (less chance of glitches)
    stream->setBufferSizeInFrames(stream->getBufferCapacityInFrames() / 2);

    result = stream->requestStart();
    if (result != oboe::Result::OK)
    {
        LOGE("Failed to start stream: %s", oboe::convertToText(result));
        return false;
    }

    LOGD("🎵 Tanpura started successfully with Oboe");
    LOGD("Performance mode: None (for smoothness), Sharing: Shared");
    return true;
}

void TanpuraEngine::stop()
{
    if (stream)
    {
        stream->stop();
        stream->close();
        stream.reset();
        LOGD("🛑 Tanpura stopped");
    }
}

void TanpuraEngine::setPitch(double freq)
{
    pitch = freq;
    if (strings.size() >= 7)
    {
        strings[0]->setFrequency(pitch * 1.498);
        strings[1]->setFrequency(pitch * 1.502);
        strings[2]->setFrequency(pitch * 0.997);
        strings[3]->setFrequency(pitch * 1.000);
        strings[4]->setFrequency(pitch * 1.003);
        strings[5]->setFrequency(pitch * 0.497);
        strings[6]->setFrequency(pitch * 0.503);
    }
}

void TanpuraEngine::setJawari(double intensity)
{
    for (auto *str : strings)
    {
        str->setJawari(intensity);
    }
}

void TanpuraEngine::setTempo(double tempo)
{
    double multiplier = tempo / 60.0;
    for (auto *str : strings)
    {
        str->setTempo(multiplier);
    }
}

oboe::DataCallbackResult TanpuraEngine::onAudioReady(
    oboe::AudioStream *audioStream,
    void *audioData,
    int32_t numFrames)
{

    auto *outputBuffer = static_cast<float *>(audioData);

    // Generate mono samples (reuse tempBuffer for efficiency)
    int samplesToGenerate = std::min(numFrames, bufferSize);

    // Clear temp buffer
    for (int i = 0; i < samplesToGenerate; i++)
    {
        tempBuffer[i] = 0.0f;
    }

    // Mix all strings into temp buffer
    static float stringBuffer[2048]; // Static to avoid allocation
    for (auto *str : strings)
    {
        str->generateSamples(stringBuffer, samplesToGenerate);
        for (int i = 0; i < samplesToGenerate; i++)
        {
            tempBuffer[i] += stringBuffer[i];
        }
    }

    // DC blocker (maintains state across callbacks - CRITICAL for continuity)
    const double R = 0.995;
    for (int i = 0; i < samplesToGenerate; i++)
    {
        double x = tempBuffer[i];
        dcBlockerY = x - dcBlockerX + R * dcBlockerY;
        dcBlockerX = x;
        tempBuffer[i] = static_cast<float>(dcBlockerY);
    }

    // Lowpass filter and output gain (state maintained across callbacks)
    for (int i = 0; i < samplesToGenerate; i++)
    {
        lpf1 = 0.95 * lpf1 + 0.05 * tempBuffer[i];
        lpf2 = 0.95 * lpf2 + 0.05 * lpf1;

        double x = lpf2 * 2.2;
        float filtered = static_cast<float>(x / (1.0 + std::abs(x) * 0.05));

        // Clamp
        if (filtered > 1.0f)
            filtered = 1.0f;
        if (filtered < -1.0f)
            filtered = -1.0f;
        if (!std::isfinite(filtered))
            filtered = 0.0f;

        tempBuffer[i] = filtered;
    }

    // Write stereo with crossfade if buffer size changes
    for (int i = 0; i < samplesToGenerate; i++)
    {
        outputBuffer[i * 2] = tempBuffer[i];
        outputBuffer[i * 2 + 1] = tempBuffer[i];
    }

    // Fill remaining frames with silence if needed
    for (int i = samplesToGenerate; i < numFrames; i++)
    {
        outputBuffer[i * 2] = 0.0f;
        outputBuffer[i * 2 + 1] = 0.0f;
    }

    return oboe::DataCallbackResult::Continue;
}