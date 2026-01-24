import 'package:flutter/material.dart';
import 'package:tanpura_engine/tanpura_engine.dart';

class OctaveSelector extends StatefulWidget {
  const OctaveSelector({super.key});

  @override
  State<OctaveSelector> createState() => _OctaveSelectorState();
}

class _OctaveSelectorState extends State<OctaveSelector> {
  // Octave levels: Low (-1), Medium (0), High (1)
  static const List<String> _octaveLabels = <String>['Low', 'Medium', 'High'];
  static const List<int> _octaveValues = <int>[-1, 0, 1];

  int _selectedValue = 0; // Default to Medium (0)

  @override
  void initState() {
    super.initState();
    TanpuraEngine.setScale(_selectedValue);
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
              'Octave',
              style: TextStyle(
                color: Color(0xFFC9A24D),
                fontWeight: FontWeight.w600,
                fontSize: 16,
              ),
            ),
            Text(
              _octaveLabels[_octaveValues.indexOf(_selectedValue)],
              style: const TextStyle(
                color: Color(0xFFC9A24D),
                fontWeight: FontWeight.w600,
                fontSize: 16,
              ),
            ),
          ],
        ),
        const SizedBox(height: 8),
        Row(
          children: List.generate(_octaveLabels.length, (int i) {
            final String label = _octaveLabels[i];
            final int value = _octaveValues[i];
            final bool active = _selectedValue == value;
            return Expanded(
              child: GestureDetector(
                onTap: () {
                  setState(() {
                    _selectedValue = value;
                  });
                  // Pass octave value to engine
                  TanpuraEngine.setScale(value);
                },
                child: Container(
                  padding: const EdgeInsets.symmetric(vertical: 12),
                  margin: const EdgeInsets.symmetric(horizontal: 4),
                  decoration: BoxDecoration(
                    color:
                        active ? const Color(0xFFC9A24D) : Colors.transparent,
                    borderRadius: BorderRadius.circular(8),
                    border: Border.all(color: const Color(0xFFC9A24D)),
                  ),
                  child: Text(
                    label,
                    textAlign: TextAlign.center,
                    style: TextStyle(
                      color: active ? Colors.black : const Color(0xFFC9A24D),
                      fontWeight: FontWeight.w600,
                      fontSize: 14,
                    ),
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
