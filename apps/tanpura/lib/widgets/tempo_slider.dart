import 'package:flutter/material.dart';
import 'package:tanpura_engine/tanpura_engine.dart';

class TempoSlider extends StatefulWidget {
  const TempoSlider({super.key});

  @override
  State<TempoSlider> createState() => _TempoSliderState();
}

class _TempoSliderState extends State<TempoSlider> {
  // Tempo in seconds (interval between plucks)
  // Range: 0.25s (fast) to 1.2s (slow)
  double _tempo = 0.5;

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text("Tempo", style: TextStyle(color: Colors.white70)),
        Slider(
          value: _tempo,
          min: 0.25,
          max: 1.2,
          onChanged: (value) {
            setState(() {
              _tempo = value;
            });
            TanpuraEngine.setTempo(_tempo);
          },
        ),
      ],
    );
  }
}
