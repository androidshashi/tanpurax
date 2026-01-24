import 'package:flutter_test/flutter_test.dart';
import 'package:tanpura_engine/enum/engine_enums.dart';
import 'package:tanpura_engine/tanpura_engine.dart';
import 'package:tanpura_engine/tanpura_engine_platform_interface.dart';
import 'package:tanpura_engine/tanpura_engine_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockTanpuraEnginePlatform
    with MockPlatformInterfaceMixin
    implements TanpuraEnginePlatform {
  @override
  Future<String?> getPlatformVersion() => Future.value('42');

  @override
  Future<void> initialize() => Future.value();

  @override
  Future<void> release() => Future.value();

  @override
  Future<void> setTempo(double intervalSec) => Future.value();

  @override
  Future<bool> isEngineRunning() {
    // TODO: implement isEngineRunning
    throw UnimplementedError();
  }

  @override
  Future<bool> isPlaying() {
    // TODO: implement isPlaying
    throw UnimplementedError();
  }

  @override
  Future<void> pause() {
    // TODO: implement pause
    throw UnimplementedError();
  }

  @override
  Future<void> play() {
    // TODO: implement play
    throw UnimplementedError();
  }

  @override
  Future<void> setFirstString(FirstString firstString) {
    // TODO: implement setFirstString
    throw UnimplementedError();
  }

  @override
  Future<void> setVolume(double volume) {
    // TODO: implement setVolume
    throw UnimplementedError();
  }

  @override
  Future<bool> exportWav(String filePath, double durationSec) {
    // TODO: implement exportWav
    throw UnimplementedError();
  }

  @override
  Future<void> setScale(Scale scale) {
    // TODO: implement setScale
    throw UnimplementedError();
  }
}

void main() {
  final TanpuraEnginePlatform initialPlatform = TanpuraEnginePlatform.instance;

  test('$MethodChannelTanpuraEngine is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelTanpuraEngine>());
  });

  // test('getPlatformVersion', () async {
  //   TanpuraEngine tanpuraEnginePlugin = TanpuraEngine();
  //   MockTanpuraEnginePlatform fakePlatform = MockTanpuraEnginePlatform();
  //   TanpuraEnginePlatform.instance = fakePlatform;

  //   expect(await TanpuraEngine.getPlatformVersion(), '42');
  // });
}
