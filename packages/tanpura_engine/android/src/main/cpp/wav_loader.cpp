#include "wav_loader.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <cstdint>
#include <cstring>

static uint32_t read_u32(const uint8_t *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

static uint16_t read_u16(const uint8_t *p)
{
    return p[0] | (p[1] << 8);
}

bool load_wav_from_assets(
    AAssetManager *asset_manager,
    const char *asset_path,
    std::vector<float> &out_samples,
    int &out_sample_rate)
{
    if (!asset_manager)
        return false;

    AAsset *asset = AAssetManager_open(
        asset_manager,
        asset_path,
        AASSET_MODE_BUFFER);

    if (!asset)
        return false;

    const uint8_t *data =
        (const uint8_t *)AAsset_getBuffer(asset);
    size_t size = AAsset_getLength(asset);

    if (!data || size < 44)
    {
        AAsset_close(asset);
        return false;
    }

    // ---- WAV header (PCM 16-bit only) ----
    int channels = read_u16(data + 22);
    out_sample_rate = read_u32(data + 24);
    int bits_per_sample = read_u16(data + 34);

    if (bits_per_sample != 16)
    {
        AAsset_close(asset);
        return false;
    }

    // Find "data" chunk (robust)
    size_t offset = 36;
    while (offset + 8 < size)
    {
        uint32_t chunk_id = read_u32(data + offset);
        uint32_t chunk_size = read_u32(data + offset + 4);
        if (chunk_id == 0x61746164)
        { // "data"
            offset += 8;
            break;
        }
        offset += 8 + chunk_size;
    }

    if (offset >= size)
    {
        AAsset_close(asset);
        return false;
    }

    int total_samples = (size - offset) / 2;
    out_samples.resize(total_samples / channels);

    const int16_t *pcm =
        (const int16_t *)(data + offset);

    for (int i = 0; i < out_samples.size(); i++)
    {
        int sum = 0;
        for (int c = 0; c < channels; c++)
        {
            sum += pcm[i * channels + c];
        }
        out_samples[i] =
            (sum / (float)channels) / 32768.0f;
    }

    AAsset_close(asset);
    return true;
}
