#pragma once
#include <oboe/Oboe.h>
#include <atomic>
#include <memory>

class AudioEngine : public oboe::AudioStreamCallback
{
public:
    // ---------------- Engine lifecycle ----------------
    bool initialize();
    void release();
    bool isEngineRunning() const;

    // ---------------- Playback lifecycle ----------------
    void play();
    void pause();
    bool isPlaying() const;

    // ---------------- Parameters ----------------
    void setTempo(float intervalSec);
    void setFirstString(int firstStringIndex);
    void setScale(int scaleIndex);
    void setVolume(float volume);
    void setOctave(int octaveIndex); // 0=Low (C2-B2), 1=Mid (C3-B3), 2=High (C4-B4)

    // Getters for UI state
    int getOctave() const;

    // ---------------- Export ----------------
    bool exportToWav(const char *filePath, float durationSec);

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
    std::atomic<bool> playing{false};
    std::atomic<float> masterVolume{0.85f};

    // Core
    float sampleRate = 48000.0f;

    // Tempo - 0.8s interval = each string every 3.2 seconds
    std::atomic<float> pluckIntervalSec{0.8f};

    // Scale / pitch
    std::atomic<int> currentScale{2};  // Default D
    std::atomic<int> currentOctave{1}; // Default Mid octave (1) - 0=Low, 1=Mid, 2=High
    int currentFirstString = 7;        // Default Pa
    void updateStringFrequencies();

    // Harmonic mode (Sa-Pa vs Sa-Ma)
    bool isSaPaMode = true;

    // ===== TANPURA STRINGS =====
    static constexpr int kNumStrings = 4;

    float stringFreq[kNumStrings] = {
        220.0f,  // First string (Pa/Ma)
        146.83f, // Sa
        146.83f, // Sa
        73.42f   // Sa low octave (kharaj)
    };

    float stringPhase[kNumStrings] = {0};
    // Start with high envelope - tanpura is continuous drone
    float stringEnvelope[kNumStrings] = {0.6f, 0.6f, 0.6f, 0.6f};

    // Per-string STATIC permanent detune (creates natural beating)
    // Slight detune between same-note strings creates the shimmer
    float stringDetune[4] = {0.0f, 0.0015f, -0.0012f, 0.0008f};

    // Plucking state
    int framesSincePluck = 0;
    int activeString = 0;
    int stringAge[kNumStrings] = {0, 0, 0, 0};
    bool stringRising[kNumStrings] = {false, false, false, false};

    // Stereo spread - strings panned across stereo field
    float stringPan[4] = {-0.4f, -0.1f, 0.1f, 0.4f};

    // Per-string volume
    // First string (Pa/Ma) slightly softer, bass string also softer
    float stringVolume[4] = {0.85f, 1.0f, 1.0f, 0.75f};
};
