import 'package:firebase_core/firebase_core.dart';
import 'package:flutter/material.dart';
import 'package:tanpura/firebase_options.dart';
import 'package:tanpura/theme/app_theme.dart';
import 'package:tanpura/ui/home_screen.dart';
import 'package:tanpura_engine/tanpura_engine.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await TanpuraEngine.initialize();
  WidgetsFlutterBinding.ensureInitialized();

  await Firebase.initializeApp(options: DefaultFirebaseOptions.currentPlatform);

  runApp(const TanpuraApp());
}

class TanpuraApp extends StatelessWidget {
  const TanpuraApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      theme: AppTheme.darkTheme,
      home: const HomeScreen(),
    );
  }
}
