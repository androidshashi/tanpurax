#include "tanpura_engine.h"
#include <android/log.h>
#include <cmath>
#include <cstring>
#include <sstream>

#define TSF_IMPLEMENTATION
#include "tsf.h"

#define LOG_TAG "TanpuraEngine"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// Swara ratios (relative to Sa = 1.0)
static const float kSwaraRatios[12] = {
    1.0000f,  // Sa
    1.05946f, // Re♭ (komal)
    1.12246f, // Re
    1.18921f, // Ga♭ (komal)
    1.25992f, // Ga
    1.33484f, // Ma
    1.41421f, // Ma# (tivra)
    1.49831f, // Pa
    1.58740f, // Dha♭ (komal)
    1.68179f, // Dha
    1.78180f, // Ni♭ (komal)
    1.88775f  // Ni
};

// Scale base frequencies (relative to C)
static const float kScaleRatios[12] = {
    1.0000f,  // C
    1.05946f, // C#
    1.12246f, // D
    1.18921f, // D#
    1.25992f, // E
    1.33484f, // F
    1.41421f, // F#
    1.49831f, // G
    1.58740f, // G#
    1.68179f, // A
    1.78180f, // A#
    1.88775f  // B
};

TanpuraEngine::TanpuraEngine() {
    // Default string configuration: Pa-Sa-Sa-Sa (kharaj)
    mStrings[0] = {7, 0.28f, -0.5f, 0.0f, true};   // Pa
    mStrings[1] = {0, 0.22f, -0.2f, 0.0f, true};   // Sa
    mStrings[2] = {0, 0.26f, 0.2f, 0.0f, true};    // Sa
    mStrings[3] = {0, 0.24f, 0.5f, -1200.0f, true}; // Sa (kharaj - one octave lower)
}

TanpuraEngine::~TanpuraEngine() {
    dispose();
}

bool TanpuraEngine::initialize(AAssetManager* assetManager) {
    if (mInitialized.load()) {
        LOGI("Already initialized");
        return true;
    }
    
    mAssetManager = assetManager;
    
    // Load SoundFont
    if (!loadSoundFont()) {
        LOGE("Failed to load SoundFont");
        // Continue anyway - we'll generate simple tones as fallback
    }
    
    // Scan for available MIDI files
    loadMidiData();
    
    // Create Oboe audio stream
    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Output)
           ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
           ->setSharingMode(oboe::SharingMode::Exclusive)
           ->setChannelCount(oboe::ChannelCount::Stereo)
           ->setFormat(oboe::AudioFormat::Float)
           ->setCallback(this);
    
    oboe::Result result = builder.openStream(mStream);
    if (result != oboe::Result::OK) {
        LOGE("Failed to open audio stream: %s", oboe::convertToText(result));
        return false;
    }
    
    mSampleRate = static_cast<float>(mStream->getSampleRate());
    LOGI("Audio stream opened: %.0f Hz", mSampleRate);
    
    // Configure SoundFont output if loaded
    if (mSoundFont) {
        tsf_set_output(mSoundFont, TSF_STEREO_INTERLEAVED, static_cast<int>(mSampleRate), 0.0f);
        tsf_set_max_voices(mSoundFont, 32);
    }
    
    // Start the stream
    result = mStream->requestStart();
    if (result != oboe::Result::OK) {
        LOGE("Failed to start stream: %s", oboe::convertToText(result));
        return false;
    }
    
    mInitialized.store(true);
    LOGI("TanpuraEngine initialized successfully");
    return true;
}

void TanpuraEngine::dispose() {
    if (!mInitialized.load()) return;
    
    mPlaying.store(false);
    
    if (mStream) {
        mStream->requestStop();
        mStream->close();
        mStream.reset();
    }
    
    std::lock_guard<std::mutex> lock(mSynthMutex);
    if (mSoundFont) {
        tsf_close(mSoundFont);
        mSoundFont = nullptr;
    }
    
    mInitialized.store(false);
    LOGI("TanpuraEngine disposed");
}

bool TanpuraEngine::loadSoundFont() {
    if (!mAssetManager) {
        LOGE("AssetManager not available");
        return false;
    }
    
    // Try to load the SoundFont from assets
    AAsset* asset = AAssetManager_open(mAssetManager, "soundfont/Tanpura.sf2", AASSET_MODE_BUFFER);
    if (!asset) {
        LOGE("Failed to open Tanpura.sf2");
        return false;
    }
    
    off_t size = AAsset_getLength(asset);
    const void* buffer = AAsset_getBuffer(asset);
    
    LOGI("Loading SoundFont: %ld bytes", static_cast<long>(size));
    
    // Check if data looks like valid SF2 (should start with "RIFF")
    // Your file appears encrypted - handle accordingly
    const unsigned char* data = static_cast<const unsigned char*>(buffer);
    if (size >= 4) {
        LOGD("SF2 header bytes: %02x %02x %02x %02x", data[0], data[1], data[2], data[3]);
    }
    
    std::lock_guard<std::mutex> lock(mSynthMutex);
    mSoundFont = tsf_load_memory(buffer, static_cast<int>(size));
    
    AAsset_close(asset);
    
    if (!mSoundFont) {
        LOGE("Failed to parse SoundFont");
        return false;
    }
    
    int presetCount = tsf_get_presetcount(mSoundFont);
    LOGI("SoundFont loaded with %d presets", presetCount);
    
    return true;
}

bool TanpuraEngine::loadMidiData() {
    if (!mAssetManager) return false;
    
    // List of expected MIDI files
    const char* midiFiles[] = {
        "sa.mid", "re.mid", "re1.mid", "ga.mid", "ga1.mid",
        "ma.mid", "ma2.mid", "pa.mid", "dha.mid", "dha1.mid",
        "ni.mid", "ni1.mid"
    };
    
    mAvailableMidiFiles.clear();
    
    for (const char* filename : midiFiles) {
        std::string path = std::string("midi/") + filename;
        AAsset* asset = AAssetManager_open(mAssetManager, path.c_str(), AASSET_MODE_BUFFER);
        if (asset) {
            mAvailableMidiFiles.push_back(filename);
            
            // Log first few bytes of MIDI file
            off_t size = AAsset_getLength(asset);
            const unsigned char* data = static_cast<const unsigned char*>(AAsset_getBuffer(asset));
            if (size >= 4) {
                LOGD("MIDI %s header: %02x %02x %02x %02x", filename, data[0], data[1], data[2], data[3]);
            }
            
            AAsset_close(asset);
        }
    }
    
    LOGI("Found %zu MIDI files", mAvailableMidiFiles.size());
    return !mAvailableMidiFiles.empty();
}

void TanpuraEngine::play() {
    if (!mInitialized.load()) {
        LOGE("Cannot play - not initialized");
        return;
    }
    
    mPlaying.store(true);
    mPluckTimer = 0.0;
    mCurrentString = 0;
    
    // Trigger all strings initially
    for (int i = 0; i < kNumStrings; i++) {
        if (mStrings[i].enabled) {
            triggerString(i);
        }
    }
    
    if (mCallback) {
        mCallback->onStateChanged(true);
    }
    
    LOGI("Playback started");
}

void TanpuraEngine::pause() {
    mPlaying.store(false);
    
    // Release all notes
    for (int i = 0; i < kNumStrings; i++) {
        releaseString(i);
    }
    
    if (mCallback) {
        mCallback->onStateChanged(false);
    }
    
    LOGI("Playback paused");
}

void TanpuraEngine::stop() {
    mPlaying.store(false);
    
    // Release all notes and reset
    for (int i = 0; i < kNumStrings; i++) {
        releaseString(i);
    }
    
    mPluckTimer = 0.0;
    mCurrentString = 0;
    
    if (mSoundFont) {
        std::lock_guard<std::mutex> lock(mSynthMutex);
        tsf_reset(mSoundFont);
    }
    
    if (mCallback) {
        mCallback->onStateChanged(false);
    }
    
    LOGI("Playback stopped");
}

void TanpuraEngine::setScale(int scaleIndex) {
    mScaleIndex = std::max(0, std::min(11, scaleIndex));
    LOGD("Scale set to %d", mScaleIndex);
}

void TanpuraEngine::setOctave(int octaveValue) {
    mOctaveValue = std::max(-1, std::min(1, octaveValue));
    mBaseMidiNote = 60 + (mOctaveValue * 12); // C4 ± octave
    LOGD("Octave set to %d, base MIDI note: %d", mOctaveValue, mBaseMidiNote);
}

void TanpuraEngine::setVolume(float volume) {
    mMasterVolume.store(std::max(0.0f, std::min(1.0f, volume)));
    LOGD("Volume set to %.2f", volume);
}

void TanpuraEngine::setTempo(float seconds) {
    mTempo = std::max(0.2f, std::min(2.0f, seconds));
    LOGD("Tempo set to %.2f seconds", mTempo);
}

void TanpuraEngine::setFirstString(int noteIndex) {
    noteIndex = std::max(0, std::min(11, noteIndex));
    mStrings[0].noteIndex = noteIndex;
    LOGD("First string set to note index %d", noteIndex);
}

void TanpuraEngine::setStringConfig(int stringIndex, const StringConfig& config) {
    if (stringIndex < 0 || stringIndex >= kNumStrings) return;
    mStrings[stringIndex] = config;
    LOGD("String %d configured: note=%d, vol=%.2f, pan=%.2f", 
         stringIndex, config.noteIndex, config.volume, config.pan);
}

void TanpuraEngine::setStringEnabled(int stringIndex, bool enabled) {
    if (stringIndex < 0 || stringIndex >= kNumStrings) return;
    mStrings[stringIndex].enabled = enabled;
    
    if (!enabled) {
        releaseString(stringIndex);
    }
}

void TanpuraEngine::setStringFineTune(int stringIndex, float cents) {
    if (stringIndex < 0 || stringIndex >= kNumStrings) return;
    mStrings[stringIndex].fineTune = std::max(-100.0f, std::min(100.0f, cents));
}

std::vector<std::string> TanpuraEngine::getAvailableMidiFiles() const {
    return mAvailableMidiFiles;
}

bool TanpuraEngine::isNoteAvailable(const std::string& midiFile) const {
    for (const auto& file : mAvailableMidiFiles) {
        if (file == midiFile) return true;
    }
    return false;
}

std::string TanpuraEngine::getEngineInfo() const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"sampleRate\":" << mSampleRate << ",";
    oss << "\"initialized\":" << (mInitialized.load() ? "true" : "false") << ",";
    oss << "\"playing\":" << (mPlaying.load() ? "true" : "false") << ",";
    oss << "\"midiFileCount\":" << mAvailableMidiFiles.size() << ",";
    oss << "\"soundFontLoaded\":" << (mSoundFont ? "true" : "false");
    oss << "}";
    return oss.str();
}

float TanpuraEngine::getSwaraRatio(int noteIndex) {
    return kSwaraRatios[noteIndex % 12];
}

int TanpuraEngine::calculateMidiNote(int stringIndex) {
    const StringConfig& config = mStrings[stringIndex];
    
    // Base note + scale offset + swara offset
    int midiNote = mBaseMidiNote + mScaleIndex + config.noteIndex;
    
    // Apply fine tuning (convert cents to semitone offset for MIDI)
    // Fine tuning will be applied via pitch bend in a real implementation
    if (config.fineTune < -600.0f) {
        midiNote -= 12; // One octave down for kharaj
    }
    
    return midiNote;
}

void TanpuraEngine::triggerString(int stringIndex) {
    if (stringIndex < 0 || stringIndex >= kNumStrings) return;
    if (!mStrings[stringIndex].enabled) return;
    
    int midiNote = calculateMidiNote(stringIndex);
    
    // Track active note for release
    mActiveNotes[stringIndex].midiNote = midiNote;
    mActiveNotes[stringIndex].active = true;
    
    if (mSoundFont) {
        std::lock_guard<std::mutex> lock(mSynthMutex);
        tsf_note_on(mSoundFont, 0, midiNote, mStrings[stringIndex].volume);
    }
    
    // Notify callback
    if (mCallback) {
        mCallback->onStringPluck(stringIndex);
    }
}

void TanpuraEngine::releaseString(int stringIndex) {
    if (stringIndex < 0 || stringIndex >= kNumStrings) return;
    
    if (mActiveNotes[stringIndex].active) {
        if (mSoundFont) {
            std::lock_guard<std::mutex> lock(mSynthMutex);
            tsf_note_off(mSoundFont, 0, mActiveNotes[stringIndex].midiNote);
        }
        mActiveNotes[stringIndex].active = false;
    }
}

oboe::DataCallbackResult TanpuraEngine::onAudioReady(
    oboe::AudioStream* audioStream,
    void* audioData,
    int32_t numFrames
) {
    float* output = static_cast<float*>(audioData);
    
    if (!mInitialized.load() || !mPlaying.load()) {
        // Output silence
        memset(output, 0, sizeof(float) * numFrames * 2);
        return oboe::DataCallbackResult::Continue;
    }
    
    float volume = mMasterVolume.load();
    float invSampleRate = 1.0f / mSampleRate;
    
    // Render audio from SoundFont if available
    if (mSoundFont) {
        std::lock_guard<std::mutex> lock(mSynthMutex);
        tsf_render_float(mSoundFont, output, numFrames, 0);
    } else {
        // Fallback: Generate simple sine tones
        for (int i = 0; i < numFrames; i++) {
            float left = 0.0f;
            float right = 0.0f;
            
            // Simple tone generation as fallback
            static float phases[4] = {0, 0, 0, 0};
            
            for (int s = 0; s < kNumStrings; s++) {
                if (!mStrings[s].enabled || !mActiveNotes[s].active) continue;
                
                float freq = 261.63f * kScaleRatios[mScaleIndex] * 
                             getSwaraRatio(mStrings[s].noteIndex) *
                             powf(2.0f, mOctaveValue);
                
                // Apply fine tune
                if (mStrings[s].fineTune < -600.0f) {
                    freq *= 0.5f; // Octave down
                }
                
                phases[s] += 2.0f * M_PI * freq * invSampleRate;
                if (phases[s] > 2.0f * M_PI) phases[s] -= 2.0f * M_PI;
                
                // Generate rich tone with harmonics
                float sample = sinf(phases[s]) * 0.5f;
                sample += sinf(phases[s] * 2.0f) * 0.25f;
                sample += sinf(phases[s] * 3.0f) * 0.125f;
                sample += sinf(phases[s] * 4.0f) * 0.0625f;
                
                sample *= mStrings[s].volume;
                
                // Apply panning
                float pan = mStrings[s].pan;
                left += sample * sqrtf(0.5f * (1.0f - pan));
                right += sample * sqrtf(0.5f * (1.0f + pan));
            }
            
            // Apply soft saturation (jawari effect)
            left = tanhf(left * 1.5f);
            right = tanhf(right * 1.5f);
            
            output[i * 2] = left * volume;
            output[i * 2 + 1] = right * volume;
        }
    }
    
    // Handle string pluck timing
    for (int i = 0; i < numFrames; i++) {
        mPluckTimer += invSampleRate;
        
        if (mPluckTimer >= mTempo) {
            // Find next enabled string
            int attempts = 0;
            do {
                mCurrentString = (mCurrentString + 1) % kNumStrings;
                attempts++;
            } while (!mStrings[mCurrentString].enabled && attempts < kNumStrings);
            
            if (mStrings[mCurrentString].enabled) {
                triggerString(mCurrentString);
            }
            
            mPluckTimer = 0.0;
        }
    }
    
    return oboe::DataCallbackResult::Continue;
}
