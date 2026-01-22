import 'package:flutter/material.dart';
import 'package:tanpura_engine/tanpura_engine.dart';

class VolumeSlider extends StatefulWidget {
  const VolumeSlider({super.key});

  @override
  State<VolumeSlider> createState() => _VolumeSliderState();
}

class _VolumeSliderState extends State<VolumeSlider> {
  double volume = 0.8;

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text("Volume", style: TextStyle(color: Colors.white70)),
        Slider(
          value: volume,
          min: 0,
          max: 1,
          onChanged: (value) {
            setState(() {
              volume = value;
            });
            // Phase 2: Send volume to native engine
            TanpuraEngine.setVolume(volume);
          },
        ),
      ],
    );
  }
}
