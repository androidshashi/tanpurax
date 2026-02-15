#pragma once
#include <oboe/Oboe.h>
#include <atomic>
#include <memory>
#include <cmath>
#include <cstring>

// ============================================================================
// TANPURA DSP ENGINE - Final Verified Implementation
//
// Based on the "Real Tanpura DSP Algorithm Blueprint" with:
// - 40 vibration modes per string (rich harmonics)
// - Power-Law Bridge Model (P^1.6) for authentic jivari
// - Shared bridge displacement for sympathetic resonance
// - Miraj body resonance (teakwood gourd)
//
// Key physics:
// - Dynamic Timbre: Buzz changes as string decays
// - Inharmonicity: Higher partials slightly sharp (natural shimmer)
// - Acoustic Interdependence: Strings sing together through shared bridge
// ============================================================================

static constexpr double kPi = 3.14159265358979323846;
static constexpr double kTwoPi = 6.28318530717958647692;

// ----------------------------------------------------------------------------
// Biquad Filter for Body Resonance
// ----------------------------------------------------------------------------
class Biquad {
public:
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;
    float z1 = 0.0f, z2 = 0.0f;

    void setPeakEQ(float freq, float gainDb, float Q, float sampleRate) {
        float A = powf(10.0f, gainDb / 40.0f);
        float w0 = 2.0f * static_cast<float>(kPi) * freq / sampleRate;
        float cosw = cosf(w0);
        float sinw = sinf(w0);
        float alpha = sinw / (2.0f * Q);

        float a0 = 1.0f + alpha / A;
        b0 = (1.0f + alpha * A) / a0;
        b1 = (-2.0f * cosw) / a0;
        b2 = (1.0f - alpha * A) / a0;
        a1 = (-2.0f * cosw) / a0;
        a2 = (1.0f - alpha / A) / a0;
    }

    float process(float x) {
        float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }

    void reset() {
        z1 = z2 = 0.0f;
    }
};

// ----------------------------------------------------------------------------
// Miraj Body Resonator - Simulates teakwood gourd resonance
// Without this, the strings sound like they're floating in mid-air
// ----------------------------------------------------------------------------
class MirajBodyResonator {
public:
    Biquad gourdFilter;     // 110Hz - Air volume resonance
    Biquad woodFilter;      // 280Hz - Wood body
    Biquad formantFilter;   // 1.2kHz - Vocal formant
    Biquad shimmerFilter;   // 3.5kHz - Bridge shimmer

    void init(float sampleRate) {
        gourdFilter.setPeakEQ(110.0f, 6.0f, 0.5f, sampleRate);
        woodFilter.setPeakEQ(280.0f, 4.0f, 0.7f, sampleRate);
        formantFilter.setPeakEQ(1200.0f, 3.0f, 1.0f, sampleRate);
        shimmerFilter.setPeakEQ(3500.0f, 2.0f, 1.5f, sampleRate);
    }

    float process(float input) {
        float out = input;
        out = gourdFilter.process(out);
        out = woodFilter.process(out);
        out = formantFilter.process(out);
        out = shimmerFilter.process(out);
        return out;
    }

    void reset() {
        gourdFilter.reset();
        woodFilter.reset();
        formantFilter.reset();
        shimmerFilter.reset();
    }
};

// ----------------------------------------------------------------------------
// String Mode - Single vibration mode of a string
// ----------------------------------------------------------------------------
struct StringMode {
    double q = 0;           // Displacement
    double v = 0;           // Velocity
    double w2 = 0;          // Angular frequency SQUARED (for efficiency)
    double d = 0;           // Damping coefficient
    double phi_bridge = 0;  // Mode shape at bridge contact point
};

// ----------------------------------------------------------------------------
// TanpuraString - Single string with 40 vibration modes
// ----------------------------------------------------------------------------
class TanpuraString {
public:
    static constexpr int kNumModes = 40;
    StringMode modes[kNumModes];
    double currentForce = 0;  // Current jivari force on this string

    void init(double f0, double sampleRate) {
        for (int n = 1; n <= kNumModes; ++n) {
            StringMode& m = modes[n - 1];

            // Inharmonicity: higher partials are slightly sharp
            // This creates the natural "shimmer" and "beating"
            double stiffness = 1.0 + (n * n * 0.00001);

            // Store angular frequency SQUARED for efficiency
            double w = kTwoPi * f0 * n * stiffness;
            m.w2 = w * w;

            // Frequency-dependent damping (higher harmonics decay faster)
            m.d = 0.4 + (n * 0.12);

            // Mode shape at bridge position (5% from end)
            m.phi_bridge = sin(n * kPi * 0.05);

            m.q = m.v = 0;
        }
    }

    void pluck(double force, double position) {
        // Apply force to each mode based on mode shape at pluck position
        for (int n = 1; n <= kNumModes; ++n) {
            modes[n - 1].v += force * sin(n * kPi * position);
        }
    }

    void reset() {
        for (int i = 0; i < kNumModes; ++i) {
            modes[i].q = 0;
            modes[i].v = 0;
        }
        currentForce = 0;
    }
};

// ----------------------------------------------------------------------------
// TanpuraDSP - The main DSP engine with shared bridge coupling
//
// This is the "Real-Deal" implementation:
// - All 4 strings share a single bridge state
// - Jivari uses Power-Law (P^1.6) for teakwood hardness
// - Strings resonate sympathetically through the bridge
// ----------------------------------------------------------------------------
class TanpuraDSP {
public:
    static constexpr int kNumStrings = 4;
    TanpuraString strings[kNumStrings];

    // Shared bridge state - this is what makes strings "sing together"
    double bridgeState = 0;
    double sampleRate = 48000.0;
    double jivariStrength = 1.0;
    double dt = 1.0 / 48000.0;

    // Verified Jivari constants
    static constexpr double kBridgeThreshold = -0.00008;   // Thread clearance (h_b)
    static constexpr double kBridgeStiffness = 9000000.0;  // Contact stiffness
    static constexpr double kBridgeExponent = 1.6;         // Teakwood sweet spot
    static constexpr double kCouplingFactor = 0.02;        // Sympathetic resonance

    void init(double rootFreq, double sr) {
        sampleRate = sr;
        dt = 1.0 / sr;

        // Standard tanpura tuning
        // String 0: Pa (1.5x root) or Ma depending on setting
        // String 1: Sa (root frequency)
        // String 2: Sa (root frequency, slight detune for beating)
        // String 3: Sa Low / Kharaj (0.5x root)
        strings[0].init(rootFreq * 1.5, sr);
        strings[1].init(rootFreq, sr);
        strings[2].init(rootFreq * 1.003, sr);  // Slight detune for natural beating
        strings[3].init(rootFreq * 0.5, sr);
    }

    void initWithFrequencies(double f0, double f1, double f2, double f3, double sr) {
        sampleRate = sr;
        dt = 1.0 / sr;
        strings[0].init(f0, sr);
        strings[1].init(f1, sr);
        strings[2].init(f2, sr);
        strings[3].init(f3, sr);
    }

    void setJivari(double strength) {
        jivariStrength = strength;
    }

    float processSample() {
        double totalBridgeForce = 0;

        // STEP 1: Calculate COLLECTIVE bridge force from all strings
        for (int s = 0; s < kNumStrings; ++s) {
            TanpuraString& str = strings[s];

            // Sum displacement at bridge position across all modes
            double y_bridge = 0;
            for (int i = 0; i < TanpuraString::kNumModes; ++i) {
                y_bridge += str.modes[i].q * str.modes[i].phi_bridge;
            }

            // Jivari Law: Power function for curved bridge contact
            // When string goes below threshold, bridge pushes back
            if (y_bridge < kBridgeThreshold) {
                double penetration = kBridgeThreshold - y_bridge;
                // F = k * d^1.6 (verified teakwood exponent)
                str.currentForce = jivariStrength * kBridgeStiffness
                                 * pow(penetration, kBridgeExponent);
            } else {
                str.currentForce = 0;
            }

            totalBridgeForce += str.currentForce;
        }

        // STEP 2: Update shared bridge coupling (inertial smoothing)
        // This is what makes the strings "sing together"
        bridgeState = bridgeState * 0.9 + totalBridgeForce * 0.1;

        // STEP 3: Update physics for each string mode
        float mix = 0;
        for (int s = 0; s < kNumStrings; ++s) {
            TanpuraString& str = strings[s];

            for (int i = 0; i < TanpuraString::kNumModes; ++i) {
                StringMode& m = str.modes[i];

                // Sympathetic resonance from shared bridge
                double coupling = bridgeState * kCouplingFactor;

                // Newton's second law: a = -w²q - dv + (F_jivari + F_coupling) * φ
                double a = (-m.w2 * m.q)
                         - (m.d * m.v)
                         + (str.currentForce + coupling) * m.phi_bridge;

                // Euler integration
                m.v += a * dt;
                m.q += m.v * dt;

                // Output is sum of velocities (proportional to sound pressure)
                mix += static_cast<float>(m.v);
            }
        }

        // Output normalization
        return mix * 0.005f;
    }

    void pluckString(int stringIndex, double force, double position) {
        if (stringIndex >= 0 && stringIndex < kNumStrings) {
            strings[stringIndex].pluck(force, position);
        }
    }

    void reset() {
        for (int s = 0; s < kNumStrings; ++s) {
            strings[s].reset();
        }
        bridgeState = 0;
    }
};

// ----------------------------------------------------------------------------
// Raised Cosine Excitation - Smooth pluck without clicks
// ----------------------------------------------------------------------------
class RaisedCosineExcitation {
public:
    int pluckSamples = 0;
    int totalSamples = 0;
    double amplitude = 0.0;
    double position = 0.25;
    double sampleRate = 48000.0;

    void setSampleRate(double sr) {
        sampleRate = sr;
    }

    void trigger(double intensity, double durationMs, double pos = 0.25) {
        amplitude = intensity;
        position = pos;
        totalSamples = static_cast<int>(durationMs * sampleRate / 1000.0);
        pluckSamples = totalSamples;
    }

    double generate() {
        if (pluckSamples <= 0) return 0.0;

        int elapsed = totalSamples - pluckSamples;
        double phase = (kTwoPi * elapsed) / totalSamples;

        // Raised cosine: smooth bell-shaped pulse
        double force = 0.5 * amplitude * (1.0 - cos(phase));

        pluckSamples--;
        return force;
    }

    bool isActive() const { return pluckSamples > 0; }
    double getPosition() const { return position; }

    void reset() {
        pluckSamples = 0;
        totalSamples = 0;
    }
};

// ============================================================================
// Main Audio Engine
// ============================================================================
class AudioEngine : public oboe::AudioStreamCallback {
public:
    bool initialize();
    void release();
    bool isEngineRunning() const;

    void play();
    void pause();
    bool isPlaying() const;

    void setTempo(float intervalSec);
    void setFirstString(int idx);
    void setScale(int idx);
    void setVolume(float vol);
    void setOctave(int idx);
    void setJivari(float level);

    int getOctave() const;
    float getJivari() const;

    bool exportToWav(const char* path, float duration);

    oboe::DataCallbackResult onAudioReady(
        oboe::AudioStream* stream,
        void* audioData,
        int32_t numFrames) override;

private:
    std::shared_ptr<oboe::AudioStream> stream;

    std::atomic<bool> engineRunning{false};
    std::atomic<bool> playing{false};
    std::atomic<float> masterVolume{0.85f};
    std::atomic<float> jivariLevel{0.5f};
    std::atomic<float> pluckIntervalSec{0.8f};
    std::atomic<int> currentScale{2};
    std::atomic<int> currentOctave{1};

    float sampleRate = 48000.0f;
    int currentFirstString = 7;
    bool isSaPaMode = true;

    // String frequencies (updated when scale/octave changes)
    float stringFreq[4] = {220.0f, 146.83f, 147.27f, 73.42f};

    // The main DSP engine
    TanpuraDSP dsp;

    // Raised cosine excitation for each string
    RaisedCosineExcitation excitation[4];

    // Body resonator
    MirajBodyResonator bodyResonator;

    // Stereo panning
    float stringPan[4] = {-0.3f, -0.1f, 0.1f, 0.3f};

    // Pluck scheduling (6-beat Darda cycle)
    int framesSincePluck = 0;
    int currentBeat = 0;

    void updateFrequencies();
    void initEngine();
};
