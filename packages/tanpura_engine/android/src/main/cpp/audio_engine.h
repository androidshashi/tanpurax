#pragma once
#include <oboe/Oboe.h>
#include <memory>

class AudioEngine : public oboe::AudioStreamCallback {
public:
    bool start();
    void stop();

    oboe::DataCallbackResult onAudioReady(
            oboe::AudioStream* audioStream,
            void* audioData,
            int32_t numFrames
    ) override;

    void setTempo(float intervalSec) { pluckIntervalSec = intervalSec; }

private:
    std::shared_ptr<oboe::AudioStream> stream;

    // ===== CORE =====
    float sampleRate = 48000.0f;

    // ===== TEMPO =====
    float pluckIntervalSec = 0.5f; // slow energy refresh

    // ===== TANPURA STRINGS (Sa–Pa–Sa–Sa) =====
    static constexpr int kNumStrings = 4;

    float stringFreq[kNumStrings] = {
            130.81f, // Sa
            196.00f, // Pa
            130.81f, // Sa
            130.81f  // Sa
    };

    float stringPhase[kNumStrings] = {0};
    float stringEnvelope[kNumStrings] = {0.4f, 0.4f, 0.4f, 0.4f};

    // ===== DRONE ENVELOPE =====
    float decayRate = 0.9997f;
    float sustainLevel = 0.35f;

    // ===== MUSICAL STATE =====
    int framesSincePluck = 0;
    int activeString = 0;
};
