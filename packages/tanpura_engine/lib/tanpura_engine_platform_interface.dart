import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'tanpura_engine_method_channel.dart';

abstract class TanpuraEnginePlatform extends PlatformInterface {
  TanpuraEnginePlatform() : super(token: _token);

  static final Object _token = Object();

  static TanpuraEnginePlatform _instance = MethodChannelTanpuraEngine();

  static TanpuraEnginePlatform get instance => _instance;

  static set instance(TanpuraEnginePlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<void> start() {
    throw UnimplementedError('start() has not been implemented.');
  }

  Future<void> stop() {
    throw UnimplementedError('stop() has not been implemented.');
  }

  Future<void> setTempo(double intervalSec) {
    throw UnimplementedError('setTempo() has not been implemented.');
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('getPlatformVersion() has not been implemented.');
  }
}
