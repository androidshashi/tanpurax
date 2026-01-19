// You have generated a new plugin project without specifying the `--platforms`
// flag. A plugin project with no platform support was generated. To add a
// platform, run `flutter create -t plugin --platforms <platforms> .` under the
// same directory. You can also find a detailed instruction on how to add
// platforms in the `pubspec.yaml` at
// https://flutter.dev/to/pubspec-plugin-platforms.

import 'tanpura_engine_platform_interface.dart';

/// This is Tanpura engine
/// Provides methods to start, stop, and set tempo of the Tanpura sound.
class TanpuraEngine {
  /// Starts the Tanpura sound.
  static Future<void> start() {
    return TanpuraEnginePlatform.instance.start();
  }

  /// Stops the Tanpura sound.
  static Future<void> stop() {
    return TanpuraEnginePlatform.instance.stop();
  }

  /// Sets the tempo of the Tanpura sound.
  /// [intervalSec] is the interval in seconds.
  static Future<void> setTempo(double intervalSec) {
    return TanpuraEnginePlatform.instance.setTempo(intervalSec);
  }

  /// Gets the platform version.
  static Future<String?> getPlatformVersion() {
    return TanpuraEnginePlatform.instance.getPlatformVersion();
  }
}
