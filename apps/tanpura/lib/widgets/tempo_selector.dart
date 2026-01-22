import 'package:flutter/material.dart';
import 'package:tanpura_engine/tanpura_engine.dart';

enum Tempo { slow, medium, fast }

class TempoSelector extends StatefulWidget {
  const TempoSelector({super.key});

  @override
  State<TempoSelector> createState() => _TempoSelectorState();
}

class _TempoSelectorState extends State<TempoSelector> {
  Tempo selected = Tempo.medium;

  Widget _buildButton(String text, Tempo tempo) {
    final bool active = selected == tempo;

    void setTempo(Tempo t) {
      switch (t) {
        case Tempo.slow:
          TanpuraEngine.setTempo(0.7);
          break;
        case Tempo.medium:
          TanpuraEngine.setTempo(0.5);
          break;
        case Tempo.fast:
          TanpuraEngine.setTempo(0.35);
          break;
      }
    }

    return Expanded(
      child: GestureDetector(
        onTap: () {
          // Phase 2: Send tempo to native engine
          setTempo(tempo);

          setState(() {
            selected = tempo;
          });
        },
        child: Container(
          padding: const EdgeInsets.symmetric(vertical: 12),
          decoration: BoxDecoration(
            color: active ? const Color(0xFFC9A24D) : Colors.transparent,
            borderRadius: BorderRadius.circular(8),
            border: Border.all(color: const Color(0xFFC9A24D)),
          ),
          child: Text(
            text,
            textAlign: TextAlign.center,
            style: TextStyle(
              color: active ? Colors.black : const Color(0xFFC9A24D),
              fontWeight: FontWeight.w600,
            ),
          ),
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        _buildButton("Slow", Tempo.slow),
        const SizedBox(width: 12),
        _buildButton("Medium", Tempo.medium),
        const SizedBox(width: 12),
        _buildButton("Fast", Tempo.fast),
      ],
    );
  }
}
