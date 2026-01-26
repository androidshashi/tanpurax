import 'package:flutter/material.dart';
import 'package:tanpuray/tanpura_dsp.dart';
import 'dart:math' as math;
import 'dart:async';

void main() {
  runApp(const TanpuraApp());
}

class TanpuraApp extends StatelessWidget {
  const TanpuraApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Tanpura',
      theme: ThemeData.dark().copyWith(
        scaffoldBackgroundColor: const Color(0xFF1A1A2E),
        primaryColor: const Color(0xFFE94560),
      ),
      home: const TanpuraScreen(),
    );
  }
}

class TanpuraScreen extends StatefulWidget {
  const TanpuraScreen({Key? key}) : super(key: key);

  @override
  State<TanpuraScreen> createState() => _TanpuraScreenState();
}

class _TanpuraScreenState extends State<TanpuraScreen>
    with TickerProviderStateMixin {
  late TanpuraNativeDSP _dsp;
  bool _isPlaying = false;

  // Parameters
  String _selectedPitch = 'C';
  int _selectedOctave = 4;
  double _tempo = 60.0;
  double _jawari = 0.7;
  double _resonance = 0.4;

  // Animation controllers for visual feedback
  late List<AnimationController> _stringControllers;

  final Map<String, double> _pitchFrequencies = {
    'C': 261.63,
    'C#': 277.18,
    'D': 293.66,
    'D#': 311.13,
    'E': 329.63,
    'F': 349.23,
    'F#': 369.99,
    'G': 392.00,
    'G#': 415.30,
    'A': 440.00,
    'A#': 466.16,
    'B': 493.88,
  };

  @override
  void initState() {
    super.initState();
    _dsp = TanpuraNativeDSP();

    // Initialize string animation controllers
    _stringControllers = List.generate(
      4,
      (index) => AnimationController(
        vsync: this,
        duration: Duration(milliseconds: (60000 / _tempo).round()),
      ),
    );
  }

  @override
  void dispose() {
    _dsp.dispose();
    for (var controller in _stringControllers) {
      controller.dispose();
    }
    super.dispose();
  }

  void _togglePlay() async {
    setState(() {
      _isPlaying = !_isPlaying;
    });

    if (_isPlaying) {
      _updatePitch();
      await _dsp.start();
      _startStringAnimations();
    } else {
      await _dsp.stop();
      _stopStringAnimations();
    }
  }

  void _updatePitch() {
    final baseFreq = _pitchFrequencies[_selectedPitch]!;
    final octaveMultiplier = math.pow(2, _selectedOctave - 4);
    _dsp.setPitch(baseFreq * octaveMultiplier);
  }

  void _startStringAnimations() {
    for (int i = 0; i < _stringControllers.length; i++) {
      Future.delayed(Duration(milliseconds: i * (60000 / _tempo ~/ 4)), () {
        if (_isPlaying) {
          _stringControllers[i].repeat();
        }
      });
    }
  }

  void _stopStringAnimations() {
    for (var controller in _stringControllers) {
      controller.stop();
      controller.reset();
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: SafeArea(
        child: Column(
          children: [
            _buildHeader(),
            Expanded(
              child: SingleChildScrollView(
                child: Column(
                  children: [
                    const SizedBox(height: 30),
                    _buildTanpuraVisual(),
                    const SizedBox(height: 40),
                    _buildControls(),
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildHeader() {
    return Container(
      padding: const EdgeInsets.all(20),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          const Text(
            'Tanpura',
            style: TextStyle(
              fontSize: 28,
              fontWeight: FontWeight.bold,
              color: Colors.white,
            ),
          ),
          Container(
            padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
            decoration: BoxDecoration(
              color: _isPlaying ? const Color(0xFFE94560) : Colors.grey[800],
              borderRadius: BorderRadius.circular(20),
            ),
            child: Text(
              _isPlaying ? 'Playing' : 'Stopped',
              style: const TextStyle(
                color: Colors.white,
                fontWeight: FontWeight.w600,
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildTanpuraVisual() {
    return Container(
      height: 350,
      padding: const EdgeInsets.symmetric(horizontal: 40),
      child: Stack(
        children: [
          // Tanpura body
          Center(
            child: Container(
              width: 180,
              height: 300,
              decoration: BoxDecoration(
                gradient: LinearGradient(
                  begin: Alignment.topLeft,
                  end: Alignment.bottomRight,
                  colors: [const Color(0xFF8B4513), const Color(0xFF654321)],
                ),
                borderRadius: const BorderRadius.only(
                  topLeft: Radius.circular(90),
                  topRight: Radius.circular(90),
                  bottomLeft: Radius.circular(20),
                  bottomRight: Radius.circular(20),
                ),
                boxShadow: [
                  BoxShadow(
                    color: Colors.black.withOpacity(0.3),
                    blurRadius: 20,
                    offset: const Offset(0, 10),
                  ),
                ],
              ),
            ),
          ),
          // Strings
          ...List.generate(4, (index) => _buildString(index)),
          // Play button
          Positioned.fill(
            child: Center(
              child: GestureDetector(
                onTap: _togglePlay,
                child: Container(
                  width: 80,
                  height: 80,
                  decoration: BoxDecoration(
                    shape: BoxShape.circle,
                    color: const Color(0xFFE94560),
                    boxShadow: [
                      BoxShadow(
                        color: const Color(0xFFE94560).withOpacity(0.5),
                        blurRadius: 20,
                        spreadRadius: 2,
                      ),
                    ],
                  ),
                  child: Icon(
                    _isPlaying ? Icons.pause : Icons.play_arrow,
                    color: Colors.white,
                    size: 40,
                  ),
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildString(int index) {
    final positions = [0.25, 0.42, 0.58, 0.75];

    return Positioned(
      left: MediaQuery.of(context).size.width * positions[index] - 20,
      top: 20,
      bottom: 20,
      child: AnimatedBuilder(
        animation: _stringControllers[index],
        builder: (context, child) {
          final vibration = _isPlaying
              ? math.sin(_stringControllers[index].value * 2 * math.pi) * 3.0
              : 0.0;

          return Transform.translate(
            offset: Offset(vibration, 0),
            child: Container(
              width: 2,
              decoration: BoxDecoration(
                gradient: LinearGradient(
                  begin: Alignment.topCenter,
                  end: Alignment.bottomCenter,
                  colors: [
                    const Color(0xFFFFD700).withOpacity(0.8),
                    const Color(0xFFB8860B).withOpacity(0.8),
                  ],
                ),
                boxShadow: _isPlaying
                    ? [
                        BoxShadow(
                          color: const Color(0xFFFFD700).withOpacity(0.6),
                          blurRadius: 8,
                          spreadRadius: 1,
                        ),
                      ]
                    : null,
              ),
            ),
          );
        },
      ),
    );
  }

  Widget _buildControls() {
    return Container(
      padding: const EdgeInsets.all(20),
      child: Column(
        children: [
          _buildPitchSelector(),
          const SizedBox(height: 30),
          _buildSlider(
            label: 'Tempo',
            value: _tempo,
            min: 30,
            max: 120,
            divisions: 90,
            unit: ' BPM',
            onChanged: (value) {
              setState(() {
                _tempo = value;
              });
              // _dsp.setTempo(_tempo);
              _updateStringAnimationSpeed();
            },
          ),
          const SizedBox(height: 20),
          _buildSlider(
            label: 'Jawari',
            value: _jawari,
            min: 0.0,
            max: 1.0,
            divisions: 100,
            unit: '',
            onChanged: (value) {
              setState(() {
                _jawari = value;
              });
              _dsp.setJawari(_jawari);
            },
          ),
          const SizedBox(height: 20),
          _buildSlider(
            label: 'Resonance',
            value: _resonance,
            min: 0.0,
            max: 1.0,
            divisions: 100,
            unit: '',
            onChanged: (value) {
              setState(() {
                _resonance = value;
              });
              // _dsp.setResonance(_resonance);
            },
          ),
        ],
      ),
    );
  }

  Widget _buildPitchSelector() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text(
          'Pitch',
          style: TextStyle(
            fontSize: 16,
            fontWeight: FontWeight.w600,
            color: Colors.white70,
          ),
        ),
        const SizedBox(height: 15),
        Row(
          children: [
            Expanded(
              child: Container(
                height: 50,
                decoration: BoxDecoration(
                  color: Colors.grey[850],
                  borderRadius: BorderRadius.circular(12),
                ),
                child: ListView.builder(
                  scrollDirection: Axis.horizontal,
                  itemCount: _pitchFrequencies.keys.length,
                  itemBuilder: (context, index) {
                    final pitch = _pitchFrequencies.keys.elementAt(index);
                    final isSelected = pitch == _selectedPitch;

                    return GestureDetector(
                      onTap: () {
                        setState(() {
                          _selectedPitch = pitch;
                        });
                        _updatePitch();
                      },
                      child: Container(
                        width: 50,
                        margin: const EdgeInsets.all(6),
                        decoration: BoxDecoration(
                          color: isSelected
                              ? const Color(0xFFE94560)
                              : Colors.transparent,
                          borderRadius: BorderRadius.circular(8),
                        ),
                        child: Center(
                          child: Text(
                            pitch,
                            style: TextStyle(
                              color: isSelected ? Colors.white : Colors.grey,
                              fontWeight: FontWeight.w600,
                            ),
                          ),
                        ),
                      ),
                    );
                  },
                ),
              ),
            ),
            const SizedBox(width: 12),
            Container(
              height: 50,
              padding: const EdgeInsets.symmetric(horizontal: 16),
              decoration: BoxDecoration(
                color: Colors.grey[850],
                borderRadius: BorderRadius.circular(12),
              ),
              child: DropdownButton<int>(
                value: _selectedOctave,
                underline: const SizedBox(),
                dropdownColor: Colors.grey[850],
                style: const TextStyle(
                  color: Colors.white,
                  fontSize: 16,
                  fontWeight: FontWeight.w600,
                ),
                items: [2, 3, 4, 5, 6]
                    .map(
                      (octave) => DropdownMenuItem(
                        value: octave,
                        child: Text('Oct $octave'),
                      ),
                    )
                    .toList(),
                onChanged: (value) {
                  if (value != null) {
                    setState(() {
                      _selectedOctave = value;
                    });
                    _updatePitch();
                  }
                },
              ),
            ),
          ],
        ),
      ],
    );
  }

  Widget _buildSlider({
    required String label,
    required double value,
    required double min,
    required double max,
    required int divisions,
    required String unit,
    required ValueChanged<double> onChanged,
  }) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Text(
              label,
              style: const TextStyle(
                fontSize: 16,
                fontWeight: FontWeight.w600,
                color: Colors.white70,
              ),
            ),
            Text(
              '${value.toStringAsFixed(unit.isEmpty ? 2 : 0)}$unit',
              style: const TextStyle(
                fontSize: 16,
                fontWeight: FontWeight.w600,
                color: Color(0xFFE94560),
              ),
            ),
          ],
        ),
        SliderTheme(
          data: SliderThemeData(
            activeTrackColor: const Color(0xFFE94560),
            inactiveTrackColor: Colors.grey[800],
            thumbColor: const Color(0xFFE94560),
            overlayColor: const Color(0xFFE94560).withOpacity(0.2),
            trackHeight: 4,
          ),
          child: Slider(
            value: value,
            min: min,
            max: max,
            divisions: divisions,
            onChanged: onChanged,
          ),
        ),
      ],
    );
  }

  void _updateStringAnimationSpeed() {
    final duration = Duration(milliseconds: (60000 / _tempo).round());
    for (var controller in _stringControllers) {
      controller.duration = duration;
    }
  }
}
