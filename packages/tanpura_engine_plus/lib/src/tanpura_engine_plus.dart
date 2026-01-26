import 'dart:async';
import 'package:flutter/services.dart';
import '../tanpura_engine_plus_platform_interface.dart';
import 'tanpura_config.dart';
import 'tanpura_note.dart';

/// Callback for engine state changes
typedef EngineStateCallback = void Function(bool isPlaying);

/// Callback for string pluck events
typedef StringPluckCallback = void Function(int stringIndex);

/// Main Tanpura Engine Plus class
///
/// This class provides the interface to the native audio engine that
/// synthesizes tanpura sound using SoundFont (.sf2) and MIDI files.
///
/// Usage:
/// ```dart
/// final engine = TanpuraEnginePlus();
/// await engine.initialize();
/// await engine.setConfig(TanpuraConfig.standard(scale: Scale.c));
/// await engine.play();
/// ```
class TanpuraEnginePlus {
  static TanpuraEnginePlus? _instance;

  /// Get singleton instance
  static TanpuraEnginePlus get instance {
    _instance ??= TanpuraEnginePlus._internal();
    return _instance!;
  }

  TanpuraEnginePlus._internal();

  /// Create a new instance (for multiple engines if needed)
  factory TanpuraEnginePlus() => instance;

  // State
  bool _isInitialized = false;
  bool _isPlaying = false;
  TanpuraConfig _currentConfig = TanpuraConfig.standard();

  // Callbacks
  EngineStateCallback? _onStateChanged;
  StringPluckCallback? _onStringPluck;
  StreamSubscription? _eventSubscription;

  // ============================================================
  // Getters
  // ============================================================

  /// Whether the engine is initialized
  bool get isInitialized => _isInitialized;

  /// Whether the engine is currently playing
  bool get isPlaying => _isPlaying;

  /// Current configuration
  TanpuraConfig get currentConfig => _currentConfig;

  // ============================================================
  // Lifecycle Methods
  // ============================================================

  /// Initialize the audio engine
  ///
  /// This loads the SoundFont and prepares the audio system.
  /// Must be called before any other methods.
  ///
  /// Returns true if initialization was successful.
  Future<bool> initialize() async {
    if (_isInitialized) return true;

    try {
      final result = await TanpuraEnginePlusPlatform.instance.initialize();
      _isInitialized = result;

      if (_isInitialized) {
        _setupEventListener();
        // Apply default config
        await setConfig(_currentConfig);
      }

      return _isInitialized;
    } on PlatformException catch (e) {
      print('TanpuraEnginePlus: Failed to initialize - ${e.message}');
      return false;
    }
  }

  /// Release all resources
  ///
  /// Call this when the engine is no longer needed.
  Future<void> dispose() async {
    if (!_isInitialized) return;

    try {
      await stop();
      _eventSubscription?.cancel();
      await TanpuraEnginePlusPlatform.instance.dispose();
      _isInitialized = false;
      _isPlaying = false;
    } on PlatformException catch (e) {
      print('TanpuraEnginePlus: Failed to dispose - ${e.message}');
    }
  }

  // ============================================================
  // Playback Control
  // ============================================================

  /// Start playing the tanpura
  Future<void> play() async {
    if (!_isInitialized) {
      throw StateError('Engine not initialized. Call initialize() first.');
    }

    try {
      await TanpuraEnginePlusPlatform.instance.play();
      _isPlaying = true;
      _onStateChanged?.call(true);
    } on PlatformException catch (e) {
      print('TanpuraEnginePlus: Failed to play - ${e.message}');
    }
  }

  /// Pause playback
  Future<void> pause() async {
    try {
      await TanpuraEnginePlusPlatform.instance.pause();
      _isPlaying = false;
      _onStateChanged?.call(false);
    } on PlatformException catch (e) {
      print('TanpuraEnginePlus: Failed to pause - ${e.message}');
    }
  }

  /// Stop playback completely
  Future<void> stop() async {
    try {
      await TanpuraEnginePlusPlatform.instance.stop();
      _isPlaying = false;
      _onStateChanged?.call(false);
    } on PlatformException catch (e) {
      print('TanpuraEnginePlus: Failed to stop - ${e.message}');
    }
  }

  /// Toggle between play and pause
  Future<void> togglePlayPause() async {
    if (_isPlaying) {
      await pause();
    } else {
      await play();
    }
  }

  // ============================================================
  // Configuration Methods
  // ============================================================

  /// Apply a complete configuration
  Future<void> setConfig(TanpuraConfig config) async {
    _currentConfig = config;

    try {
      await TanpuraEnginePlusPlatform.instance.setConfig(config.toMap());
    } on PlatformException catch (e) {
      print('TanpuraEnginePlus: Failed to set config - ${e.message}');
    }
  }

  /// Set the scale (pitch/key)
  ///
  /// [scale] - The scale to use (C, C#, D, etc.)
  Future<void> setScale(Scale scale) async {
    _currentConfig = _currentConfig.copyWith(scale: scale);

    try {
      await TanpuraEnginePlusPlatform.instance.setScale(scale.semitoneOffset);
    } on PlatformException catch (e) {
      print('TanpuraEnginePlus: Failed to set scale - ${e.message}');
    }
  }

  /// Set the octave
  ///
  /// [octave] - Low (male voice), Middle, or High (female voice)
  Future<void> setOctave(TanpuraOctave octave) async {
    _currentConfig = _currentConfig.copyWith(octave: octave);

    try {
      await TanpuraEnginePlusPlatform.instance.setOctave(octave.value);
    } on PlatformException catch (e) {
      print('TanpuraEnginePlus: Failed to set octave - ${e.message}');
    }
  }

  /// Set the master volume
  ///
  /// [volume] - Volume level from 0.0 (silent) to 1.0 (full)
  Future<void> setVolume(double volume) async {
    volume = volume.clamp(0.0, 1.0);
    _currentConfig = _currentConfig.copyWith(masterVolume: volume);

    try {
      await TanpuraEnginePlusPlatform.instance.setVolume(volume);
    } on PlatformException catch (e) {
      print('TanpuraEnginePlus: Failed to set volume - ${e.message}');
    }
  }

  /// Set the tempo (time between string plucks)
  ///
  /// [seconds] - Interval in seconds (typically 0.3 to 1.0)
  Future<void> setTempo(double seconds) async {
    seconds = seconds.clamp(0.2, 2.0);
    _currentConfig = _currentConfig.copyWith(tempo: seconds);

    try {
      await TanpuraEnginePlusPlatform.instance.setTempo(seconds);
    } on PlatformException catch (e) {
      print('TanpuraEnginePlus: Failed to set tempo - ${e.message}');
    }
  }

  /// Set the first string note
  ///
  /// [note] - Usually Pa, Ma, or Ni
  Future<void> setFirstString(TanpuraNote note) async {
    final strings = List<StringConfig>.from(_currentConfig.strings);
    strings[0] = strings[0].copyWith(note: note);
    _currentConfig = _currentConfig.copyWith(strings: strings);

    try {
      await TanpuraEnginePlusPlatform.instance.setFirstString(
        note.semitoneOffset,
      );
    } on PlatformException catch (e) {
      print('TanpuraEnginePlus: Failed to set first string - ${e.message}');
    }
  }

  /// Configure a specific string
  ///
  /// [stringIndex] - Index 0-3 (0=first string, 3=kharaj/bass)
  /// [config] - The string configuration
  Future<void> setStringConfig(int stringIndex, StringConfig config) async {
    if (stringIndex < 0 || stringIndex >= 4) {
      throw ArgumentError('stringIndex must be 0-3');
    }

    final strings = List<StringConfig>.from(_currentConfig.strings);
    strings[stringIndex] = config;
    _currentConfig = _currentConfig.copyWith(strings: strings);

    try {
      await TanpuraEnginePlusPlatform.instance.setStringConfig(
        stringIndex,
        config.toMap(),
      );
    } on PlatformException catch (e) {
      print('TanpuraEnginePlus: Failed to set string config - ${e.message}');
    }
  }

  /// Enable or disable a specific string
  Future<void> setStringEnabled(int stringIndex, bool enabled) async {
    if (stringIndex < 0 || stringIndex >= 4) {
      throw ArgumentError('stringIndex must be 0-3');
    }

    final strings = List<StringConfig>.from(_currentConfig.strings);
    strings[stringIndex] = strings[stringIndex].copyWith(enabled: enabled);
    _currentConfig = _currentConfig.copyWith(strings: strings);

    try {
      await TanpuraEnginePlusPlatform.instance.setStringEnabled(
        stringIndex,
        enabled,
      );
    } on PlatformException catch (e) {
      print('TanpuraEnginePlus: Failed to set string enabled - ${e.message}');
    }
  }

  /// Set fine tuning for a string in cents
  Future<void> setStringFineTune(int stringIndex, double cents) async {
    if (stringIndex < 0 || stringIndex >= 4) {
      throw ArgumentError('stringIndex must be 0-3');
    }

    cents = cents.clamp(-100.0, 100.0);
    final strings = List<StringConfig>.from(_currentConfig.strings);
    strings[stringIndex] = strings[stringIndex].copyWith(fineTune: cents);
    _currentConfig = _currentConfig.copyWith(strings: strings);

    try {
      await TanpuraEnginePlusPlatform.instance.setStringFineTune(
        stringIndex,
        cents,
      );
    } on PlatformException catch (e) {
      print('TanpuraEnginePlus: Failed to set fine tune - ${e.message}');
    }
  }

  // ============================================================
  // Preset Configurations
  // ============================================================

  /// Apply standard Pa-Sa-Sa-Sa configuration
  Future<void> applyStandardConfig({Scale scale = Scale.c}) async {
    await setConfig(TanpuraConfig.standard(scale: scale));
  }

  /// Apply Ma-Sa-Sa-Sa configuration
  Future<void> applyMaConfig({Scale scale = Scale.c}) async {
    await setConfig(TanpuraConfig.withMa(scale: scale));
  }

  /// Apply Ni-Sa-Sa-Sa configuration
  Future<void> applyNiConfig({Scale scale = Scale.c}) async {
    await setConfig(TanpuraConfig.withNi(scale: scale));
  }

  // ============================================================
  // Callbacks
  // ============================================================

  /// Set callback for play/pause state changes
  void setOnStateChanged(EngineStateCallback? callback) {
    _onStateChanged = callback;
  }

  /// Set callback for string pluck events (for UI animation)
  void setOnStringPluck(StringPluckCallback? callback) {
    _onStringPluck = callback;
  }

  void _setupEventListener() {
    _eventSubscription = TanpuraEnginePlusPlatform.instance.eventStream.listen(
      (event) {
        final type = event['type'] as String?;
        if (type == 'stringPluck') {
          final index = event['stringIndex'] as int?;
          if (index != null) {
            _onStringPluck?.call(index);
          }
        } else if (type == 'stateChanged') {
          final playing = event['isPlaying'] as bool?;
          if (playing != null) {
            _isPlaying = playing;
            _onStateChanged?.call(playing);
          }
        }
      },
      onError: (error) {
        print('TanpuraEnginePlus: Event stream error - $error');
      },
    );
  }

  // ============================================================
  // Utility Methods
  // ============================================================

  /// Get available MIDI files
  Future<List<String>> getAvailableMidiFiles() async {
    try {
      return await TanpuraEnginePlusPlatform.instance.getAvailableMidiFiles();
    } on PlatformException catch (e) {
      print('TanpuraEnginePlus: Failed to get MIDI files - ${e.message}');
      return [];
    }
  }

  /// Check if a specific note MIDI file is available
  Future<bool> isNoteAvailable(TanpuraNote note) async {
    try {
      return await TanpuraEnginePlusPlatform.instance.isNoteAvailable(
        note.midiFile,
      );
    } on PlatformException {
      return false;
    }
  }

  /// Get engine info (sample rate, buffer size, etc.)
  Future<Map<String, dynamic>> getEngineInfo() async {
    try {
      return await TanpuraEnginePlusPlatform.instance.getEngineInfo();
    } on PlatformException catch (e) {
      print('TanpuraEnginePlus: Failed to get engine info - ${e.message}');
      return {};
    }
  }
}
