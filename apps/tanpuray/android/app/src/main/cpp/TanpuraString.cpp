// android/app/src/main/cpp/TanpuraString.cpp
#include "TanpuraString.h"
#include <cmath>

TanpuraString::TanpuraString(double freq, double sr, double phaseOff)
    : frequency(freq), sampleRate(sr), jawariIntensity(0.7),
      jawariLFO1(phaseOff * 0.3), jawariLFO2(phaseOff * 0.5),
      tempoMultiplier(1.0), dist(-1.0, 1.0),
      fadeIn(0.0), fadeInSamples(0), phaseOffset(phaseOff)
{
    rng.seed(std::random_device{}());
    initializeHarmonics();
}

void TanpuraString::initializeHarmonics()
{
    phases.clear();
    amplitudes.clear();
    lfoPhases.clear();
    lfoFreqs.clear();

    for (int n = 1; n <= 40; n++)
    {
        double inharmonicity = 1.0 + (n * n - n) * 0.00002;

        double amp;
        if (n <= 3)
        {
            amp = 0.8 / std::pow(n, 0.7);
        }
        else if (n <= 10)
        {
            amp = 0.6 / std::pow(n, 1.0);
        }
        else if (n <= 25)
        {
            amp = 0.4 / std::pow(n, 1.2);
        }
        else
        {
            amp = 0.3 / std::pow(n, 1.4);
        }

        phases.push_back(phaseOffset);
        amplitudes.push_back(amp);
        lfoPhases.push_back(phaseOffset * 0.5);
        lfoFreqs.push_back(0.001 + (dist(rng) + 1.0) * 0.002);
    }

    fadeIn = 0.0;
    fadeInSamples = 0;
}

void TanpuraString::setFrequency(double freq)
{
    frequency = freq;
    initializeHarmonics();
}

void TanpuraString::setJawari(double intensity)
{
    jawariIntensity = intensity;
}

void TanpuraString::setTempo(double multiplier)
{
    tempoMultiplier = multiplier;
}

void TanpuraString::generateSamples(float *output, int numSamples)
{
    const double PI2 = 2.0 * M_PI;
    const int FADE_IN_DURATION = static_cast<int>(sampleRate * 5.0);

    for (int i = 0; i < numSamples; i++)
    {
        double sample = 0.0;

        if (fadeInSamples < FADE_IN_DURATION)
        {
            fadeIn = static_cast<double>(fadeInSamples) / FADE_IN_DURATION;
            fadeIn = fadeIn * fadeIn * fadeIn;
            fadeInSamples++;
        }
        else
        {
            fadeIn = 1.0;
        }

        for (size_t h = 0; h < phases.size(); h++)
        {
            int harmonic = h + 1;
            double inharmonicity = 1.0 + (harmonic * harmonic - harmonic) * 0.00002;
            double harmFreq = frequency * harmonic * inharmonicity;

            double lfoMod = std::sin(lfoPhases[h]) * 0.01 + 0.99;

            lfoPhases[h] += PI2 * lfoFreqs[h] / sampleRate;
            if (lfoPhases[h] >= PI2)
            {
                lfoPhases[h] = std::fmod(lfoPhases[h], PI2);
            }

            sample += std::sin(phases[h]) * amplitudes[h] * lfoMod;

            phases[h] += PI2 * harmFreq / sampleRate;
            if (phases[h] >= PI2)
            {
                phases[h] = std::fmod(phases[h], PI2);
            }
        }

        sample *= 0.03;
        sample *= 4.0;
        sample *= fadeIn;

        if (jawariIntensity > 0.0)
        {
            jawariLFO1 += PI2 * 0.008 * tempoMultiplier / sampleRate;
            jawariLFO2 += PI2 * 0.013 * tempoMultiplier / sampleRate;

            if (jawariLFO1 >= PI2)
                jawariLFO1 = std::fmod(jawariLFO1, PI2);
            if (jawariLFO2 >= PI2)
                jawariLFO2 = std::fmod(jawariLFO2, PI2);

            double shimmer = (std::sin(jawariLFO1) * 0.06 + std::sin(jawariLFO2) * 0.04);
            double modAmount = 1.0 + shimmer * jawariIntensity * 0.015;
            sample *= modAmount;

            double fundPhase = phases[0];
            double buzz = std::sin(fundPhase * 2.1) * 0.012 +
                          std::sin(fundPhase * 3.3) * 0.008 +
                          std::sin(fundPhase * 4.7) * 0.005 +
                          std::sin(fundPhase * 6.2) * 0.003;

            sample += buzz * jawariIntensity * 0.2;

            double noise = dist(rng) * 0.0006 * jawariIntensity * 0.3;
            sample += noise;
        }

        if (sample > 3.0)
            sample = 3.0;
        if (sample < -3.0)
            sample = -3.0;

        output[i] = static_cast<float>(sample);
    }
}