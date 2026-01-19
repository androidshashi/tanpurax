#include <jni.h>
#include "audio_engine.h"

// Single global engine instance
static AudioEngine engine;

extern "C"
{

    // -------------------------------------------------
    // START ENGINE
    // -------------------------------------------------
    JNIEXPORT void JNICALL
    Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativeStart(
        JNIEnv *env,
        jobject /* this */)
    {

        engine.start();
    }

    // -------------------------------------------------
    // STOP ENGINE
    // -------------------------------------------------
    JNIEXPORT void JNICALL
    Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativeStop(
        JNIEnv *env,
        jobject /* this */)
    {

        engine.stop();
    }

    // -------------------------------------------------
    // SET TEMPO
    // -------------------------------------------------
    JNIEXPORT void JNICALL
    Java_com_tanpurax_tanpura_1engine_TanpuraEnginePlugin_nativeSetTempo(
        JNIEnv *env,
        jobject /* this */,
        jfloat intervalSec)
    {

        engine.setTempo(intervalSec);
    }

} // extern "C"
