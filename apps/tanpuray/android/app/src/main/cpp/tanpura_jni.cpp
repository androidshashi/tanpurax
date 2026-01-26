// android/app/src/main/cpp/tanpura_jni.cpp
#include <jni.h>
#include "tanpura_engine.h"

static TanpuraEngine *engine = nullptr;

extern "C"
{

    JNIEXPORT void JNICALL
    Java_com_tanpuray_tanpura_tanpuray_TanpuraNative_initialize(
        JNIEnv *env,
        jobject /* this */,
        jint sampleRate,
        jint bufferSize)
    {

        if (engine != nullptr)
        {
            delete engine;
        }

        engine = new TanpuraEngine(sampleRate, bufferSize);
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
    Java_com_tanpuray_tanpura_tanpuray_TanpuraNative_generateAudio(
        JNIEnv *env,
        jobject /* this */,
        jfloatArray outputArray)
    {

        if (engine == nullptr)
            return;

        jsize len = env->GetArrayLength(outputArray);
        jfloat *output = env->GetFloatArrayElements(outputArray, nullptr);

        engine->generateAudio(output, len);

        env->ReleaseFloatArrayElements(outputArray, output, 0);
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