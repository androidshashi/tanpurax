import 'package:flutter_test/flutter_test.dart';
import 'package:tanpura_engine/enum/engine_enums.dart';
import 'package:tanpura_engine/tanpura_engine.dart';
import 'package:tanpura_engine/tanpura_engine_platform_interface.dart';
import 'package:tanpura_engine/tanpura_engine_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockTanpuraEnginePlatform
    with MockPlatformInterfaceMixin
    implements TanpuraEnginePlatform {
  // Track state
  bool _engineRunning = false;
  bool _isPlaying = false;
  double _tempo = 1.0;
  double _volume = 1.0;
  FirstString _firstString = FirstString.sa;
  int _scale = 0;

  // Track method calls
  final List<String> _methodCalls = [];

  @override
  Future<String?> getPlatformVersion() async {
    _methodCalls.add('getPlatformVersion');
    return '42';
  }

  @override
  Future<void> initialize() async {
    _methodCalls.add('initialize');
    _engineRunning = true;
  }

  @override
  Future<void> release() async {
    _methodCalls.add('release');
    _engineRunning = false;
    _isPlaying = false;
  }

  @override
  Future<void> setTempo(double intervalSec) async {
    _methodCalls.add('setTempo');
    _tempo = intervalSec;
  }

  @override
  Future<void> play() async {
    _methodCalls.add('play');
    if (!_engineRunning) throw Exception('Engine not initialized');
    _isPlaying = true;
  }

  @override
  Future<void> pause() async {
    _methodCalls.add('pause');
    _isPlaying = false;
  }

  @override
  Future<bool> isEngineRunning() async {
    _methodCalls.add('isEngineRunning');
    return _engineRunning;
  }

  @override
  Future<bool> isPlaying() async {
    _methodCalls.add('isPlaying');
    return _isPlaying;
  }

  @override
  Future<void> setFirstString(FirstString firstString) async {
    _methodCalls.add('setFirstString');
    _firstString = firstString;
  }

  @override
  Future<void> setVolume(double volume) async {
    _methodCalls.add('setVolume');
    _volume = volume;
  }

  void resetCalls() => _methodCalls.clear();
  List<String> get methodCalls => _methodCalls;
  double get tempo => _tempo;
  double get volume => _volume;
  FirstString get firstString => _firstString;
  int get scale => _scale;

  @override
  Future<void> setScale(int scale) {
    _methodCalls.add('setScale');
    _scale = scale;
    return Future.value();
  }
}

void main() {
  final TanpuraEnginePlatform initialPlatform = TanpuraEnginePlatform.instance;

  setUp(() {
    // Reset to mock before each test
    TanpuraEnginePlatform.instance = MockTanpuraEnginePlatform();
  });

  test('$MethodChannelTanpuraEngine is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelTanpuraEngine>());
  });

  test('getPlatformVersion', () async {
    expect(await TanpuraEngine.getPlatformVersion(), '42');
  });

  group('Engine Lifecycle', () {
    test('initialize() sets engine to running', () async {
      await TanpuraEngine.initialize();
      expect(await TanpuraEnginePlatform.instance.isEngineRunning(), true);
    });

    test('release() stops engine', () async {
      await TanpuraEngine.initialize();
      await TanpuraEngine.release();
      expect(await TanpuraEnginePlatform.instance.isEngineRunning(), false);
    });
  });

  group('Playback Control', () {
    setUp(() async {
      await TanpuraEngine.initialize();
    });

    test('play() starts playback', () async {
      await TanpuraEngine.play();
      expect(await TanpuraEnginePlatform.instance.isPlaying(), true);
    });

    test('pause() stops playback', () async {
      await TanpuraEngine.play();
      await TanpuraEngine.pause();
      expect(await TanpuraEnginePlatform.instance.isPlaying(), false);
    });
  });

  group('Parameters', () {
    test('setTempo() changes tempo', () async {
      await TanpuraEngine.setTempo(2.5);
      final mock = TanpuraEnginePlatform.instance as MockTanpuraEnginePlatform;
      expect(mock.tempo, 2.5);
    });

    test('setVolume() changes volume', () async {
      await TanpuraEngine.setVolume(0.5);
      final mock = TanpuraEnginePlatform.instance as MockTanpuraEnginePlatform;
      expect(mock.volume, 0.5);
    });

    test('setFirstString() changes tonal center', () async {
      await TanpuraEngine.setFirstString(FirstString.pa);
      final mock = TanpuraEnginePlatform.instance as MockTanpuraEnginePlatform;
      expect(mock.firstString, FirstString.pa);
    });

    test('setScale() changes scale', () async {
      await TanpuraEngine.setScale(3);
      final mock = TanpuraEnginePlatform.instance as MockTanpuraEnginePlatform;
      expect(mock.scale, 3);
    });
  });
}
