import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'tanpura_engine_plus_platform_interface.dart';

/// An implementation of [TanpuraEnginePlusPlatform] that uses method channels.
class MethodChannelTanpuraEnginePlus extends TanpuraEnginePlusPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('com.tanpura.engine/methods');

  /// The event channel for streaming events from native.
  @visibleForTesting
  final eventChannel = const EventChannel('com.tanpura.engine/events');

  @override
  Future<bool> initialize() async {
    final result = await methodChannel.invokeMethod<bool>('initialize');
    return result ?? false;
  }

  @override
  Future<void> dispose() async {
    await methodChannel.invokeMethod('dispose');
  }

  @override
  Future<void> play() async {
    await methodChannel.invokeMethod('play');
  }

  @override
  Future<void> pause() async {
    await methodChannel.invokeMethod('pause');
  }

  @override
  Future<void> stop() async {
    await methodChannel.invokeMethod('stop');
  }

  @override
  Future<void> setConfig(Map<String, dynamic> config) async {
    await methodChannel.invokeMethod('setConfig', config);
  }

  @override
  Future<void> setScale(int index) async {
    await methodChannel.invokeMethod('setScale', {'index': index});
  }

  @override
  Future<void> setOctave(int value) async {
    await methodChannel.invokeMethod('setOctave', {'value': value});
  }

  @override
  Future<void> setVolume(double volume) async {
    await methodChannel.invokeMethod('setVolume', {'volume': volume});
  }

  @override
  Future<void> setTempo(double seconds) async {
    await methodChannel.invokeMethod('setTempo', {'seconds': seconds});
  }

  @override
  Future<void> setFirstString(int noteIndex) async {
    await methodChannel.invokeMethod('setFirstString', {
      'noteIndex': noteIndex,
    });
  }

  @override
  Future<void> setStringConfig(
    int stringIndex,
    Map<String, dynamic> config,
  ) async {
    await methodChannel.invokeMethod('setStringConfig', {
      'stringIndex': stringIndex,
      ...config,
    });
  }

  @override
  Future<void> setStringEnabled(int stringIndex, bool enabled) async {
    await methodChannel.invokeMethod('setStringEnabled', {
      'stringIndex': stringIndex,
      'enabled': enabled,
    });
  }

  @override
  Future<void> setStringFineTune(int stringIndex, double cents) async {
    await methodChannel.invokeMethod('setStringFineTune', {
      'stringIndex': stringIndex,
      'cents': cents,
    });
  }

  @override
  Future<List<String>> getAvailableMidiFiles() async {
    final result = await methodChannel.invokeMethod<List>(
      'getAvailableMidiFiles',
    );
    return result?.cast<String>() ?? [];
  }

  @override
  Future<bool> isNoteAvailable(String midiFile) async {
    final result = await methodChannel.invokeMethod<bool>('isNoteAvailable', {
      'midiFile': midiFile,
    });
    return result ?? false;
  }

  @override
  Future<Map<String, dynamic>> getEngineInfo() async {
    final result = await methodChannel.invokeMethod<Map>('getEngineInfo');
    return result?.cast<String, dynamic>() ?? {};
  }

  @override
  Stream<Map<String, dynamic>> get eventStream {
    return eventChannel.receiveBroadcastStream().map(
      (event) => Map<String, dynamic>.from(event as Map),
    );
  }
}
