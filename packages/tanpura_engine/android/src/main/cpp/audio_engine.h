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

    sample_player tanpura_sample;
    float playback_rate = 1.0f;

    static constexpr int kNumStrings = 4;

    sample_player strings[kNumStrings];

    // base Sa rate (changes with scale / octave)
    float base_rate = 1.0f;

    // per-string offsets
    float string_rate[kNumStrings] = {1.0f, 1.5f, 1.0f, 1.0f};
    float string_detune[kNumStrings] = {0.0f, -0.003f, 0.002f, -0.0015f};
    float string_gain[kNumStrings] = {0.28f, 0.22f, 0.26f, 0.24f};
    float string_pan[kNumStrings] = {-0.6f, -0.2f, 0.2f, 0.6f};

    // breathing
    float breath_phase = 0.0f;

    // stereo
    float stereo_width = 0.6f;

    float detune_phase[4] = {0, 0, 0, 0};
};
