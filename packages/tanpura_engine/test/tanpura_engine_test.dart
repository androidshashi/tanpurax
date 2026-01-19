import 'package:flutter_test/flutter_test.dart';
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
  Future<void> start() => Future.value();

  @override
  Future<void> stop() => Future.value();

  @override
  Future<void> setTempo(double intervalSec) => Future.value();
}

void main() {
  final TanpuraEnginePlatform initialPlatform = TanpuraEnginePlatform.instance;

  test('$MethodChannelTanpuraEngine is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelTanpuraEngine>());
  });

  test('getPlatformVersion', () async {
    TanpuraEngine tanpuraEnginePlugin = TanpuraEngine();
    MockTanpuraEnginePlatform fakePlatform = MockTanpuraEnginePlatform();
    TanpuraEnginePlatform.instance = fakePlatform;

    expect(await TanpuraEngine.getPlatformVersion(), '42');
  });
}
