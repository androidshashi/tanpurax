import 'package:flutter/material.dart';
import 'dart:async';

import 'package:flutter/services.dart';
import 'package:tanpura_engine_plus/tanpura_engine_plus.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  bool _isInitialized = false;
  bool _isPlaying = false;
  final _tanpuraEngine = TanpuraEnginePlus();

  @override
  void initState() {
    super.initState();
    initEngine();
  }

  Future<void> initEngine() async {
    try {
      final initialized = await _tanpuraEngine.initialize();
      if (mounted) {
        setState(() {
          _isInitialized = initialized;
        });
      }
    } on PlatformException catch (e) {
      print('Failed to initialize: $e');
    }
  }

  Future<void> togglePlayback() async {
    if (!_isInitialized) return;

    try {
      if (_isPlaying) {
        await _tanpuraEngine.stop();
      } else {
        await _tanpuraEngine.play();
      }
      if (mounted) {
        setState(() {
          _isPlaying = !_isPlaying;
        });
      }
    } on PlatformException catch (e) {
      print('Playback error: $e');
    }
  }

  @override
  void dispose() {
    _tanpuraEngine.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('Tanpura Engine Plus Example')),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Text(
                _isInitialized ? 'Engine Initialized ✓' : 'Initializing...',
                style: Theme.of(context).textTheme.headlineSmall,
              ),
              const SizedBox(height: 40),
              ElevatedButton.icon(
                onPressed: _isInitialized ? togglePlayback : null,
                icon: Icon(_isPlaying ? Icons.stop : Icons.play_arrow),
                label: Text(_isPlaying ? 'Stop' : 'Play'),
                style: ElevatedButton.styleFrom(
                  padding: const EdgeInsets.symmetric(
                    horizontal: 32,
                    vertical: 16,
                  ),
                ),
              ),
              const SizedBox(height: 20),
              Text(
                _isPlaying ? '🎵 Playing...' : 'Ready',
                style: Theme.of(context).textTheme.bodyLarge,
              ),
            ],
          ),
        ),
      ),
    );
  }
}
