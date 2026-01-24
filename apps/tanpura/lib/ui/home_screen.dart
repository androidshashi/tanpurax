import 'package:flutter/material.dart';
import 'package:tanpura/widgets/first_string_dropdown.dart';
import 'package:tanpura/widgets/octave_selector.dart';
import 'package:tanpura/widgets/play_button.dart';
import 'package:tanpura/widgets/tempo_selector.dart';
import 'package:tanpura/widgets/volume_slider.dart';
import 'package:tanpura/widgets/scale_selector.dart';

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
      body: Padding(
        padding: const EdgeInsets.symmetric(horizontal: 32),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.spaceEvenly,
          children: [
            PlayButton(),
            ScaleSelector(),
            // TempoSelector(),
            VolumeSlider(),
            FirstStringDropdown(),
            OctaveSelector(),
          ],
        ),
      ),
    );
  }
}
