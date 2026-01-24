import 'package:flutter/material.dart';
import 'package:tanpura_engine/tanpura_engine.dart';

class ScaleSelector extends StatefulWidget {
  const ScaleSelector({super.key});

  @override
  State<ScaleSelector> createState() => _ScaleSelectorState();
}

class _ScaleSelectorState extends State<ScaleSelector> {
  // Chromatic scale names from C to B
  static const List<String> _scales = <String>[
    'C',
    'C#',
    'D',
    'D#',
    'E',
    'F',
    'F#',
    'G',
    'G#',
    'A',
    'A#',
    'B',
  ];

  int _selectedIndex = 0; // Default to C

  @override
  void initState() {
    super.initState();
    TanpuraEngine.setScale(_selectedIndex);
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            const Text(
              'Scale',
              style: TextStyle(
                color: Color(0xFFC9A24D),
                fontWeight: FontWeight.w600,
                fontSize: 16,
              ),
            ),
            Text(
              _scales[_selectedIndex],
              style: const TextStyle(
                color: Color(0xFFC9A24D),
                fontWeight: FontWeight.w600,
                fontSize: 16,
              ),
            ),
          ],
        ),
        const SizedBox(height: 8),
        Wrap(
          spacing: 8,
          runSpacing: 8,
          children: List.generate(_scales.length, (int i) {
            final bool active = _selectedIndex == i;
            return GestureDetector(
              onTap: () {
                setState(() {
                  _selectedIndex = i;
                });
                TanpuraEngine.setScale(i);
              },
              child: Container(
                padding: const EdgeInsets.symmetric(
                  vertical: 10,
                  horizontal: 14,
                ),
                decoration: BoxDecoration(
                  color: active ? const Color(0xFFC9A24D) : Colors.transparent,
                  borderRadius: BorderRadius.circular(8),
                  border: Border.all(color: const Color(0xFFC9A24D)),
                ),
                child: Text(
                  _scales[i],
                  style: TextStyle(
                    color: active ? Colors.black : const Color(0xFFC9A24D),
                    fontWeight: FontWeight.w600,
                  ),
                ),
              ),
            );
          }),
        ),
      ],
    );
  }
}
