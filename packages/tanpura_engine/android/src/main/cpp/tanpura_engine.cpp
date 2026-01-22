#include <jni.h>
#include "audio_engine.h"

// Single global engine instance
static AudioEngine engine;

extern "C"
{

    // ------------------------------------------------------------
    // Engine lifecycle
    // ------------------------------------------------------------

    JNIEXPORT void JNICALL
    Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativeInitialize(
        JNIEnv *, jobject)
    {
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

} // extern "C"
