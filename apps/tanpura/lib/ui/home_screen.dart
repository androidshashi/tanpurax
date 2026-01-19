import 'package:flutter/material.dart';
import 'package:tanpura/widgets/play_button.dart';
import 'package:tanpura/widgets/tempo_selector.dart';
import 'package:tanpura/widgets/volume_slider.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Tanpura')),
      body: Column(
        spacing: 40,
        children: [PlayButton(), TempoSelector(), VolumeSlider()],
      ),
    );
  }
}
