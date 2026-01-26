// android/app/src/main/cpp/tanpura_jni.cpp
#include <jni.h>
#include "TanpuraEngine.h"

static TanpuraEngine *engine = nullptr;

extern "C"
{

    JNIEXPORT jboolean JNICALL
    Java_com_tanpuray_tanpura_tanpuray_TanpuraNative_initialize(
        JNIEnv *env,
        jobject /* this */)
    {

        if (engine != nullptr)
        {
            delete engine;
        }

        engine = new TanpuraEngine();
        return JNI_TRUE;
    }

    JNIEXPORT jboolean JNICALL
    Java_com_tanpuray_tanpura_tanpuray_TanpuraNative_start(
        JNIEnv *env,
        jobject /* this */)
    {

        if (engine != nullptr)
        {
            return engine->start() ? JNI_TRUE : JNI_FALSE;
        }
        return JNI_FALSE;
    }

    JNIEXPORT void JNICALL
    Java_com_tanpuray_tanpura_tanpuray_TanpuraNative_stop(
        JNIEnv *env,
        jobject /* this */)
    {

        if (engine != nullptr)
        {
            engine->stop();
        }
    }

    JNIEXPORT void JNICALL
    Java_com_tanpuray_tanpura_tanpuray_TanpuraNative_setPitch(
        JNIEnv *env,
        jobject /* this */,
        jdouble frequency)
    {

        if (engine != nullptr)
        {
            engine->setPitch(frequency);
        }
    }

    JNIEXPORT void JNICALL
    Java_com_tanpuray_tanpura_tanpuray_TanpuraNative_setJawari(
        JNIEnv *env,
        jobject /* this */,
        jdouble intensity)
    {

        if (engine != nullptr)
        {
            engine->setJawari(intensity);
        }
    }

    JNIEXPORT void JNICALL
    Java_com_tanpuray_tanpura_tanpuray_TanpuraNative_setTempo(
        JNIEnv *env,
        jobject /* this */,
        jdouble tempo)
    {

        if (engine != nullptr)
        {
            engine->setTempo(tempo);
        }
    }

    JNIEXPORT void JNICALL
    Java_com_tanpuray_tanpura_tanpuray_TanpuraNative_destroy(
        JNIEnv *env,
        jobject /* this */)
    {

        if (engine != nullptr)
        {
            delete engine;
            engine = nullptr;
        }
    }

} // extern "C"