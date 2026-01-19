import 'package:flutter/material.dart';
import 'package:tanpura/theme/app_theme.dart';
import 'package:tanpura/ui/home_screen.dart';

void main() {
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
