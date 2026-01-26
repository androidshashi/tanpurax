#include <jni.h>
#include "tanpura_engine.h"
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <string>

#define LOG_TAG "TanpuraJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Global engine instance
static TanpuraEngine *gEngine = nullptr;

// JNI callback implementation
class JniTanpuraCallback : public TanpuraCallback
{
public:
    JniTanpuraCallback(JNIEnv *env, jobject plugin)
    {
        env->GetJavaVM(&jvm);
        pluginRef = env->NewGlobalRef(plugin);

        jclass cls = env->GetObjectClass(plugin);
        onStringPluckMethod = env->GetMethodID(cls, "onStringPluck", "(I)V");
        onStateChangedMethod = env->GetMethodID(cls, "onStateChanged", "(Z)V");
    }

    ~JniTanpuraCallback()
    {
        JNIEnv *env = getEnv();
        if (env && pluginRef)
        {
            env->DeleteGlobalRef(pluginRef);
        }
    }

    void onStringPluck(int stringIndex) override
    {
        JNIEnv *env = getEnv();
        if (env && pluginRef && onStringPluckMethod)
        {
            env->CallVoidMethod(pluginRef, onStringPluckMethod, stringIndex);
        }
    }

    void onStateChanged(bool isPlaying) override
    {
        JNIEnv *env = getEnv();
        if (env && pluginRef && onStateChangedMethod)
        {
            env->CallVoidMethod(pluginRef, onStateChangedMethod, isPlaying ? JNI_TRUE : JNI_FALSE);
        }
    }

private:
    JavaVM *jvm = nullptr;
    jobject pluginRef = nullptr;
    jmethodID onStringPluckMethod = nullptr;
    jmethodID onStateChangedMethod = nullptr;

    JNIEnv *getEnv()
    {
        JNIEnv *env = nullptr;
        if (jvm)
        {
            int status = jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
            if (status == JNI_EDETACHED)
            {
                jvm->AttachCurrentThread(&env, nullptr);
            }
        }
        return env;
    }
};

static JniTanpuraCallback *gCallback = nullptr;

extern "C"
{

    // ============================================================
    // Lifecycle
    // ============================================================

    JNIEXPORT jboolean JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativeInitialize(
        JNIEnv *env,
        jobject thiz,
        jobject assetManager)
    {
        LOGI("nativeInitialize called");

        if (gEngine)
        {
            gEngine->dispose();
            delete gEngine;
            delete gCallback;
        }

        gEngine = new TanpuraEngine();
        gCallback = new JniTanpuraCallback(env, thiz);
        gEngine->setCallback(gCallback);

        AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
        if (!mgr)
        {
            LOGE("Failed to get AssetManager");
            return JNI_FALSE;
        }

        bool success = gEngine->initialize(mgr);
        return success ? JNI_TRUE : JNI_FALSE;
    }

    JNIEXPORT void JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativeDispose(
        JNIEnv *env,
        jobject thiz)
    {
        if (gEngine)
        {
            gEngine->dispose();
            delete gEngine;
            gEngine = nullptr;
        }
        if (gCallback)
        {
            delete gCallback;
            gCallback = nullptr;
        }
    }

    // ============================================================
    // Playback
    // ============================================================

    JNIEXPORT void JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativePlay(JNIEnv *env, jobject thiz)
    {
        if (gEngine)
            gEngine->play();
    }

    JNIEXPORT void JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativePause(JNIEnv *env, jobject thiz)
    {
        if (gEngine)
            gEngine->pause();
    }

    JNIEXPORT void JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativeStop(JNIEnv *env, jobject thiz)
    {
        if (gEngine)
            gEngine->stop();
    }

    JNIEXPORT jboolean JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativeIsPlaying(JNIEnv *env, jobject thiz)
    {
        return (gEngine && gEngine->isPlaying()) ? JNI_TRUE : JNI_FALSE;
    }

    // ============================================================
    // Configuration
    // ============================================================

    JNIEXPORT void JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativeSetConfig(
        JNIEnv *env, jobject thiz, jstring configJson)
    {
        // JSON parsing handled in individual setters
    }

    JNIEXPORT void JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativeSetScale(
        JNIEnv *env, jobject thiz, jint scaleIndex)
    {
        if (gEngine)
            gEngine->setScale(scaleIndex);
    }

    JNIEXPORT void JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativeSetOctave(
        JNIEnv *env, jobject thiz, jint octaveValue)
    {
        if (gEngine)
            gEngine->setOctave(octaveValue);
    }

    JNIEXPORT void JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativeSetVolume(
        JNIEnv *env, jobject thiz, jfloat volume)
    {
        if (gEngine)
            gEngine->setVolume(volume);
    }

    JNIEXPORT void JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativeSetTempo(
        JNIEnv *env, jobject thiz, jfloat seconds)
    {
        if (gEngine)
            gEngine->setTempo(seconds);
    }

    JNIEXPORT void JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativeSetFirstString(
        JNIEnv *env, jobject thiz, jint noteIndex)
    {
        if (gEngine)
            gEngine->setFirstString(noteIndex);
    }

    JNIEXPORT void JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativeSetStringConfig(
        JNIEnv *env, jobject thiz,
        jint stringIndex, jint noteIndex, jfloat volume,
        jfloat pan, jfloat fineTune, jboolean enabled)
    {
        if (gEngine)
        {
            StringConfig config;
            config.noteIndex = noteIndex;
            config.volume = volume;
            config.pan = pan;
            config.fineTune = fineTune;
            config.enabled = enabled == JNI_TRUE;
            gEngine->setStringConfig(stringIndex, config);
        }
    }

    JNIEXPORT void JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativeSetStringEnabled(
        JNIEnv *env, jobject thiz, jint stringIndex, jboolean enabled)
    {
        if (gEngine)
            gEngine->setStringEnabled(stringIndex, enabled == JNI_TRUE);
    }

    JNIEXPORT void JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativeSetStringFineTune(
        JNIEnv *env, jobject thiz, jint stringIndex, jfloat cents)
    {
        if (gEngine)
            gEngine->setStringFineTune(stringIndex, cents);
    }

    // ============================================================
    // Info
    // ============================================================

    JNIEXPORT jobjectArray JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativeGetAvailableMidiFiles(
        JNIEnv *env, jobject thiz)
    {
        std::vector<std::string> files;
        if (gEngine)
        {
            files = gEngine->getAvailableMidiFiles();
        }

        jobjectArray result = env->NewObjectArray(
            static_cast<jsize>(files.size()),
            env->FindClass("java/lang/String"),
            env->NewStringUTF(""));

        for (size_t i = 0; i < files.size(); i++)
        {
            env->SetObjectArrayElement(result, static_cast<jsize>(i),
                                       env->NewStringUTF(files[i].c_str()));
        }
        return result;
    }

    JNIEXPORT jboolean JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativeIsNoteAvailable(
        JNIEnv *env, jobject thiz, jstring midiFile)
    {
        if (!gEngine)
            return JNI_FALSE;

        const char *filename = env->GetStringUTFChars(midiFile, nullptr);
        bool available = gEngine->isNoteAvailable(filename);
        env->ReleaseStringUTFChars(midiFile, filename);
        return available ? JNI_TRUE : JNI_FALSE;
    }

    JNIEXPORT jstring JNICALL
    Java_com_tanpura_engine_plus_tanpura_1engine_1plus_TanpuraEnginePlusPlugin_nativeGetEngineInfo(
        JNIEnv *env, jobject thiz)
    {
        std::string info = "{}";
        if (gEngine)
        {
            info = gEngine->getEngineInfo();
        }
        return env->NewStringUTF(info.c_str());
    }

} // extern "C"
