// android/app/src/main/cpp/tanpura_engine.h
#ifndef TANPURA_ENGINE_H
#define TANPURA_ENGINE_H

#include <vector>
#include <cmath>
#include <random>

class TanpuraString
{
private:
    double frequency;
    double sampleRate;
    std::vector<double> phases;
    std::vector<double> amplitudes;
    std::vector<double> lfoPhases;
    std::vector<double> lfoFreqs;
    double jawariIntensity;
    double jawariLFO1;
    double jawariLFO2;
    double tempoMultiplier;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;

    double fadeIn;
    int fadeInSamples;
    double phaseOffset;

public:
    TanpuraString(double freq, double sr, double phaseOff = 0.0)
        : frequency(freq), sampleRate(sr), jawariIntensity(0.7),
          jawariLFO1(phaseOff * 0.3), jawariLFO2(phaseOff * 0.5),
          tempoMultiplier(1.0), dist(-1.0, 1.0),
          fadeIn(0.0), fadeInSamples(0), phaseOffset(phaseOff)
    {
        rng.seed(std::random_device{}());
        initializeHarmonics();
    }

    void initializeHarmonics()
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

    void setFrequency(double freq)
    {
        frequency = freq;
        initializeHarmonics();
    }

    void setJawari(double intensity)
    {
        jawariIntensity = intensity;
    }

    void setTempo(double multiplier)
    {
        tempoMultiplier = multiplier;
    }

    void generateSamples(float *output, int numSamples)
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
            sample *= 4.0; // Increased from 2.5 - much louder!
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
};

class TanpuraEngine
{
private:
    std::vector<TanpuraString *> strings;
    double sampleRate;
    double pitch;
    float *tempBuffer;
    int bufferSize;

    double lpf1;
    double lpf2;
    double dcBlockerX;
    double dcBlockerY;

public:
    TanpuraEngine(double sr, int bufSize)
        : sampleRate(sr), pitch(261.63), bufferSize(bufSize),
          lpf1(0.0), lpf2(0.0), dcBlockerX(0.0), dcBlockerY(0.0)
    {
        tempBuffer = new float[bufSize];
        initializeStrings();
    }

    ~TanpuraEngine()
    {
        for (auto *str : strings)
        {
            delete str;
        }
        delete[] tempBuffer;
    }

    void initializeStrings()
    {
        for (auto *str : strings)
        {
            delete str;
        }
        strings.clear();

        const double PI = 3.14159265358979323846;

        strings.push_back(new TanpuraString(pitch * 1.498, sampleRate, 0.0));
        strings.push_back(new TanpuraString(pitch * 1.502, sampleRate, PI * 0.3));
        strings.push_back(new TanpuraString(pitch * 0.997, sampleRate, 0.0));
        strings.push_back(new TanpuraString(pitch * 1.000, sampleRate, PI * 0.4));
        strings.push_back(new TanpuraString(pitch * 1.003, sampleRate, PI * 0.7));
        strings.push_back(new TanpuraString(pitch * 0.497, sampleRate, 0.0));
        strings.push_back(new TanpuraString(pitch * 0.503, sampleRate, PI * 0.5));
    }

    void setPitch(double freq)
    {
        pitch = freq;
        strings[0]->setFrequency(pitch * 1.498);
        strings[1]->setFrequency(pitch * 1.502);
        strings[2]->setFrequency(pitch * 0.997);
        strings[3]->setFrequency(pitch * 1.000);
        strings[4]->setFrequency(pitch * 1.003);
        strings[5]->setFrequency(pitch * 0.497);
        strings[6]->setFrequency(pitch * 0.503);
    }

    void setJawari(double intensity)
    {
        for (auto *str : strings)
        {
            str->setJawari(intensity);
        }
    }

    void setTempo(double tempo)
    {
        double multiplier = tempo / 60.0;
        for (auto *str : strings)
        {
            str->setTempo(multiplier);
        }
    }

    void generateAudio(float *output, int numSamples)
    {
        for (int i = 0; i < numSamples; i++)
        {
            output[i] = 0.0f;
        }

        for (auto *str : strings)
        {
            str->generateSamples(tempBuffer, numSamples);
            for (int i = 0; i < numSamples; i++)
            {
                output[i] += tempBuffer[i];
            }
        }

        const double R = 0.995;
        for (int i = 0; i < numSamples; i++)
        {
            double x = output[i];
            dcBlockerY = x - dcBlockerX + R * dcBlockerY;
            dcBlockerX = x;
            output[i] = static_cast<float>(dcBlockerY);
        }

        for (int i = 0; i < numSamples; i++)
        {
            lpf1 = 0.95 * lpf1 + 0.05 * output[i];
            lpf2 = 0.95 * lpf2 + 0.05 * lpf1;

            double x = lpf2 * 2.2;                                          // Increased from 1.4 - much louder!
            output[i] = static_cast<float>(x / (1.0 + std::abs(x) * 0.05)); // Less saturation for more volume

            if (output[i] > 1.0f)
                output[i] = 1.0f;
            if (output[i] < -1.0f)
                output[i] = -1.0f;

            if (!std::isfinite(output[i]))
            {
                output[i] = 0.0f;
            }
        }
    }
};

#endif // TANPURA_ENGINE_H