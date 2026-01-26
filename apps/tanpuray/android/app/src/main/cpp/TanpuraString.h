// android/app/src/main/cpp/TanpuraString.h
#ifndef TANPURA_STRING_H
#define TANPURA_STRING_H

#include <vector>
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

    void initializeHarmonics();

public:
    TanpuraString(double freq, double sr, double phaseOff = 0.0);

    void setFrequency(double freq);
    void setJawari(double intensity);
    void setTempo(double multiplier);
    void generateSamples(float *output, int numSamples);
};

#endif // TANPURA_STRING_H