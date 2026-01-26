import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:tanpura_engine_plus/tanpura_engine_plus_method_channel.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  MethodChannelTanpuraEnginePlus platform = MethodChannelTanpuraEnginePlus();
  const MethodChannel channel = MethodChannel('com.tanpura.engine/methods');

  setUp(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
        .setMockMethodCallHandler(channel, (MethodCall methodCall) async {
          if (methodCall.method == 'initialize') {
            return true;
          }
          return null;
        });
  });

  tearDown(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
        .setMockMethodCallHandler(channel, null);
  });

  test('initialize', () async {
    expect(await platform.initialize(), true);
  });
}
