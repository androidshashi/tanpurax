import 'package:flutter/material.dart';
import 'package:tanpura_engine/enum/engine_enums.dart';
import 'package:tanpura_engine/tanpura_engine.dart';

class OctaveSelector extends StatefulWidget {
  final Function(Octave)? onChanged;

  const OctaveSelector({super.key, this.onChanged});

  @override
  State<OctaveSelector> createState() => _OctaveSelectorState();
}

class _OctaveSelectorState extends State<OctaveSelector> {
  Octave _selectedOctave = Octave.octave3; // Default to middle octave (1.0x)

  @override
  void initState() {
    super.initState();
    // Sync engine with default selection on widget init
    TanpuraEngine.setOctave(_selectedOctave);
  }

  String _getDisplayText(Octave octave) {
    switch (octave) {
      case Octave.octave1:
        return 'Octave 1 (Very Low)';
      case Octave.octave2:
        return 'Octave 2 (Low)';
      case Octave.octave3:
        return 'Octave 3 (Middle)';
      case Octave.octave4:
        return 'Octave 4 (High)';
      case Octave.octave5:
        return 'Octave 5 (Very High)';
    }
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text("Octave", style: TextStyle(color: Colors.white70)),
        const SizedBox(height: 8),
        Container(
          padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 4),
          decoration: BoxDecoration(
            border: Border.all(color: const Color(0xFFC9A24D)),
            borderRadius: BorderRadius.circular(8),
          ),
          child: DropdownButton<Octave>(
            value: _selectedOctave,
            isExpanded: true,
            underline: const SizedBox(),
            dropdownColor: const Color(0xFF1A1A1A),
            items: Octave.values.map((Octave octave) {
              return DropdownMenuItem<Octave>(
                value: octave,
                child: Text(
                  _getDisplayText(octave),
                  style: const TextStyle(
                    color: Color(0xFFC9A24D),
                    fontSize: 16,
                    fontWeight: FontWeight.w500,
                  ),
                ),
              );
            }).toList(),
            onChanged: (Octave? newValue) {
              if (newValue != null) {
                setState(() {
                  _selectedOctave = newValue;
                });
                TanpuraEngine.setOctave(newValue);
                widget.onChanged?.call(newValue);
              }
            },
            style: const TextStyle(
              color: Color(0xFFC9A24D),
              fontSize: 16,
              fontWeight: FontWeight.w500,
            ),
            iconEnabledColor: const Color(0xFFC9A24D),
          ),
        ),
      ],
    );
  }
}
