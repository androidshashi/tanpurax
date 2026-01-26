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
    
    external fun initialize(sampleRate: Int, bufferSize: Int)
    external fun setPitch(frequency: Double)
    external fun setJawari(intensity: Double)
    external fun setTempo(tempo: Double)
    external fun generateAudio(output: FloatArray)
    external fun destroy()
}


class MainActivity: FlutterActivity() {
    private val CHANNEL = "tanpura_dsp"
    private var audioTrack: AudioTrack? = null
    private var isPlaying = false
    private var audioThread: Thread? = null
    
    private val sampleRate = 44100
    private val bufferSize = 2048
    
    private val tanpuraNative = TanpuraNative()
    private val audioBuffer = FloatArray(bufferSize)
    
    override fun configureFlutterEngine(flutterEngine: FlutterEngine) {
        super.configureFlutterEngine(flutterEngine)
        
        MethodChannel(flutterEngine.dartExecutor.binaryMessenger, CHANNEL).setMethodCallHandler { call, result ->
            when (call.method) {
                "initialize" -> {
                    val success = initialize()
                    result.success(success)
                }
                "start" -> {
                    startAudio()
                    result.success(true)
                }
                "stop" -> {
                    stopAudio()
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
    
    private fun initialize(): Boolean {
        return try {
            // Initialize C++ engine
            tanpuraNative.initialize(sampleRate, bufferSize)
            
            // Initialize AudioTrack with MUCH larger buffer
            val minBufferSize = AudioTrack.getMinBufferSize(
                sampleRate,
                AudioFormat.CHANNEL_OUT_STEREO,
                AudioFormat.ENCODING_PCM_FLOAT
            )
            
            // Use MUCH LARGER buffer to prevent any glitches
            val bufferSizeBytes = maxOf(minBufferSize * 4, bufferSize * 32) // Much larger
            
            android.util.Log.d("Tanpura", "Min buffer: $minBufferSize, Using: $bufferSizeBytes")
            
            val audioAttributes = AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_MEDIA)
                .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                .build()
            
            val audioFormat = AudioFormat.Builder()
                .setSampleRate(sampleRate)
                .setEncoding(AudioFormat.ENCODING_PCM_FLOAT)
                .setChannelMask(AudioFormat.CHANNEL_OUT_STEREO)
                .build()
            
            audioTrack = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                AudioTrack.Builder()
                    .setAudioAttributes(audioAttributes)
                    .setAudioFormat(audioFormat)
                    .setBufferSizeInBytes(bufferSizeBytes)
                    .setTransferMode(AudioTrack.MODE_STREAM)
                    .setPerformanceMode(AudioTrack.PERFORMANCE_MODE_NONE) // Changed from LOW_LATENCY
                    .build()
            } else {
                @Suppress("DEPRECATION")
                AudioTrack(
                    AudioAttributes.USAGE_MEDIA,
                    sampleRate,
                    AudioFormat.CHANNEL_OUT_STEREO,
                    AudioFormat.ENCODING_PCM_FLOAT,
                    bufferSizeBytes,
                    AudioTrack.MODE_STREAM
                )
            }
            
            // Set volume to MAXIMUM
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                audioTrack?.setVolume(AudioTrack.getMaxVolume())
            } else {
                @Suppress("DEPRECATION")
                audioTrack?.setStereoVolume(AudioTrack.getMaxVolume(), AudioTrack.getMaxVolume())
            }
            
            android.util.Log.d("Tanpura", "✅ Initialized: $sampleRate Hz, native C++ engine")
            android.util.Log.d("Tanpura", "🔊 Volume set to maximum: ${AudioTrack.getMaxVolume()}")
            true
        } catch (e: Exception) {
            android.util.Log.e("Tanpura", "❌ Initialization failed", e)
            false
        }
    }
    
    private fun startAudio() {
        if (isPlaying || audioTrack == null) return
        
        isPlaying = true
        
        android.util.Log.d("Tanpura", "🎵 Starting continuous drone")
        
        // Pre-generate some buffers before starting AudioTrack to prevent initial glitch
        val prerollBuffers = 8 // Increased from 5
        val prerollData = FloatArray(bufferSize * 2 * prerollBuffers)
        
        for (b in 0 until prerollBuffers) {
            tanpuraNative.generateAudio(audioBuffer)
            for (i in audioBuffer.indices) {
                val offset = b * bufferSize * 2
                prerollData[offset + i * 2] = audioBuffer[i]
                prerollData[offset + i * 2 + 1] = audioBuffer[i]
            }
        }
        
        // Now start playback
        audioTrack?.play()
        
        // Write preroll data
        audioTrack?.write(prerollData, 0, prerollData.size, AudioTrack.WRITE_BLOCKING)
        
        android.util.Log.d("Tanpura", "📊 Preroll complete, starting main audio loop")
        
        // High priority audio thread  
        audioThread = thread(start = true, name = "TanpuraAudio", priority = Thread.MAX_PRIORITY) {
            val stereoBuffer = FloatArray(bufferSize * 2)
            var consecutiveErrors = 0
            var frameCount = 0
            
            // Try to prevent thread from being interrupted
            android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO)
            
            while (isPlaying) {
                try {
                    // Generate audio in C++ (FAST!)
                    tanpuraNative.generateAudio(audioBuffer)
                    
                    // Convert mono to stereo - check for issues
                    for (i in audioBuffer.indices) {
                        val sample = audioBuffer[i]
                        
                        // Sanity check
                        if (sample.isNaN() || sample.isInfinite()) {
                            android.util.Log.e("Tanpura", "❌ Invalid sample at $i: $sample")
                            stereoBuffer[i * 2] = 0f
                            stereoBuffer[i * 2 + 1] = 0f
                        } else {
                            stereoBuffer[i * 2] = sample
                            stereoBuffer[i * 2 + 1] = sample
                        }
                    }
                    
                    // Write to AudioTrack with error checking
                    val written = audioTrack?.write(
                        stereoBuffer,
                        0,
                        stereoBuffer.size,
                        AudioTrack.WRITE_BLOCKING
                    ) ?: 0
                    
                    if (written < stereoBuffer.size) {
                        consecutiveErrors++
                        android.util.Log.w("Tanpura", "⚠️ Incomplete write: $written / ${stereoBuffer.size}")
                        
                        if (consecutiveErrors > 10) {
                            android.util.Log.e("Tanpura", "❌ Too many errors, stopping")
                            break
                        }
                    } else {
                        consecutiveErrors = 0
                    }
                    
                    if (written < 0) {
                        android.util.Log.e("Tanpura", "❌ Write error code: $written")
                        break
                    }
                    
                    frameCount++
                    
                    // Log status every 5 seconds
                    if (frameCount % 1000 == 0) {
                        val underrunCount = audioTrack?.underrunCount ?: 0
                        val playbackHeadPosition = audioTrack?.playbackHeadPosition ?: 0
                        android.util.Log.d("Tanpura", "📊 Frames: $frameCount, Underruns: $underrunCount, Head: $playbackHeadPosition")
                    }
                    
                } catch (e: Exception) {
                    android.util.Log.e("Tanpura", "❌ Audio thread error", e)
                    break
                }
            }
            
            android.util.Log.d("Tanpura", "🛑 Audio thread stopped after $frameCount frames")
        }
    }
    
    private fun stopAudio() {
        if (!isPlaying) return
        
        isPlaying = false
        audioThread?.join(1000)
        
        audioTrack?.pause()
        audioTrack?.flush()
        
        android.util.Log.d("Tanpura", "✅ Stopped")
    }
    
    override fun onDestroy() {
        stopAudio()
        audioTrack?.release()
        audioTrack = null
        tanpuraNative.destroy()
        super.onDestroy()
    }
}