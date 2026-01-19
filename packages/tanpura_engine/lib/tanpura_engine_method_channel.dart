import 'package:flutter/services.dart';
import 'dart:io' show Platform;
import 'tanpura_engine_platform_interface.dart';

class MethodChannelTanpuraEngine extends TanpuraEnginePlatform {
  static const MethodChannel _channel = MethodChannel(
    'com.tanpurax.tanpura/tanpura_audio',
  );

  @override
  Future<void> start() async {
    await _channel.invokeMethod('start');
  }

  @override
  Future<void> stop() async {
    await _channel.invokeMethod('stop');
  }

  @override
  Future<void> setTempo(double intervalSec) async {
    await _channel.invokeMethod('set_tempo', {'interval_sec': intervalSec});
  }

  @override
  Future<String?> getPlatformVersion() async {
    return Platform.operatingSystemVersion;
  }
}
