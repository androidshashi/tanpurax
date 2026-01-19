#include "audio_engine.h"
#include <android/log.h>
#include <cmath>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "TanpuraEngine", __VA_ARGS__)

bool AudioEngine::start() {
    LOGI("AudioEngine started");

    oboe::AudioStreamBuilder builder;

    builder.setDirection(oboe::Direction::Output)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setSharingMode(oboe::SharingMode::Exclusive)
            ->setChannelCount(oboe::ChannelCount::Mono)
            ->setFormat(oboe::AudioFormat::Float)
            ->setCallback(this);

    if (builder.openStream(stream) != oboe::Result::OK) {
        return false;
    }

    sampleRate = static_cast<float>(stream->getSampleRate());
    stream->requestStart();

    return true;
}

void AudioEngine::stop() {
    LOGI("AudioEngine stopped");

    if (stream) {
        stream->requestStop();
        stream->close();
        stream.reset();
    }
}

oboe::DataCallbackResult AudioEngine::onAudioReady(
        oboe::AudioStream*,
        void* audioData,
        int32_t numFrames) {

    auto* output = static_cast<float*>(audioData);

    // ================= MUSICAL CLOCK =================
    framesSincePluck += numFrames;
    int framesPerRefresh = static_cast<int>(pluckIntervalSec * sampleRate);

    // Gentle energy refresh (NOT a pluck)
    if (framesSincePluck >= framesPerRefresh) {

        // excite only ONE string at a time
        stringEnvelope[activeString] += 0.2f;
        if (stringEnvelope[activeString] > 1.0f)
            stringEnvelope[activeString] = 1.0f;

        activeString = (activeString + 1) % kNumStrings;
        framesSincePluck = 0;
    }

    const float twoPi = 2.0f * M_PI;

    for (int i = 0; i < numFrames; i++) {

        float mix = 0.0f;

        for (int s = 0; s < kNumStrings; s++) {

            // ---------- SUSTAINED ENVELOPE ----------
            stringEnvelope[s] *= decayRate;

            // never let sound die (tanpura resonance)
            if (stringEnvelope[s] < sustainLevel)
                stringEnvelope[s] = sustainLevel;

            // ---------- TONE ----------
            float phaseInc = twoPi * stringFreq[s] / sampleRate;

            float sample =
                    sin(stringPhase[s]) * 0.6f +
                    sin(2.0f * stringPhase[s]) * 0.25f +
                    sin(3.0f * stringPhase[s]) * 0.15f;

            sample *= stringEnvelope[s];
            mix += sample;

            // advance phase
            stringPhase[s] += phaseInc;
            if (stringPhase[s] > twoPi)
                stringPhase[s] -= twoPi;
        }

        // normalize (4 strings)
        output[i] = mix * 0.25f;
    }

    return oboe::DataCallbackResult::Continue;
}
