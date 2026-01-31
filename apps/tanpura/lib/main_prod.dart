import 'package:tanpura/main.dart' as app;
import 'package:tanpura/utils/env.dart'; // your env holder

void main() {
  EnvConfig.current = Env.prod; // set flavor-specific config
  app.main();
}
