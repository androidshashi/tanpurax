#pragma once
#include <oboe/Oboe.h>
#include <atomic>
#include <memory>
#include <cmath>
#include <cstring>

// ============================================================================
// TANPURA DSP ENGINE – Exact mirror of tanpura-engine.js
//
// Signal chain per string (identical to JS):
//   osc1(sine) ─────────────────────┐
//                                    ├─► eqFilter ─► env ─► panner ─► masterGain ─► compressor
//   osc2(saw) ─► javariFilter(BP) ──┘
//
// String layout:
//   0 PA:   freq = Sa × firstStringRatio  pan=−0.6  gainMod=0.9  highshelf 3kHz +5dB
//   1 SA1:  freq = Sa × 1.001            pan=−0.2  gainMod=1.0  no EQ
//   2 SA2:  freq = Sa × 0.999            pan=+0.2  gainMod=1.0  no EQ
//   3 BASS: freq = Sa × 0.5             pan=+0.6  gainMod=1.2  lowshelf 150Hz +4dB
//
// Javari: bandpass Q=3, sweeps freq×6 → freq over 6 s (JS: duration/2 = 12/2)
// Envelope: linear attack 0 → 0.5×gainMod in 0.8 s,
//           exponential decay → 0.001 over 11.2 s  (JS: total duration 12 s)
// Pluck gap: 1.3 / tempo  (default tempo=1 → 1.3 s)
// Compressor: threshold=−8 dBFS, ratio=4:1, attack=0.01 s, release=0.1 s
// ============================================================================

static constexpr double kPi    = 3.14159265358979323846;
static constexpr double kTwoPi = 6.28318530717958647692;

// ----------------------------------------------------------------------------
// Biquad – PeakEQ | Bandpass | HighShelf | LowShelf
// Coefficients match Web Audio BiquadFilterNode spec exactly.
// ----------------------------------------------------------------------------
class Biquad {
public:
    float b0=1,b1=0,b2=0, a1=0,a2=0, z1=0,z2=0;

    // Peaking EQ
    void setPeakEQ(float freq, float gainDb, float Q, float sr) {
        float A  = powf(10.f, gainDb / 40.f);
        float w0 = 2.f * (float)kPi * freq / sr;
        float cw = cosf(w0), sw = sinf(w0);
        float al = sw / (2.f * Q);
        float a0 = 1.f + al / A;
        b0=(1.f+al*A)/a0; b1=(-2.f*cw)/a0; b2=(1.f-al*A)/a0;
        a1=(-2.f*cw)/a0;  a2=(1.f-al/A)/a0;
    }

    // Constant-0-dB-peak bandpass (Web Audio BPF spec)
    void setBandpass(float freq, float Q, float sr) {
        float w0 = 2.f * (float)kPi * freq / sr;
        float cw = cosf(w0), sw = sinf(w0);
        float al = sw / (2.f * Q);
        float a0 = 1.f + al;
        b0= al/a0; b1=0.f; b2=-al/a0;
        a1=(-2.f*cw)/a0; a2=(1.f-al)/a0;
    }

    // High shelf, slope S=1  (Web Audio default Q = 1/√2)
    void setHighShelf(float freq, float gainDb, float sr) {
        float A  = powf(10.f, gainDb / 40.f);
        float w0 = 2.f * (float)kPi * freq / sr;
        float cw = cosf(w0), sw = sinf(w0);
        float al = sw * 0.5f * sqrtf(2.f);        // α for S=1
        float sA = 2.f * sqrtf(A) * al;
        float a0 = (A+1.f)-(A-1.f)*cw+sA;
        b0 =  A*((A+1.f)+(A-1.f)*cw+sA)/a0;
        b1 = -2.f*A*((A-1.f)+(A+1.f)*cw)/a0;
        b2 =  A*((A+1.f)+(A-1.f)*cw-sA)/a0;
        a1 =  2.f*((A-1.f)-(A+1.f)*cw)/a0;
        a2 =     ((A+1.f)-(A-1.f)*cw-sA)/a0;
    }

    // Low shelf, slope S=1
    void setLowShelf(float freq, float gainDb, float sr) {
        float A  = powf(10.f, gainDb / 40.f);
        float w0 = 2.f * (float)kPi * freq / sr;
        float cw = cosf(w0), sw = sinf(w0);
        float al = sw * 0.5f * sqrtf(2.f);
        float sA = 2.f * sqrtf(A) * al;
        float a0 = (A+1.f)+(A-1.f)*cw+sA;
        b0 =  A*((A+1.f)-(A-1.f)*cw+sA)/a0;
        b1 =  2.f*A*((A-1.f)-(A+1.f)*cw)/a0;
        b2 =  A*((A+1.f)-(A-1.f)*cw-sA)/a0;
        a1 = -2.f*((A-1.f)+(A+1.f)*cw)/a0;
        a2 =      ((A+1.f)+(A-1.f)*cw-sA)/a0;
    }

    float process(float x) {
        float y = b0*x + z1;
        z1 = b1*x - a1*y + z2;
        z2 = b2*x - a2*y;
        return y;
    }

    void reset() { z1=z2=0.f; }
};

// ----------------------------------------------------------------------------
// FeedforwardCompressor – mirrors JS DynamicsCompressor exactly
//   threshold = −8 dBFS  (0.398107 linear)
//   ratio     = 4 : 1
//   attack    = 0.01 s
//   release   = 0.10 s
//   Sidechain: max(|L|, |R|) per sample → apply same gain to both channels
// ----------------------------------------------------------------------------
class FeedforwardCompressor {
public:
    static constexpr float kThresholdLin = 0.398107f;  // 10^(−8/20)
    static constexpr float kRatioExp     = 0.75f;       // 1 − 1/ratio = 1 − 1/4

    float envLevel    = 0.f;
    float attackCoef  = 0.f;
    float releaseCoef = 0.f;

    void init(float sr) {
        // Discrete-time RC coefficients: coef = exp(−1 / (τ × sr))
        attackCoef  = expf(-1.f / (0.01f * sr));
        releaseCoef = expf(-1.f / (0.10f * sr));
    }

    // Process one stereo sample pair in-place
    void process(float& L, float& R) {
        float inputPeak = fmaxf(fabsf(L), fabsf(R));

        // Peak envelope follower with different attack / release
        if (inputPeak > envLevel)
            envLevel = attackCoef  * envLevel + (1.f - attackCoef)  * inputPeak;
        else
            envLevel = releaseCoef * envLevel + (1.f - releaseCoef) * inputPeak;

        if (envLevel > kThresholdLin) {
            // gain = (threshold / envLevel)^(1 − 1/ratio)
            float gain = powf(kThresholdLin / envLevel, kRatioExp);
            L *= gain;
            R *= gain;
        }
    }

    void reset() { envLevel = 0.f; }
};

// ----------------------------------------------------------------------------
// StringVoice – one tanpura string, mirrors JS playString() exactly
//
// Signal path:
//   sine + javariFilter(saw) → eqFilter → envelope × [panL, panR]
// ----------------------------------------------------------------------------
struct StringVoice {
    enum EnvState { IDLE, ATTACK, DECAY } envState = IDLE;

    // Oscillators
    double sinePhase = 0;
    double sawPhase  = 0;
    double phaseInc  = 0;

    // Javari bandpass (sweeps freq×6 → freq over 6 s, coeffs updated every 64 samples)
    Biquad javariFilter;
    double javariFreq       = 0;
    double javariTargetFreq = 0;
    double javariMult64     = 1.0;   // compound per-64-sample sweep multiplier
    int    javariCount      = 0;
    bool   javariSweeping   = false;

    // Per-string EQ
    Biquad eqFilter;
    bool   useEq = false;

    // Envelope
    double envGain    = 0;
    double peakGain   = 0;
    double attackStep = 0;
    double decayMult  = 1.0;

    // Constant-power stereo pan
    float panL = 0.707f;
    float panR = 0.707f;

    double sampleRate = 48000.0;

    // Trigger a new pluck (mirrors JS playString parameters)
    void trigger(double freq, double gainMod, float pan,
                 bool highShelf, bool lowShelf,
                 float shelfFreq, float shelfGainDb, double sr);

    // Accumulate one stereo sample into outL / outR
    void processSample(float& outL, float& outR);

    bool isActive() const { return envState != IDLE; }

    void reset() {
        envState  = IDLE;
        envGain   = 0;
        sinePhase = sawPhase = 0;
        javariFilter.reset();
        eqFilter.reset();
    }
};

// ============================================================================
// AudioEngine – Oboe-based host, same public API as before
// ============================================================================
class AudioEngine : public oboe::AudioStreamCallback {
public:
    bool initialize();
    void release();
    bool isEngineRunning() const;

    void play();
    void pause();
    bool isPlaying() const;

    void setTempo(float intervalSec);       // gap between string plucks in seconds
    void setFirstString(int idx);           // 0..11 chromatic (Pa=7, Ma=5, Ni=11)
    void setScale(int idx);                 // 0..11  (C3=0 .. B3=11)
    void setVolume(float vol);              // 0..1   → masterGain equivalent
    void setOctave(int idx);               // 0=low 1=mid 2=high
    void setJivari(float level);            // API-compat no-op (JS has no jivari knob)

    int   getOctave()  const;
    float getJivari()  const;

    bool exportToWav(const char* path, float duration);

    oboe::DataCallbackResult onAudioReady(
        oboe::AudioStream* stream,
        void* audioData,
        int32_t numFrames) override;

private:
    std::shared_ptr<oboe::AudioStream> stream;

    std::atomic<bool>  engineRunning{false};
    std::atomic<bool>  playing{false};
    std::atomic<float> masterVolume{0.8f};         // JS masterGain = 0.8
    std::atomic<float> jivariLevel{0.5f};
    std::atomic<float> pluckIntervalSec{1.3f};     // JS gap = 1.3 / tempo; default tempo=1
    std::atomic<int>   currentScale{2};             // D3 default
    std::atomic<int>   currentOctave{1};            // mid

    float sampleRate         = 48000.f;
    int   currentFirstString = 7;                   // Pa (≈ 3/2 above, halved → 0.75 below)

    float stringFreq[4] = {110.f, 147.0f, 146.7f, 73.4f};

    StringVoice           voices[4];
    FeedforwardCompressor compressor;               // mirrors JS DynamicsCompressor

    // 4-string sequential pluck scheduling
    int framesSincePluck = 0;
    int currentString    = 0;

    void updateFrequencies();
    void initEngine();
};
