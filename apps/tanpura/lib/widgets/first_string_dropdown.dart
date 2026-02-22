import 'package:flutter/material.dart';
import 'package:tanpura_engine/enum/engine_enums.dart';
import 'package:tanpura_engine/tanpura_engine.dart';

class FirstStringDropdown extends StatefulWidget {
  final Function(FirstString)? onChanged;

  const FirstStringDropdown({super.key, this.onChanged});

  @override
  State<FirstStringDropdown> createState() => _FirstStringDropdownState();
}

class _FirstStringDropdownState extends State<FirstStringDropdown> {
  // Default to Pa (most common tanpura tuning) - matches JS default (ratio 0.75)
  late FirstString selectedString = FirstString.pa;

  @override
  void initState() {
    super.initState();
    // Sync engine with default selection on widget init
    TanpuraEngine.setFirstString(selectedString);
  }

  /// Convert enum name to display text (e.g., reKomal -> Re Komal)
  String _getDisplayText(FirstString value) {
    final name = value.toString().split('.').last;
    // Insert space before capital letters
    return name
        .replaceAllMapped(RegExp(r'([A-Z])'), (Match m) => ' ${m.group(1)}')
        .trim();
  }

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 4),
      decoration: BoxDecoration(
        border: Border.all(color: const Color(0xFFC9A24D)),
        borderRadius: BorderRadius.circular(8),
      ),
      child: DropdownButton<FirstString>(
        value: selectedString,
        isExpanded: true,
        underline: const SizedBox(),
        dropdownColor: const Color(0xFF1A1A1A),
        items:
            FirstString.values.map((FirstString value) {
              return DropdownMenuItem<FirstString>(
                value: value,
                child: Text(
                  _getDisplayText(value),
                  style: const TextStyle(
                    color: Color(0xFFC9A24D),
                    fontSize: 14,
                    fontWeight: FontWeight.w500,
                  ),
                ),
              );
            }).toList(),
        onChanged: (FirstString? newValue) {
          if (newValue != null) {
            setState(() {
              selectedString = newValue;
            });
            TanpuraEngine.setFirstString(newValue);
            widget.onChanged?.call(newValue);
          }
        },
        style: const TextStyle(
          color: Color(0xFFC9A24D),
          fontSize: 14,
          fontWeight: FontWeight.w500,
        ),
        iconEnabledColor: const Color(0xFFC9A24D),
      ),
    );
  }
}
