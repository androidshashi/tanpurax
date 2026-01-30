import 'package:flutter/material.dart';
import 'package:tanpura_engine/enum/engine_enums.dart';
import 'package:tanpura_engine/tanpura_engine.dart';

class PlayButton extends StatefulWidget {
  const PlayButton({super.key});

  @override
  State<PlayButton> createState() => _PlayButtonState();
}

class _PlayButtonState extends State<PlayButton> {
  bool isPlaying = false;

  void _toggle() {
    if (isPlaying) {
      TanpuraEngine.pause();
    } else {
      TanpuraEngine.play();
    }
    setState(() => isPlaying = !isPlaying);
  }

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: _toggle,
      child: Container(
        width: 120,
        height: 120,
        decoration: BoxDecoration(
          shape: BoxShape.circle,
          color: const Color(0xFFC9A24D),
          boxShadow: [
            BoxShadow(
              color: const Color(0xFFC9A24D).withOpacity(0.4),
              blurRadius: 20,
            ),
          ],
        ),
        child: Icon(
          isPlaying ? Icons.pause : Icons.play_arrow,
          size: 64,
          color: Colors.black,
        ),
      ),
    );
  }
}
