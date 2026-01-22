package com.tanpurax.tanpura_engine

import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.MethodChannel.Result
import android.content.Context
import android.content.res.AssetManager

class TanpuraEnginePlugin : FlutterPlugin, MethodChannel.MethodCallHandler {

    private lateinit var channel: MethodChannel
    private lateinit var appContext: Context

    companion object {
        init {
            // Load native library
            System.loadLibrary("tanpura_engine")
        }
    }

    // ===== JNI FUNCTIONS =====
    external fun nativeInitialize(assetManager: AssetManager)
    private external fun nativeRelease()

    private external fun nativePlay()
    private external fun nativePause()

    private external fun nativeIsEngineRunning(): Boolean
    private external fun nativeIsPlaying(): Boolean

    private external fun nativeSetTempo(intervalSec: Float)
    private external fun nativeSetFirstString(value: Int)

    private external fun nativeSetVolume(volume: Float)

    override fun onAttachedToEngine(binding: FlutterPlugin.FlutterPluginBinding) {
        channel = MethodChannel(
            binding.binaryMessenger,
            "com.tanpurax.tanpura/tanpura_audio"
        )
        channel.setMethodCallHandler(this)
        appContext = binding.applicationContext
    }

    override fun onMethodCall(call: MethodCall, result: MethodChannel.Result) {
        when (call.method) {

            // ------------------------------------------------------------
            // Engine lifecycle
            // ------------------------------------------------------------

            "initialize" -> {
                nativeInitialize(appContext.assets)
                result.success(null)
            }

            "release" -> {
                nativeRelease()
                result.success(null)
            }

            "is_engine_running" -> {
                result.success(nativeIsEngineRunning())
            }

            // ------------------------------------------------------------
            // Playback lifecycle
            // ------------------------------------------------------------

            "play" -> {
                nativePlay()
                result.success(null)
            }

            "pause" -> {
                nativePause()
                result.success(null)
            }

            "is_playing" -> {
                result.success(nativeIsPlaying())
            }

            // ------------------------------------------------------------
            // Parameters
            // ------------------------------------------------------------

            "set_tempo" -> {
                val interval = call.argument<Double>("interval_sec")?.toFloat()
                if (interval != null) {
                    nativeSetTempo(interval)
                }
                result.success(null)
            }

            "set_first_string" -> {
                val value = call.argument<Int>("value")

                if (value != null) {
                    nativeSetFirstString(value)
                    result.success(null)
                } else {
                    result.error(
                        "INVALID_ARGUMENT",
                        "First string value is required",
                        null
                    )
                }
            }
            "set_volume" -> {
                val volume = call.argument<Double>("volume")?.toFloat()
                if (volume != null) {
                    nativeSetVolume(volume)
                }
                result.success(null)
            }

            else -> result.notImplemented()
        }
    }


    override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {
        channel.setMethodCallHandler(null)
    }
}
