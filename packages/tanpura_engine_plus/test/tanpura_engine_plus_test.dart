import 'package:flutter_test/flutter_test.dart';
import 'package:tanpura_engine_plus/tanpura_engine_plus.dart';
import 'package:tanpura_engine_plus/tanpura_engine_plus_platform_interface.dart';
import 'package:tanpura_engine_plus/tanpura_engine_plus_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockTanpuraEnginePlusPlatform
    with MockPlatformInterfaceMixin
    implements TanpuraEnginePlusPlatform {
  @override
  Future<bool> initialize() => Future.value(true);

  @override
  Future<void> dispose() => Future.value();

  @override
  Future<void> play() => Future.value();

  @override
  Future<void> pause() => Future.value();

  @override
  Future<void> stop() => Future.value();

  @override
  Future<void> setConfig(Map<String, dynamic> config) => Future.value();

  @override
  Future<void> setScale(int index) => Future.value();

  @override
  Future<void> setOctave(int value) => Future.value();

  @override
  Future<void> setVolume(double volume) => Future.value();

  @override
  Future<void> setTempo(double seconds) => Future.value();

  @override
  Future<void> setFirstString(int noteIndex) => Future.value();

  @override
  Future<void> setStringConfig(int stringIndex, Map<String, dynamic> config) =>
      Future.value();

  @override
  Future<void> setStringEnabled(int stringIndex, bool enabled) =>
      Future.value();

  @override
  Future<void> setStringFineTune(int stringIndex, double cents) =>
      Future.value();

  @override
  Future<List<String>> getAvailableMidiFiles() => Future.value([]);

  @override
  Future<bool> isNoteAvailable(String midiFile) => Future.value(false);

  @override
  Future<Map<String, dynamic>> getEngineInfo() => Future.value({});

  @override
  Stream<Map<String, dynamic>> get eventStream => Stream.empty();
}

void main() {
  final TanpuraEnginePlusPlatform initialPlatform =
      TanpuraEnginePlusPlatform.instance;

  test('$MethodChannelTanpuraEnginePlus is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelTanpuraEnginePlus>());
  });

  test('initialize', () async {
    MockTanpuraEnginePlusPlatform fakePlatform =
        MockTanpuraEnginePlusPlatform();
    TanpuraEnginePlusPlatform.instance = fakePlatform;

    expect(await TanpuraEnginePlus.instance.initialize(), true);
  });
}
