import 'package:flutter/services.dart';
import 'package:tanpura_engine/enum/engine_enums.dart';
import 'dart:io' show Platform;
import 'tanpura_engine_platform_interface.dart';

/// MethodChannel-based implementation of TanpuraEnginePlatform.
class MethodChannelTanpuraEngine extends TanpuraEnginePlatform {
  static const MethodChannel _channel = MethodChannel(
    'com.tanpurax.tanpura/tanpura_audio',
  );

  // ------------------------------------------------------------
  // Common methods
  // ------------------------------------------------------------

  @override
  Future<String?> getPlatformVersion() async {
    return Platform.operatingSystemVersion;
  }

  // ------------------------------------------------------------
  // Engine lifecycle
  // ------------------------------------------------------------

  @override
  Future<void> initialize() async {
    await _channel.invokeMethod('initialize');
  }

  @override
  Future<void> release() async {
    await _channel.invokeMethod('release');
  }

  @override
  Future<bool> isEngineRunning() async {
    final result = await _channel.invokeMethod<bool>('is_engine_running');
    return result ?? false;
  }

  // ------------------------------------------------------------
  // Playback lifecycle
  // ------------------------------------------------------------

  @override
  Future<void> play() async {
    await _channel.invokeMethod('play');
  }

  @override
  Future<void> pause() async {
    await _channel.invokeMethod('pause');
  }

  @override
  Future<bool> isPlaying() async {
    final result = await _channel.invokeMethod<bool>('is_playing');
    return result ?? false;
  }

  // ------------------------------------------------------------
  // Parameters
  // ------------------------------------------------------------

  @override
  Future<void> setTempo(double intervalSec) async {
    await _channel.invokeMethod('set_tempo', {'interval_sec': intervalSec});
  }

  @override
  Future<void> setFirstString(FirstString firstString) async {
    await _channel.invokeMethod('set_first_string', {
      'value': firstString.index,
    });
  }

  @override
  Future<void> setVolume(double volume) async {
    await _channel.invokeMethod('set_volume', {'volume': volume});
  }

  @override
  Future<void> setScale(Scale scale) async {
    await _channel.invokeMethod('set_scale', {'value': scale.index});
  }

  @override
  Future<void> setOctave(Octave octave) async {
    // 0 = Low, 1 = Mid, 2 = High
    await _channel.invokeMethod('set_octave', {'value': octave.index});
  }

  @override
  Future<Octave> getOctave() async {
    final result = await _channel.invokeMethod<int>('get_octave');
    return Octave.values[result ?? 1]; // Default to mid if null
  }

  @override
  Future<bool> exportWav(String filePath, double durationSec) async {
    final result = await _channel.invokeMethod<bool>('export_wav', {
      'file_path': filePath,
      'duration': durationSec,
    });
    return result ?? false;
  }
}
