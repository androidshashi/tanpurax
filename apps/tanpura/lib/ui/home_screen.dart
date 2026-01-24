import 'package:flutter/material.dart';
import 'package:tanpura/utils/env.dart';
import 'package:tanpura/widgets/export_button.dart';
import 'package:tanpura/widgets/first_string_dropdown.dart';
import 'package:tanpura/widgets/play_button.dart';
import 'package:tanpura/widgets/scale_selector.dart';
import 'package:tanpura/widgets/tempo_slider.dart';
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
      appBar: AppBar(title: Text(_getTitle())),
      body: Padding(
        padding: const EdgeInsets.symmetric(horizontal: 32),
        child: SingleChildScrollView(
          child: Column(
            spacing: 30,
            children: [
              PlayButton(),
              TempoSlider(),
              VolumeSlider(),
              ScaleSelector(),
              FirstStringDropdown(),
              const SizedBox(height: 10),
              ExportButton(),
            ],
          ),
        ),
      ),
    );
  }

  String _getTitle() {
    switch (EnvConfig.current) {
      case Env.dev:
        return 'Tanpura DEV';
      case Env.staging:
        return 'Tanpura STAGING';
      case Env.prod:
        return 'Tanpura';
    }
  }
}
