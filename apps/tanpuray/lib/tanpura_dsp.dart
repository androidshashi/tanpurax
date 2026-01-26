// lib/tanpura_native.dart
import 'dart:async';
import 'package:flutter/services.dart';

/// Native C++ Tanpura - No Dart synthesis, all in C++
class TanpuraNativeDSP {
  static const MethodChannel _channel = MethodChannel('tanpura_dsp');

  bool _isPlaying = false;

  double _pitch = 261.63;
  double _jawariIntensity = 0.7;
  double _tempo = 60.0;

  Future<void> start() async {
    if (_isPlaying) return;

    try {
      // Initialize native engine
      await _channel.invokeMethod('initialize');

      // Set initial parameters
      await setPitch(_pitch);
      await setJawari(_jawariIntensity);
      await setTempo(_tempo);

      // Start audio
      await _channel.invokeMethod('start');

      _isPlaying = true;
      print('🎵 Native C++ Tanpura started');
    } catch (e) {
      print('❌ Failed to start: $e');
    }
  }

  Future<void> stop() async {
    if (!_isPlaying) return;

    try {
      await _channel.invokeMethod('stop');
      _isPlaying = false;
      print('🛑 Tanpura stopped');
    } catch (e) {
      print('❌ Failed to stop: $e');
    }
  }

  Future<void> setPitch(double frequencyHz) async {
    _pitch = frequencyHz;
    if (_isPlaying) {
      try {
        await _channel.invokeMethod('setPitch', {'pitch': frequencyHz});
      } catch (e) {
        print('❌ Failed to set pitch: $e');
      }
    }
  }

  Future<void> setJawari(double intensity) async {
    _jawariIntensity = intensity.clamp(0.0, 1.0);
    if (_isPlaying) {
      try {
        await _channel.invokeMethod('setJawari', {
          'intensity': _jawariIntensity,
        });
      } catch (e) {
        print('❌ Failed to set jawari: $e');
      }
    }
  }

  Future<void> setTempo(double bpm) async {
    _tempo = bpm.clamp(30.0, 120.0);
    if (_isPlaying) {
      try {
        await _channel.invokeMethod('setTempo', {'tempo': _tempo});
      } catch (e) {
        print('❌ Failed to set tempo: $e');
      }
    }
  }

  void dispose() {
    stop();
  }
}

// Update main.dart to use this:
// Replace: late TanpuraDSP _dsp;
// With: late TanpuraNativeDSP _dsp;
