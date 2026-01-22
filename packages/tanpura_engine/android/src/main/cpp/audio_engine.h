#pragma once
#include <oboe/Oboe.h>
#include <atomic>
#include <memory>
#include "sample_player.h"

class AudioEngine : public oboe::AudioStreamCallback
{
public:
    // ---------------- Engine lifecycle ----------------
    bool initialize(); // start audio stream
    void release();    // stop audio stream
    bool isEngineRunning() const;

    // ---------------- Playback lifecycle ----------------
    void play();  // enable sound
    void pause(); // silence sound
    bool isPlaying() const;

    // ---------------- Parameters ----------------
    void setTempo(float intervalSec);
    // First string / tuning
    void setFirstString(int firstStringIndex);

    // Master volume
    void setVolume(float volume);

    // ---------------- Audio callback ----------------
    oboe::DataCallbackResult
    onAudioReady(
        oboe::AudioStream *audioStream,
        void *audioData,
        int32_t numFrames) override;
    // Load tanpura WAV samples (called from JNI, NOT audio thread)
    void load_tanpura_sample(const std::vector<float> &samples);

private:
    std::shared_ptr<oboe::AudioStream> stream;

    // Engine state
    std::atomic<bool> engineRunning{false};

    // Master output volume (0.0 – 1.0)
    std::atomic<float> masterVolume{0.85f};

    // Playback state
    std::atomic<bool> playing{false};

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

    // Stereo spread
    float stringPan[4] = {-0.6f, -0.2f, 0.2f, 0.6f}; // L → R

    // Micro timing offsets (samples)
    float stringTimeOffset[4] = {0, 0, 0, 0};
    int timingDriftCounter = 0;

    // Micro detune
    float detuneOffset[4] = {0, 0, 0, 0};
    int detuneCounter = 0;

    sample_player tanpura_sample;
    float playback_rate = 1.0f;

    float stereo_width = 0.6f; // 0.0 mono → 1.0 wide

    float breath_phase = 0.0f;
};
