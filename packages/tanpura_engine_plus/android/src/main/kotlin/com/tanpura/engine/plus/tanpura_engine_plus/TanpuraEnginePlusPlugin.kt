package com.tanpura.engine.plus.tanpura_engine_plus

import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.MethodChannel.Result
import android.content.Context
import android.os.Handler
import android.os.Looper
import androidx.annotation.NonNull
import io.flutter.plugin.common.EventChannel
import org.json.JSONObject
import android.content.res.AssetManager

/** TanpuraEnginePlusPlugin - Native audio for tanpura synthesis */
class TanpuraEnginePlusPlugin: FlutterPlugin, MethodCallHandler, EventChannel.StreamHandler {
    private lateinit var methodChannel: MethodChannel
    private lateinit var eventChannel: EventChannel
    private lateinit var context: Context
    private var eventSink: EventChannel.EventSink? = null
    private val mainHandler = Handler(Looper.getMainLooper())

    companion object {
        init {
            System.loadLibrary("tanpura_native")
        }
    }

    // Native bindings (C++ engine uses SoundFont + MIDI)
    private external fun nativeInitialize(assetManager: AssetManager): Boolean
    private external fun nativeDispose()

    private external fun nativePlay()
    private external fun nativePause()
    private external fun nativeStop()
    private external fun nativeIsPlaying(): Boolean

    private external fun nativeSetConfig(configJson: String)
    private external fun nativeSetScale(scaleIndex: Int)
    private external fun nativeSetOctave(octaveValue: Int)
    private external fun nativeSetVolume(volume: Float)
    private external fun nativeSetTempo(seconds: Float)
    private external fun nativeSetFirstString(noteIndex: Int)
    private external fun nativeSetStringConfig(
        stringIndex: Int,
        noteIndex: Int,
        volume: Float,
        pan: Float,
        fineTune: Float,
        enabled: Boolean
    )
    private external fun nativeSetStringEnabled(stringIndex: Int, enabled: Boolean)
    private external fun nativeSetStringFineTune(stringIndex: Int, cents: Float)

    private external fun nativeGetAvailableMidiFiles(): Array<String>
    private external fun nativeIsNoteAvailable(midiFile: String): Boolean
    private external fun nativeGetEngineInfo(): String

    // Callback from native for string pluck events
    @Suppress("unused")
    fun onStringPluck(stringIndex: Int) {
        mainHandler.post {
            eventSink?.success(mapOf(
                "type" to "stringPluck",
                "stringIndex" to stringIndex
            ))
        }
    }

    // Callback from native for state changes
    @Suppress("unused")
    fun onStateChanged(isPlaying: Boolean) {
        mainHandler.post {
            eventSink?.success(mapOf(
                "type" to "stateChanged",
                "isPlaying" to isPlaying
            ))
        }
    }

    // ============================================================
    // FlutterPlugin Implementation
    // ============================================================

    override fun onAttachedToEngine(@NonNull binding: FlutterPlugin.FlutterPluginBinding) {
        context = binding.applicationContext
        
        methodChannel = MethodChannel(binding.binaryMessenger, "com.tanpura.engine/methods")
        methodChannel.setMethodCallHandler(this)
        
        eventChannel = EventChannel(binding.binaryMessenger, "com.tanpura.engine/events")
        eventChannel.setStreamHandler(this)
    }

    override fun onDetachedFromEngine(@NonNull binding: FlutterPlugin.FlutterPluginBinding) {
        methodChannel.setMethodCallHandler(null)
        eventChannel.setStreamHandler(null)
    }

    // ============================================================
    // MethodCallHandler Implementation
    // ============================================================

    override fun onMethodCall(@NonNull call: MethodCall, @NonNull result: Result) {
        when (call.method) {
            // Lifecycle
            "initialize" -> {
                val ok = nativeInitialize(context.assets)
                result.success(ok)
            }
            "dispose" -> {
                nativeDispose()
                result.success(null)
            }
            
            // Playback
            "play" -> {
                nativePlay()
                result.success(null)
            }
            "pause" -> {
                nativePause()
                result.success(null)
            }
            "stop" -> {
                nativeStop()
                result.success(null)
            }
            
            // Configuration
            "setConfig" -> {
                val configMap = call.arguments as? Map<*, *>
                if (configMap != null) {
                    val json = mapToJson(configMap)
                    nativeSetConfig(json)
                }
                result.success(null)
            }
            "setScale" -> {
                val index = call.argument<Int>("index") ?: 0
                nativeSetScale(index)
                result.success(null)
            }
            "setOctave" -> {
                val value = call.argument<Int>("value") ?: 0
                nativeSetOctave(value)
                result.success(null)
            }
            "setVolume" -> {
                val volume = call.argument<Double>("volume")?.toFloat() ?: 0.85f
                nativeSetVolume(volume)
                result.success(null)
            }
            "setTempo" -> {
                val seconds = call.argument<Double>("seconds")?.toFloat() ?: 0.4f
                nativeSetTempo(seconds)
                result.success(null)
            }
            "setFirstString" -> {
                val noteIndex = call.argument<Int>("noteIndex") ?: 7
                nativeSetFirstString(noteIndex)
                result.success(null)
            }
            "setStringConfig" -> {
                val stringIndex = call.argument<Int>("stringIndex") ?: 0
                val noteIndex = call.argument<Int>("noteIndex") ?: 0
                val volume = call.argument<Double>("volume")?.toFloat() ?: 0.25f
                val pan = call.argument<Double>("pan")?.toFloat() ?: 0.0f
                val fineTune = call.argument<Double>("fineTune")?.toFloat() ?: 0.0f
                val enabled = call.argument<Boolean>("enabled") ?: true
                nativeSetStringConfig(stringIndex, noteIndex, volume, pan, fineTune, enabled)
                result.success(null)
            }
            "setStringEnabled" -> {
                val stringIndex = call.argument<Int>("stringIndex") ?: 0
                val enabled = call.argument<Boolean>("enabled") ?: true
                nativeSetStringEnabled(stringIndex, enabled)
                result.success(null)
            }
            "setStringFineTune" -> {
                val stringIndex = call.argument<Int>("stringIndex") ?: 0
                val cents = call.argument<Double>("cents")?.toFloat() ?: 0.0f
                nativeSetStringFineTune(stringIndex, cents)
                result.success(null)
            }
            
            // Info
            "getAvailableMidiFiles" -> {
                val files = nativeGetAvailableMidiFiles()
                result.success(files.toList())
            }
            "isNoteAvailable" -> {
                val midiFile = call.argument<String>("midiFile") ?: ""
                val available = nativeIsNoteAvailable(midiFile)
                result.success(available)
            }
            "getEngineInfo" -> {
                val info = nativeGetEngineInfo()
                result.success(jsonToMap(info))
            }
            
            else -> result.notImplemented()
        }
    }

    // ============================================================
    // EventChannel.StreamHandler Implementation
    // ============================================================

    override fun onListen(arguments: Any?, events: EventChannel.EventSink?) {
        eventSink = events
    }

    override fun onCancel(arguments: Any?) {
        eventSink = null
    }

    // ============================================================
    // JSON helpers
    // ============================================================

    private fun mapToJson(map: Map<*, *>): String {
        val sb = StringBuilder("{")
        var first = true
        for ((key, value) in map) {
            if (!first) sb.append(",")
            first = false
            sb.append("\"$key\":")
            when (value) {
                is String -> sb.append("\"${value}\"")
                is Number -> sb.append(value)
                is Boolean -> sb.append(value)
                is List<*> -> sb.append(listToJson(value))
                is Map<*, *> -> sb.append(mapToJson(value))
                else -> sb.append("null")
            }
        }
        sb.append("}")
        return sb.toString()
    }

    private fun listToJson(list: List<*>): String {
        val sb = StringBuilder("[")
        var first = true
        for (item in list) {
            if (!first) sb.append(",")
            first = false
            when (item) {
                is String -> sb.append("\"${item}\"")
                is Number -> sb.append(item)
                is Boolean -> sb.append(item)
                is List<*> -> sb.append(listToJson(item))
                is Map<*, *> -> sb.append(mapToJson(item))
                else -> sb.append("null")
            }
        }
        sb.append("]")
        return sb.toString()
    }

    private fun jsonToMap(json: String): Map<String, Any> {
        return try {
            val result = mutableMapOf<String, Any>()
            val obj = JSONObject(json)
            val keys = obj.keys()
            while (keys.hasNext()) {
                val k = keys.next()
                result[k] = obj.get(k)
            }
            result
        } catch (e: Exception) {
            emptyMap()
        }
    }
}