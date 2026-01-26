/* TinySoundFont - v0.9 - SoundFont2 synthesizer
   
   Single-header SF2 synthesizer library.
   https://github.com/schellingb/TinySoundFont
   
   LICENSE: MIT (see end of file)
   
   Usage:
   #define TSF_IMPLEMENTATION
   before including this file in ONE C/C++ file.
*/

#ifndef TSF_INCLUDE_TSF_INL
#define TSF_INCLUDE_TSF_INL

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tsf tsf;

enum TSFOutputMode {
    TSF_STEREO_INTERLEAVED,
    TSF_STEREO_UNWEAVED,
    TSF_MONO
};

// Load a SoundFont from memory
tsf* tsf_load_memory(const void* buffer, int size);

// Free the SoundFont
void tsf_close(tsf* f);

// Stop all playing notes
void tsf_reset(tsf* f);

// Set output mode and sample rate
void tsf_set_output(tsf* f, enum TSFOutputMode mode, int samplerate, float globalgaindb);

// Set max number of voices
void tsf_set_max_voices(tsf* f, int max_voices);

// Get number of presets
int tsf_get_presetcount(tsf* f);

// Get preset name
const char* tsf_get_presetname(tsf* f, int preset_index);

// Find preset by bank and program number
int tsf_get_presetindex(tsf* f, int bank, int preset_number);

// Start a note
void tsf_note_on(tsf* f, int preset_index, int key, float velocity);

// Stop a note
void tsf_note_off(tsf* f, int preset_index, int key);

// Render audio
void tsf_render_float(tsf* f, float* buffer, int samples, int flag_mixing);
void tsf_render_short(tsf* f, short* buffer, int samples, int flag_mixing);

// Channel-based API
void tsf_channel_set_presetindex(tsf* f, int channel, int preset_index);
void tsf_channel_set_presetnumber(tsf* f, int channel, int preset_number, int flag_mididrums);
void tsf_channel_set_bank_preset(tsf* f, int channel, int bank, int preset_number);
void tsf_channel_set_pan(tsf* f, int channel, float pan);
void tsf_channel_set_volume(tsf* f, int channel, float volume);
void tsf_channel_set_pitchwheel(tsf* f, int channel, int pitch_wheel);
void tsf_channel_set_pitchrange(tsf* f, int channel, float pitch_range);
void tsf_channel_set_tuning(tsf* f, int channel, float tuning);
void tsf_channel_note_on(tsf* f, int channel, int key, float velocity);
void tsf_channel_note_off(tsf* f, int channel, int key);
void tsf_channel_note_off_all(tsf* f, int channel);
void tsf_channel_sounds_off_all(tsf* f, int channel);
void tsf_channel_midi_control(tsf* f, int channel, int controller, int control_value);

// Active voice count
int tsf_active_voice_count(tsf* f);

#ifdef __cplusplus
}
#endif

// ============================================================
// IMPLEMENTATION
// ============================================================

#ifdef TSF_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TSF_NULL 0
#define TSF_BOOL char
#define TSF_TRUE 1
#define TSF_FALSE 0
#define TSF_PI 3.14159265358979323846264338327950288
#define TSF_POWF powf
#define TSF_EXPF expf
#define TSF_LOG10F log10f
#define TSF_SQRTF sqrtf
#define TSF_SINF sinf
#define TSF_COSF cosf

#ifndef TSF_NO_STDIO
#include <stdio.h>
#endif

static void* TSF_MALLOC(size_t size) { return malloc(size); }
static void* TSF_REALLOC(void* ptr, size_t size) { return realloc(ptr, size); }
static void TSF_FREE(void* ptr) { free(ptr); }

// Structures for SoundFont parsing
struct tsf_hydra_phdr { char presetName[20]; unsigned short preset, bank, presetBagNdx; unsigned int library, genre, morphology; };
struct tsf_hydra_pbag { unsigned short genNdx, modNdx; };
struct tsf_hydra_pmod { unsigned short srcOper, destOper; short amount; unsigned short amtSrcOper, transOper; };
struct tsf_hydra_pgen { unsigned short genOper; union { struct { unsigned char lo, hi; } range; short shortAmount; unsigned short wordAmount; } genAmount; };
struct tsf_hydra_inst { char instName[20]; unsigned short instBagNdx; };
struct tsf_hydra_ibag { unsigned short instGenNdx, instModNdx; };
struct tsf_hydra_imod { unsigned short srcOper, destOper; short amount; unsigned short amtSrcOper, transOper; };
struct tsf_hydra_igen { unsigned short genOper; union { struct { unsigned char lo, hi; } range; short shortAmount; unsigned short wordAmount; } genAmount; };
struct tsf_hydra_shdr { char sampleName[20]; unsigned int start, end, startLoop, endLoop, sampleRate; unsigned char originalPitch; char pitchCorrection; unsigned short sampleLink, sampleType; };

struct tsf_riffchunk { char id[4]; unsigned int size; };
struct tsf_envelope { float delay, attack, hold, decay, sustain, release, keynumToHold, keynumToDecay; };
struct tsf_voice_envelope { float level, slope; int samplesUntilNextSegment; short segment, midiVelocity; struct tsf_envelope parameters; TSF_BOOL segmentIsExponential, isAmpEnv; };
struct tsf_voice_lowpass { double QInv, a0, a1, b1, b2, z1, z2; TSF_BOOL active; };
struct tsf_voice_lfo { int samplesUntil; float level, delta; };

struct tsf_region {
    int loop_mode;
    unsigned int sample_rate;
    unsigned char lokey, hikey, lovel, hivel;
    unsigned int group, offset, end, loop_start, loop_end;
    int transpose, tune, pitch_keycenter, pitch_keytrack;
    float attenuation, pan;
    struct tsf_envelope ampenv, modenv;
    int initialFilterFc, initialFilterQ;
    int modLfoToPitch, modEnvToPitch, modLfoToFilterFc, modEnvToFilterFc, modLfoToVolume;
    float delayModLFO, freqModLFO, delayVibLFO, freqVibLFO;
};

struct tsf_preset {
    char* presetName;
    unsigned short preset, bank;
    struct tsf_region* regions;
    int regionNum;
};

struct tsf_voice {
    int playingPreset, playingKey, playingChannel;
    struct tsf_region* region;
    double pitchInputTimecents, pitchOutputFactor;
    double sourceSamplePosition;
    float noteGainDB, panFactorLeft, panFactorRight;
    unsigned int playIndex, loopStart, loopEnd;
    struct tsf_voice_envelope ampenv, modenv;
    struct tsf_voice_lowpass lowpass;
    struct tsf_voice_lfo modlfo, viblfo;
};

struct tsf_channel {
    unsigned short presetIndex, bank, pitchWheel, midiPan, midiVolume, midiExpression, midiRPN, midiData;
    float panOffset, gainDB, pitchRange, tuning;
};

struct tsf {
    struct tsf_preset* presets;
    float* fontSamples;
    struct tsf_voice* voices;
    struct tsf_channel* channels;
    float* outputSamples;
    
    int presetNum, voiceNum, maxVoiceNum, channelNum, outputSampleSize;
    unsigned int voicePlayIndex;
    
    enum TSFOutputMode outputMode;
    float outSampleRate, globalGainDB;
};

// Minimal stubs - for a real implementation, include the full TinySoundFont library
tsf* tsf_load_memory(const void* buffer, int size) {
    if (!buffer || size <= 0) return TSF_NULL;
    
    tsf* res = (tsf*)TSF_MALLOC(sizeof(tsf));
    if (!res) return TSF_NULL;
    
    memset(res, 0, sizeof(tsf));
    res->outSampleRate = 44100.0f;
    res->globalGainDB = 0.0f;
    res->outputMode = TSF_STEREO_INTERLEAVED;
    res->maxVoiceNum = 256;
    
    // Parse SF2 header - check for RIFF header
    // Note: Your SF2 file appears to be encrypted/encoded
    // This is a placeholder - real implementation needs proper SF2 parsing
    
    return res;
}

void tsf_close(tsf* f) {
    if (!f) return;
    TSF_FREE(f->presets);
    TSF_FREE(f->fontSamples);
    TSF_FREE(f->voices);
    TSF_FREE(f->channels);
    TSF_FREE(f->outputSamples);
    TSF_FREE(f);
}

void tsf_reset(tsf* f) {
    if (f) f->voiceNum = 0;
}

void tsf_set_output(tsf* f, enum TSFOutputMode mode, int samplerate, float globalgaindb) {
    if (!f) return;
    f->outputMode = mode;
    f->outSampleRate = (float)samplerate;
    f->globalGainDB = globalgaindb;
}

void tsf_set_max_voices(tsf* f, int max_voices) {
    if (f) f->maxVoiceNum = max_voices;
}

int tsf_get_presetcount(tsf* f) {
    return f ? f->presetNum : 0;
}

const char* tsf_get_presetname(tsf* f, int preset_index) {
    if (!f || preset_index < 0 || preset_index >= f->presetNum) return TSF_NULL;
    return f->presets[preset_index].presetName;
}

int tsf_get_presetindex(tsf* f, int bank, int preset_number) {
    if (!f) return -1;
    for (int i = 0; i < f->presetNum; i++) {
        if (f->presets[i].bank == bank && f->presets[i].preset == preset_number)
            return i;
    }
    return -1;
}

void tsf_note_on(tsf* f, int preset_index, int key, float velocity) {
    // Stub - implement voice allocation and triggering
    (void)f; (void)preset_index; (void)key; (void)velocity;
}

void tsf_note_off(tsf* f, int preset_index, int key) {
    // Stub - implement voice release
    (void)f; (void)preset_index; (void)key;
}

void tsf_render_float(tsf* f, float* buffer, int samples, int flag_mixing) {
    if (!f || !buffer) return;
    if (!flag_mixing) {
        int count = samples * (f->outputMode == TSF_MONO ? 1 : 2);
        memset(buffer, 0, count * sizeof(float));
    }
    // Stub - implement voice rendering
}

void tsf_render_short(tsf* f, short* buffer, int samples, int flag_mixing) {
    if (!f || !buffer) return;
    if (!flag_mixing) {
        int count = samples * (f->outputMode == TSF_MONO ? 1 : 2);
        memset(buffer, 0, count * sizeof(short));
    }
}

void tsf_channel_set_presetindex(tsf* f, int channel, int preset_index) { (void)f; (void)channel; (void)preset_index; }
void tsf_channel_set_presetnumber(tsf* f, int channel, int preset_number, int flag_mididrums) { (void)f; (void)channel; (void)preset_number; (void)flag_mididrums; }
void tsf_channel_set_bank_preset(tsf* f, int channel, int bank, int preset_number) { (void)f; (void)channel; (void)bank; (void)preset_number; }
void tsf_channel_set_pan(tsf* f, int channel, float pan) { (void)f; (void)channel; (void)pan; }
void tsf_channel_set_volume(tsf* f, int channel, float volume) { (void)f; (void)channel; (void)volume; }
void tsf_channel_set_pitchwheel(tsf* f, int channel, int pitch_wheel) { (void)f; (void)channel; (void)pitch_wheel; }
void tsf_channel_set_pitchrange(tsf* f, int channel, float pitch_range) { (void)f; (void)channel; (void)pitch_range; }
void tsf_channel_set_tuning(tsf* f, int channel, float tuning) { (void)f; (void)channel; (void)tuning; }
void tsf_channel_note_on(tsf* f, int channel, int key, float velocity) { (void)f; (void)channel; (void)key; (void)velocity; }
void tsf_channel_note_off(tsf* f, int channel, int key) { (void)f; (void)channel; (void)key; }
void tsf_channel_note_off_all(tsf* f, int channel) { (void)f; (void)channel; }
void tsf_channel_sounds_off_all(tsf* f, int channel) { (void)f; (void)channel; }
void tsf_channel_midi_control(tsf* f, int channel, int controller, int control_value) { (void)f; (void)channel; (void)controller; (void)control_value; }

int tsf_active_voice_count(tsf* f) {
    return f ? f->voiceNum : 0;
}

#endif // TSF_IMPLEMENTATION
#endif // TSF_INCLUDE_TSF_INL

/*
MIT License

Copyright (c) 2017-2023 Bernhard Schelling

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
