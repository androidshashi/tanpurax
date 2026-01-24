#pragma once
#include <oboe/Oboe.h>
#include <atomic>
#include <memory>

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
    // Scale / pitch (base frequency of Sa)
    void setScale(int scaleIndex);

    // Master volume
    void setVolume(float volume);

    // ---------------- Export ----------------
    bool exportToWav(const char* filePath, float durationSec);

    // ---------------- Audio callback ----------------
    oboe::DataCallbackResult
    onAudioReady(
        oboe::AudioStream *audioStream,
        void *audioData,
        int32_t numFrames) override;

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
    std::atomic<float> pluckIntervalSec{0.5f}; // interval between string plucks

    // ===== SCALE / PITCH =====
    std::atomic<int> currentScale{2};  // Default D (index 2)
    int currentFirstString = 0;         // Track current first string for recalc
    void updateStringFrequencies();     // Recalculate frequencies

    // ===== TANPURA STRINGS (Sa–Pa–Sa–Sa) =====
    static constexpr int kNumStrings = 4;

    float stringFreq[kNumStrings] = {
        146.83f, // Sa (D3)
        220.25f, // Pa
        146.83f, // Sa
        146.83f  // Sa
    };

    float stringPhase[kNumStrings] = {0};
    float stringEnvelope[kNumStrings] = {0.0f, 0.0f, 0.0f, 0.0f};

    // ===== DRONE ENVELOPE =====
    // Attack/decay rates per sample (very smooth transitions)
    float attackRate = 0.00008f;   // Slower attack (~0.25s to peak) for smoother transitions
    float decayRate = 0.999985f;   // Much slower decay - string sustains ~3-4 seconds
    float sustainLevel = 0.65f;    // Higher floor level - reduces volume pumping dramatically

    // ===== PLUCKING STATE =====
    int framesSincePluck = 0;
    int activeString = 0;

    // Per-string pluck state (time since each string was plucked)
    int stringAge[kNumStrings] = {0, 0, 0, 0};
    bool stringRising[kNumStrings] = {false, false, false, false};

    // Stereo spread
    float stringPan[4] = {-0.6f, -0.2f, 0.2f, 0.6f}; // L → R

    // Micro timing offsets (samples)
    float stringTimeOffset[4] = {0, 0, 0, 0};
    int timingDriftCounter = 0;

    // Micro detune
    float detuneOffset[4] = {0, 0, 0, 0};
    int detuneCounter = 0;
};
