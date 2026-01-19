import 'package:flutter/material.dart';

class AppTheme {
  static ThemeData get darkTheme {
    return ThemeData(
      brightness: Brightness.dark,
      scaffoldBackgroundColor: const Color(0xFF0E0E0E),
      primaryColor: const Color(0xFFC9A24D),
      sliderTheme: SliderThemeData(
        activeTrackColor: const Color(0xFFC9A24D),
        inactiveTrackColor: Colors.white24,
        thumbColor: const Color(0xFFC9A24D),
        overlayColor: const Color(0x33C9A24D),
      ),
      textTheme: const TextTheme(bodyMedium: TextStyle(color: Colors.white)),
    );
  }
}
