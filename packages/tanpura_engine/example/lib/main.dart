import 'package:flutter/material.dart';
import 'dart:async';

import 'package:flutter/services.dart';
import 'package:tanpura_engine/tanpura_engine.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _platformVersion = 'Unknown';
  final _tanpuraEnginePlugin = TanpuraEngine();

  @override
  void initState() {
    super.initState();
    initPlatformState();
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> initPlatformState() async {
    String platformVersion;
    // Platform messages may fail, so we use a try/catch PlatformException.
    // We also handle the message potentially returning null.
    try {
      platformVersion =
          await TanpuraEngine.getPlatformVersion() ??
          'Unknown platform version';

      await TanpuraEngine.start();
      await TanpuraEngine.setTempo(1.0);
    } on PlatformException {
      platformVersion = 'Failed to get platform version.';
    }

    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;

    setState(() {
      _platformVersion = platformVersion;
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('Plugin example app')),
        body: Column(
          spacing: 10,
          children: [
            Center(child: Text('Running on: $_platformVersion\n')),
            ElevatedButton(
              onPressed: () async {
                await TanpuraEngine.start();
              },
              child: const Text('Start Tanpura'),
            ),
            ElevatedButton(
              onPressed: () async {
                await TanpuraEngine.stop();
              },
              child: const Text('Stop Tanpura'),
            ),
          ],
        ),
      ),
    );
  }
}
