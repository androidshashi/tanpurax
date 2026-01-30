package com.tanpurax.tanpura_engine

import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.MethodChannel.Result

class TanpuraEnginePlugin : FlutterPlugin, MethodChannel.MethodCallHandler {

    private lateinit var channel: MethodChannel

    companion object {
        init {
            // Load native library
            System.loadLibrary("tanpura_engine")
        }
    }

    // ===== JNI FUNCTIONS =====
    private external fun nativeInitialize()
    private external fun nativeRelease()

    private external fun nativePlay()
    private external fun nativePause()

    private external fun nativeIsEngineRunning(): Boolean
    private external fun nativeIsPlaying(): Boolean

    private external fun nativeSetTempo(intervalSec: Float)
    private external fun nativeSetFirstString(value: Int)

    private external fun nativeSetVolume(volume: Float)
    private external fun nativeSetScale(value: Int)
    private external fun nativeExportWav(filePath: String, durationSec: Float): Boolean
    private external fun nativeSetOctave(value: Int)
    private external fun nativeGetOctave(): Int
    override fun onAttachedToEngine(binding: FlutterPlugin.FlutterPluginBinding) {
        channel = MethodChannel(
            binding.binaryMessenger,
            "com.tanpurax.tanpura/tanpura_audio"
        )
        channel.setMethodCallHandler(this)
    }

    override fun onMethodCall(call: MethodCall, result: MethodChannel.Result) {
        when (call.method) {

            // ------------------------------------------------------------
            // Engine lifecycle
            // ------------------------------------------------------------

            "initialize" -> {
                nativeInitialize()
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

            "set_scale" -> {
                val value = call.argument<Int>("value")
                if (value != null) {
                    nativeSetScale(value)
                    result.success(null)
                } else {
                    result.error(
                        "INVALID_ARGUMENT",
                        "Scale value is required",
                        null
                    )
                }
            }

            "set_octave" -> {
                val value = call.argument<Int>("value")
                if (value != null) {
                    // 0 = Low, 1 = Mid, 2 = High
                    nativeSetOctave(value)
                    result.success(null)
                } else {
                    result.error(
                        "INVALID_ARGUMENT",
                        "Octave value is required",
                        null
                    )
                }
            }
 
            "get_octave" -> {
                result.success(nativeGetOctave())
            }

            "export_wav" -> {
                val filePath = call.argument<String>("file_path")
                val duration = call.argument<Double>("duration")?.toFloat() ?: 5.0f
                if (filePath != null) {
                    val success = nativeExportWav(filePath, duration)
                    result.success(success)
                } else {
                    result.error(
                        "INVALID_ARGUMENT",
                        "File path is required",
                        null
                    )
                }
            }

            else -> result.notImplemented()
        }
    }


    override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {
        channel.setMethodCallHandler(null)
    }
}
