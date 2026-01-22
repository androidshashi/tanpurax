#pragma once
#include <vector>
#include <android/asset_manager.h>

bool load_wav_from_assets(
    AAssetManager *asset_manager,
    const char *asset_path,
    std::vector<float> &out_samples,
    int &out_sample_rate);
