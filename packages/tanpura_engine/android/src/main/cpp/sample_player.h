#pragma once
#include <vector>

class sample_player
{
public:
    void load(const std::vector<float> &data);
    void set_loop(float start_ratio, float end_ratio);
    float process(float rate);

private:
    std::vector<float> buffer;
    float pos = 0.0f;
    int loop_start = 0;
    int loop_end = 0;
    int fade_len = 4096; // ~40ms @ 48kHz
};
