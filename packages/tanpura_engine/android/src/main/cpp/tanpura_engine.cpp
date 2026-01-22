#include <jni.h>
#include "audio_engine.h"
#include "wav_loader.h"
#include <android/asset_manager_jni.h>

// Single global engine instance
static AudioEngine engine;

extern "C"
{

    // ------------------------------------------------------------
    // Engine lifecycle
    // ------------------------------------------------------------

    JNIEXPORT void JNICALL
    Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativeInitialize(
        JNIEnv *env, jobject, jobject assetManager)
    {
        AAssetManager *mgr =
            AAssetManager_fromJava(env, assetManager);

        std::vector<float> wav;
        int sr = 0;

        load_wav_from_assets(
            mgr,
            "audio/tanpura_c_loop.wav",
            wav,
            sr);

        engine.load_tanpura_sample(wav);

        engine.initialize(); // internal engine start
    }

    JNIEXPORT void JNICALL
    Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativeRelease(
        JNIEnv
            *,
        jobject)
    {
        engine.

            release(); // internal engine stop
    }

    JNIEXPORT jboolean

        JNICALL
        Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativeIsEngineRunning(
            JNIEnv *, jobject)
    {
        return engine.isEngineRunning();
    }

    // ------------------------------------------------------------
    // Playback lifecycle
    // ------------------------------------------------------------

    JNIEXPORT void JNICALL
    Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativePlay(
        JNIEnv
            *,
        jobject)
    {
        engine.

            play();
    }

    JNIEXPORT void JNICALL
    Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativePause(
        JNIEnv
            *,
        jobject)
    {
        engine.

            pause();
    }

    JNIEXPORT jboolean

        JNICALL
        Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativeIsPlaying(
            JNIEnv *, jobject)
    {
        return engine.isPlaying();
    }

    // ------------------------------------------------------------
    // Parameters
    // ------------------------------------------------------------

    JNIEXPORT void JNICALL
    Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativeSetTempo(
        JNIEnv
            *,
        jobject,
        jfloat intervalSec)
    {
        engine.setTempo(intervalSec);
    }

    JNIEXPORT void JNICALL
    Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativeSetFirstString(
        JNIEnv *,
        jobject,
        jint value)
    {
        engine.setFirstString(static_cast<int>(value));
    }

    JNIEXPORT void JNICALL
    Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativeSetVolume(
        JNIEnv *,
        jobject,
        jfloat volume)
    {
        engine.setVolume(volume);
    }

} // extern "C"
