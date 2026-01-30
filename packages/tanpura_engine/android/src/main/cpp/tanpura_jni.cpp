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

    JNIEXPORT void JNICALL
    Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativeSetVolume(
        JNIEnv *,
        jobject,
        jfloat volume)
    {
        engine.setVolume(volume);
    }

    JNIEXPORT void JNICALL
    Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativeSetScale(
        JNIEnv *,
        jobject,
        jint value)
    {
        engine.setScale(static_cast<int>(value));
    }

    // ------------------------------------------------------------
    // Export
    // ------------------------------------------------------------

    JNIEXPORT jboolean JNICALL
    Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativeExportWav(
        JNIEnv *env,
        jobject,
        jstring filePath,
        jfloat durationSec)
    {
        const char *path = env->GetStringUTFChars(filePath, nullptr);
        bool result = engine.exportToWav(path, durationSec);
        env->ReleaseStringUTFChars(filePath, path);
        return result;
    }

    JNIEXPORT void JNICALL
    Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativeSetOctave(
        JNIEnv *,
        jobject,
        jint value)
    {
        // 0 = Low, 1 = Mid, 2 = High
        engine.setOctave(static_cast<int>(value));
    }

    JNIEXPORT jint JNICALL
    Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativeGetOctave(
        JNIEnv *,
        jobject)
    {
        return engine.getOctave();
    }

} // extern "C"
