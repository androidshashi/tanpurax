#pragma once

#include <oboe/Oboe.h>
#include <atomic>
#include <memory>
#include <vector>
#include <mutex>
#include <string>
#include <android/asset_manager.h>

// Forward declarations
struct tsf;  // TinySoundFont

/**
 * String configuration for the tanpura
 */
struct StringConfig {
    int noteIndex = 0;      // Semitone offset (0=Sa, 7=Pa, etc.)
    float volume = 0.25f;   // 0.0 - 1.0
    float pan = 0.0f;       // -1.0 (left) to 1.0 (right)
    float fineTune = 0.0f;  // Cents (-100 to +100)
    bool enabled = true;
};

/**
 * Callback interface for events
 */
class TanpuraCallback {
public:
    virtual ~TanpuraCallback() = default;
    virtual void onStringPluck(int stringIndex) = 0;
    virtual void onStateChanged(bool isPlaying) = 0;
};

/**
 * Main Tanpura Audio Engine
 * 
 * Uses Oboe for low-latency audio output and TinySoundFont for SF2 synthesis.
 */
class TanpuraEngine : public oboe::AudioStreamCallback {
public:
    static constexpr int kNumStrings = 4;
    
    TanpuraEngine();
    ~TanpuraEngine();
    
    // ============================================================
    // Lifecycle
    // ============================================================
    
    /**
     * Initialize the engine with Android AssetManager
     * @param assetManager Android asset manager for loading SF2 and MIDI files
     * @return true if successful
     */
    bool initialize(AAssetManager* assetManager);
    
    /**
     * Release all resources
     */
    void dispose();
    
    /**
     * Check if engine is initialized
     */
    bool isInitialized() const { return mInitialized.load(); }
    
    // ============================================================
    // Playback Control
    // ============================================================
    
    void play();
    void pause();
    void stop();
    bool isPlaying() const { return mPlaying.load(); }
    
    // ============================================================
    // Configuration
    // ============================================================
    
    void setScale(int scaleIndex);
    void setOctave(int octaveValue);
    void setVolume(float volume);
    void setTempo(float seconds);
    void setFirstString(int noteIndex);
    void setStringConfig(int stringIndex, const StringConfig& config);
    void setStringEnabled(int stringIndex, bool enabled);
    void setStringFineTune(int stringIndex, float cents);
    
    // ============================================================
    // Info
    // ============================================================
    
    std::vector<std::string> getAvailableMidiFiles() const;
    bool isNoteAvailable(const std::string& midiFile) const;
    std::string getEngineInfo() const;
    
    // ============================================================
    // Callbacks
    // ============================================================
    
    void setCallback(TanpuraCallback* callback) { mCallback = callback; }
    
    // ============================================================
    // Oboe Callback
    // ============================================================
    
    oboe::DataCallbackResult onAudioReady(
        oboe::AudioStream* audioStream,
        void* audioData,
        int32_t numFrames) override;

private:
    // Audio stream
    std::shared_ptr<oboe::AudioStream> mStream;
    
    // State
    std::atomic<bool> mInitialized{false};
    std::atomic<bool> mPlaying{false};
    std::atomic<float> mMasterVolume{0.85f};
    
    // SoundFont synthesizer
    tsf* mSoundFont = nullptr;
    std::mutex mSynthMutex;
    
    // Asset manager
    AAssetManager* mAssetManager = nullptr;
    
    // Audio parameters
    float mSampleRate = 48000.0f;
    
    // Scale and pitch
    int mScaleIndex = 0;        // 0=C, 1=C#, ... 11=B
    int mOctaveValue = 0;       // -1, 0, +1
    int mBaseMidiNote = 60;     // Middle C as base Sa
    
    // Strings
    StringConfig mStrings[kNumStrings];
    
    // Timing
    float mTempo = 0.4f;        // Seconds between plucks
    double mPluckTimer = 0.0;
    int mCurrentString = 0;
    
    // Note state tracking
    struct ActiveNote {
        int midiNote = -1;
        bool active = false;
    };
    ActiveNote mActiveNotes[kNumStrings];
    
    // Callback
    TanpuraCallback* mCallback = nullptr;
    
    // ============================================================
    // Internal Methods
    // ============================================================
    
    bool loadSoundFont();
    bool loadMidiData();
    void triggerString(int stringIndex);
    void releaseString(int stringIndex);
    int calculateMidiNote(int stringIndex);
    float getSwaraRatio(int noteIndex);
    
    // Available MIDI files
    std::vector<std::string> mAvailableMidiFiles;
};
