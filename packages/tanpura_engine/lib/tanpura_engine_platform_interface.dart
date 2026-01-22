import 'package:plugin_platform_interface/plugin_platform_interface.dart';
import 'package:tanpura_engine/enum/engine_enums.dart';

import 'tanpura_engine_method_channel.dart';

/// Platform interface for TanpuraEngine.
///
/// This defines the contract that all platform implementations
/// (MethodChannel, FFI, iOS, etc.) must follow.
abstract class TanpuraEnginePlatform extends PlatformInterface {
  TanpuraEnginePlatform() : super(token: _token);

  static final Object _token = Object();

  static TanpuraEnginePlatform _instance = MethodChannelTanpuraEngine();

  static TanpuraEnginePlatform get instance => _instance;

  static set instance(TanpuraEnginePlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  // ------------------------------------------------------------
  // Common methods
  // ------------------------------------------------------------

  /// Returns the platform version.

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('getPlatformVersion() has not been implemented.');
  }

  // ------------------------------------------------------------
  // Engine lifecycle
  // ------------------------------------------------------------

  /// Initializes the native audio engine.
  ///
  /// Safe to call multiple times.
  Future<void> initialize() {
    throw UnimplementedError('initialize() has not been implemented.');
  }

  /// Releases native resources and stops the engine.
  Future<void> release() {
    throw UnimplementedError('release() has not been implemented.');
  }

  /// Returns true if the native audio engine is running.
  Future<bool> isEngineRunning() {
    throw UnimplementedError('isEngineRunning() has not been implemented.');
  }

  // ------------------------------------------------------------
  // Playback lifecycle
  // ------------------------------------------------------------

  /// Starts tanpura sound playback.
  Future<void> play() {
    throw UnimplementedError('play() has not been implemented.');
  }

  /// Pauses tanpura sound playback.
  Future<void> pause() {
    throw UnimplementedError('pause() has not been implemented.');
  }

  /// Returns true if tanpura sound is currently playing.
  Future<bool> isPlaying() {
    throw UnimplementedError('isPlaying() has not been implemented.');
  }

  // ------------------------------------------------------------
  // Parameters
  // ------------------------------------------------------------

  /// Sets tempo (energy refresh interval in seconds).
  Future<void> setTempo(double intervalSec) {
    throw UnimplementedError('setTempo() has not been implemented.');
  }

  /// Sets the first string (tonal center) of the tanpura.
  ///
  /// This affects the base pitch of the entire drone.
  Future<void> setFirstString(FirstString firstString) {
    throw UnimplementedError('setFirstString() has not been implemented.');
  }

  /// Sets the master volume (0.0 to 1.0).
  Future<void> setVolume(double volume) {
    throw UnimplementedError('setVolume() has not been implemented.');
  }
}
