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
    private external fun nativeStart()
    private external fun nativeStop()
    private external fun nativeSetTempo(intervalSec: Float)

    override fun onAttachedToEngine(binding: FlutterPlugin.FlutterPluginBinding) {
        channel = MethodChannel(
            binding.binaryMessenger,
            "com.tanpurax.tanpura/tanpura_audio"
        )
        channel.setMethodCallHandler(this)
    }

    override fun onMethodCall(call: MethodCall, result: MethodChannel.Result) {
        when (call.method) {
            "start" -> {
                nativeStart()
                result.success(null)
            }

            "stop" -> {
                nativeStop()
                result.success(null)
            }

            "set_tempo" -> {
                val interval = call.argument<Double>("interval_sec")?.toFloat()
                if (interval != null) {
                    nativeSetTempo(interval)
                }
                result.success(null)
            }

            else -> result.notImplemented()
        }
    }

    override fun onDetachedFromEngine( binding: FlutterPlugin.FlutterPluginBinding) {
        channel.setMethodCallHandler(null)
    }
}
