#include "sample_player.h"

void sample_player::load(const std::vector<float> &data)
{
    buffer = data;
    loop_start = buffer.size() * 0.15f;
    loop_end = buffer.size() * 0.95f;
}

void sample_player::set_loop(float start_ratio, float end_ratio)
{
    loop_start = buffer.size() * start_ratio;
    loop_end = buffer.size() * end_ratio;
}

float sample_player::process(float rate)
{
    int i1 = (int)pos;
    int i2 = i1 + 1;
    if (i2 >= loop_end)
        i2 = loop_start;

    float frac = pos - i1;
    float s = buffer[i1] * (1 - frac) + buffer[i2] * frac;

    // ---- Crossfade near loop end ----
    if (pos >= loop_end - fade_len)
    {
        float fade = (loop_end - pos) / fade_len;

        int j1 = loop_start + (int)(pos - (loop_end - fade_len));
        int j2 = j1 + 1;
        if (j2 >= loop_end)
            j2 = loop_start;

        float s2 = buffer[j1];
        s = s * fade + s2 * (1.0f - fade);
    }

    pos += rate;
    if (pos >= loop_end)
        pos = loop_start;

    return s;
}
