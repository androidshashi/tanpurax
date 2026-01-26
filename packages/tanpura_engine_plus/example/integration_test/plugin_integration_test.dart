// This is a basic Flutter integration test.
//
// Since integration tests run in a full Flutter application, they can interact
// with the host side of a plugin implementation, unlike Dart unit tests.
//
// For more information about Flutter integration tests, please see
// https://flutter.dev/to/integration-testing

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

import 'package:tanpura_engine_plus/tanpura_engine_plus.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('initialize test', (WidgetTester tester) async {
    final TanpuraEnginePlus plugin = TanpuraEnginePlus();
    final bool initialized = await plugin.initialize();
    // Assert that the engine initializes successfully
    expect(initialized, true);
  });
}
