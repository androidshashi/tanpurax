import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'tanpura_engine_plus_method_channel.dart';

abstract class TanpuraEnginePlusPlatform extends PlatformInterface {
  /// Constructs a TanpuraEnginePlusPlatform.
  TanpuraEnginePlusPlatform() : super(token: _token);

  static final Object _token = Object();

  static TanpuraEnginePlusPlatform _instance = MethodChannelTanpuraEnginePlus();

  /// The default instance of [TanpuraEnginePlusPlatform] to use.
  ///
  /// Defaults to [MethodChannelTanpuraEnginePlus].
  static TanpuraEnginePlusPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [TanpuraEnginePlusPlatform] when
  /// they register themselves.
  static set instance(TanpuraEnginePlusPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  // Lifecycle
  Future<bool> initialize() {
    throw UnimplementedError('initialize() has not been implemented.');
  }

  Future<void> dispose() {
    throw UnimplementedError('dispose() has not been implemented.');
  }

  // Playback
  Future<void> play() {
    throw UnimplementedError('play() has not been implemented.');
  }

  Future<void> pause() {
    throw UnimplementedError('pause() has not been implemented.');
  }

  Future<void> stop() {
    throw UnimplementedError('stop() has not been implemented.');
  }

  // Configuration
  Future<void> setConfig(Map<String, dynamic> config) {
    throw UnimplementedError('setConfig() has not been implemented.');
  }

  Future<void> setScale(int index) {
    throw UnimplementedError('setScale() has not been implemented.');
  }

  Future<void> setOctave(int value) {
    throw UnimplementedError('setOctave() has not been implemented.');
  }

  Future<void> setVolume(double volume) {
    throw UnimplementedError('setVolume() has not been implemented.');
  }

  Future<void> setTempo(double seconds) {
    throw UnimplementedError('setTempo() has not been implemented.');
  }

  Future<void> setFirstString(int noteIndex) {
    throw UnimplementedError('setFirstString() has not been implemented.');
  }

  Future<void> setStringConfig(int stringIndex, Map<String, dynamic> config) {
    throw UnimplementedError('setStringConfig() has not been implemented.');
  }

  Future<void> setStringEnabled(int stringIndex, bool enabled) {
    throw UnimplementedError('setStringEnabled() has not been implemented.');
  }

  Future<void> setStringFineTune(int stringIndex, double cents) {
    throw UnimplementedError('setStringFineTune() has not been implemented.');
  }

  // Info
  Future<List<String>> getAvailableMidiFiles() {
    throw UnimplementedError(
      'getAvailableMidiFiles() has not been implemented.',
    );
  }

  Future<bool> isNoteAvailable(String midiFile) {
    throw UnimplementedError('isNoteAvailable() has not been implemented.');
  }

  Future<Map<String, dynamic>> getEngineInfo() {
    throw UnimplementedError('getEngineInfo() has not been implemented.');
  }

  // Events
  Stream<Map<String, dynamic>> get eventStream {
    throw UnimplementedError('eventStream has not been implemented.');
  }
}
