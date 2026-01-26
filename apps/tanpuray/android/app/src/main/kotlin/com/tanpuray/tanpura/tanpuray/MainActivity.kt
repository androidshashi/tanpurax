// android/app/src/main/kotlin/com/tanpuray/tanpura/tanpuray/MainActivity.kt
package com.tanpuray.tanpura.tanpuray

import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioTrack
import android.os.Build
import android.os.Handler
import android.os.Looper
import io.flutter.embedding.android.FlutterActivity
import io.flutter.embedding.engine.FlutterEngine
import io.flutter.plugin.common.MethodChannel
import java.util.concurrent.locks.ReentrantLock
import kotlin.concurrent.thread
import java.util.concurrent.ArrayBlockingQueue

class TanpuraNative {
    companion object {
        init {
            System.loadLibrary("tanpura")
        }
    }
    
    external fun initialize(): Boolean
    external fun start(): Boolean
    external fun stop()
    external fun setPitch(frequency: Double)
    external fun setJawari(intensity: Double)
    external fun setTempo(tempo: Double)
    external fun destroy()
}

class MainActivity: FlutterActivity() {
    private val CHANNEL = "tanpura_dsp"
    private val tanpuraNative = TanpuraNative()
    
    override fun configureFlutterEngine(flutterEngine: FlutterEngine) {
        super.configureFlutterEngine(flutterEngine)
        
        MethodChannel(flutterEngine.dartExecutor.binaryMessenger, CHANNEL).setMethodCallHandler { call, result ->
            when (call.method) {
                "initialize" -> {
                    val success = tanpuraNative.initialize()
                    android.util.Log.d("Tanpura", if (success) "✅ Initialized with Oboe" else "❌ Init failed")
                    result.success(success)
                }
                "start" -> {
                    val success = tanpuraNative.start()
                    android.util.Log.d("Tanpura", if (success) "🎵 Started" else "❌ Start failed")
                    result.success(success)
                }
                "stop" -> {
                    tanpuraNative.stop()
                    android.util.Log.d("Tanpura", "🛑 Stopped")
                    result.success(true)
                }
                "setPitch" -> {
                    val pitch = call.argument<Double>("pitch") ?: 261.63
                    tanpuraNative.setPitch(pitch)
                    result.success(true)
                }
                "setJawari" -> {
                    val intensity = call.argument<Double>("intensity") ?: 0.7
                    tanpuraNative.setJawari(intensity)
                    result.success(true)
                }
                "setTempo" -> {
                    val tempo = call.argument<Double>("tempo") ?: 60.0
                    tanpuraNative.setTempo(tempo)
                    result.success(true)
                }
                else -> {
                    result.notImplemented()
                }
            }
        }
    }
    
    override fun onDestroy() {
        tanpuraNative.destroy()
        super.onDestroy()
    }
}