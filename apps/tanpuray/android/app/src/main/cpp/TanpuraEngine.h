// android/app/src/main/cpp/TanpuraEngine.h
#ifndef TANPURA_ENGINE_H
#define TANPURA_ENGINE_H

#include <oboe/Oboe.h>
#include <vector>
#include "TanpuraString.h"

class TanpuraEngine : public oboe::AudioStreamDataCallback
{
private:
    std::vector<TanpuraString *> strings;
    double pitch;
    float *tempBuffer;
    int bufferSize;

    // Filters
    double lpf1;
    double lpf2;
    double dcBlockerX;
    double dcBlockerY;

    // Oboe stream
    std::shared_ptr<oboe::AudioStream> stream;

    void initializeStrings();

public:
    TanpuraEngine();
    ~TanpuraEngine();

    bool start();
    void stop();

    void setPitch(double freq);
    void setJawari(double intensity);
    void setTempo(double tempo);

    // Oboe callback
    oboe::DataCallbackResult onAudioReady(
        oboe::AudioStream *audioStream,
        void *audioData,
        int32_t numFrames) override;
};

#endif // TANPURA_ENGINE_H