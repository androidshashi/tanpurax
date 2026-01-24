import 'package:flutter/material.dart';
import 'package:tanpura_engine/enum/engine_enums.dart';
import 'package:tanpura_engine/tanpura_engine.dart';

class ScaleSelector extends StatefulWidget {
  final Function(Scale)? onChanged;

  const ScaleSelector({super.key, this.onChanged});

  @override
  State<ScaleSelector> createState() => _ScaleSelectorState();
}

class _ScaleSelectorState extends State<ScaleSelector> {
  Scale _selectedScale = Scale.d; // Default to D

  String _getDisplayText(Scale scale) {
    switch (scale) {
      case Scale.c:
        return 'C';
      case Scale.cSharp:
        return 'C#';
      case Scale.d:
        return 'D';
      case Scale.dSharp:
        return 'D#';
      case Scale.e:
        return 'E';
      case Scale.f:
        return 'F';
      case Scale.fSharp:
        return 'F#';
      case Scale.g:
        return 'G';
      case Scale.gSharp:
        return 'G#';
      case Scale.a:
        return 'A';
      case Scale.aSharp:
        return 'A#';
      case Scale.b:
        return 'B';
    }
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text("Scale (Sa = )", style: TextStyle(color: Colors.white70)),
        const SizedBox(height: 8),
        Container(
          padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 4),
          decoration: BoxDecoration(
            border: Border.all(color: const Color(0xFFC9A24D)),
            borderRadius: BorderRadius.circular(8),
          ),
          child: DropdownButton<Scale>(
            value: _selectedScale,
            isExpanded: true,
            underline: const SizedBox(),
            dropdownColor: const Color(0xFF1A1A1A),
            items: Scale.values.map((Scale scale) {
              return DropdownMenuItem<Scale>(
                value: scale,
                child: Text(
                  _getDisplayText(scale),
                  style: const TextStyle(
                    color: Color(0xFFC9A24D),
                    fontSize: 16,
                    fontWeight: FontWeight.w500,
                  ),
                ),
              );
            }).toList(),
            onChanged: (Scale? newValue) {
              if (newValue != null) {
                setState(() {
                  _selectedScale = newValue;
                });
                TanpuraEngine.setScale(newValue);
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
