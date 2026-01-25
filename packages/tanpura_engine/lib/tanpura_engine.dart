import 'package:tanpura_engine/enum/engine_enums.dart';

import 'tanpura_engine_platform_interface.dart';

/// Public API for the Tanpura audio engine.
///
/// This API separates:
/// - Engine lifecycle (initialize / release)
/// - Playback lifecycle (play / pause)
class TanpuraEngine {
  TanpuraEngine._(); // no instances

  /// Sets the first string (tonal center) of the tanpura.
  ///
  /// This affects the base pitch of the entire drone.
  static Future<void> setFirstString(FirstString firstString) {
    return TanpuraEnginePlatform.instance.setFirstString(firstString);
  }

  /// Returns the platform version.
  static Future<String?> getPlatformVersion() {
    return TanpuraEnginePlatform.instance.getPlatformVersion();
  }

  /// Initializes the native audio engine.
  ///
  /// Safe to call multiple times.
  /// Does NOT start sound playback.
  static Future<void> initialize() {
    return TanpuraEnginePlatform.instance.initialize();
  }

  /// Starts tanpura sound playback.
  ///
  /// Engine must be initialized.
  static Future<void> play() {
    return TanpuraEnginePlatform.instance.play();
  }

  /// Pauses tanpura sound playback.
  ///
  /// Engine remains alive.
  static Future<void> pause() {
    return TanpuraEnginePlatform.instance.pause();
  }

  /// Releases native resources.
  ///
  /// Should be called when the app is done with audio.
  static Future<void> release() {
    return TanpuraEnginePlatform.instance.release();
  }

  /// Sets tempo (energy refresh interval in seconds).
  static Future<void> setTempo(double intervalSec) {
    assert(intervalSec > 0);
    return TanpuraEnginePlatform.instance.setTempo(intervalSec);
  }

  /// Returns true if the native audio engine is running.
  static Future<bool> isEngineRunning() {
    return TanpuraEnginePlatform.instance.isEngineRunning();
  }

  /// Returns true if tanpura sound is currently playing.
  static Future<bool> isPlaying() {
    return TanpuraEnginePlatform.instance.isPlaying();
  }

  /// Sets the master volume (0.0 to 1.0).
  static Future<void> setVolume(double volume) {
    return TanpuraEnginePlatform.instance.setVolume(volume);
  }

  /// Sets the scale (C, C#, D, ...).
  static Future<void> setScale(int scale) {
    return TanpuraEnginePlatform.instance.setScale(scale);
  }

  /// Sets the octave shift (-1, 0, +1).
  static Future<void> setOctave(int octave) {
    return TanpuraEnginePlatform.instance.setOctave(octave);
  }
}
