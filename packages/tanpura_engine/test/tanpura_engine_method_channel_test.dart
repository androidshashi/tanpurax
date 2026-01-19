import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:tanpura_engine/tanpura_engine_method_channel.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  MethodChannelTanpuraEngine platform = MethodChannelTanpuraEngine();
  const MethodChannel channel = MethodChannel(
    'com.tanpurax.tanpura/tanpura_audio',
  );

  setUp(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
        .setMockMethodCallHandler(channel, (MethodCall methodCall) async {
          return '42';
        });
  });

  tearDown(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
        .setMockMethodCallHandler(channel, null);
  });

  test('getPlatformVersion', () async {
    expect(await platform.getPlatformVersion(), isNotNull);
    expect(await platform.getPlatformVersion(), isA<String>());
  });
}
