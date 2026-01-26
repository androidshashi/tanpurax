import 'tanpura_note.dart';

/// Configuration for individual tanpura strings
class StringConfig {
  /// The note this string plays
  final TanpuraNote note;
  
  /// Relative volume (0.0 - 1.0)
  final double volume;
  
  /// Stereo pan position (-1.0 left to 1.0 right)
  final double pan;
  
  /// Fine tuning in cents (-100 to +100)
  final double fineTune;
  
  /// Whether this string is enabled
  final bool enabled;

  const StringConfig({
    required this.note,
    this.volume = 0.25,
    this.pan = 0.0,
    this.fineTune = 0.0,
    this.enabled = true,
  });

  StringConfig copyWith({
    TanpuraNote? note,
    double? volume,
    double? pan,
    double? fineTune,
    bool? enabled,
  }) {
    return StringConfig(
      note: note ?? this.note,
      volume: volume ?? this.volume,
      pan: pan ?? this.pan,
      fineTune: fineTune ?? this.fineTune,
      enabled: enabled ?? this.enabled,
    );
  }

  Map<String, dynamic> toMap() {
    return {
      'noteIndex': note.semitoneOffset,
      'volume': volume,
      'pan': pan,
      'fineTune': fineTune,
      'enabled': enabled,
    };
  }
}

/// Complete tanpura configuration
class TanpuraConfig {
  /// Base scale (Sa = which Western note)
  final Scale scale;
  
  /// Octave setting
  final TanpuraOctave octave;
  
  /// Master volume (0.0 - 1.0)
  final double masterVolume;
  
  /// Tempo - interval between string plucks in seconds
  final double tempo;
  
  /// Configuration for each of the 4 strings
  /// Standard: [First string (Pa/Ma/Ni), Jodi (Sa), Sa, Kharaj (low Sa)]
  final List<StringConfig> strings;

  const TanpuraConfig({
    this.scale = Scale.c,
    this.octave = TanpuraOctave.middle,
    this.masterVolume = 0.85,
    this.tempo = 0.4,
    this.strings = const [
      StringConfig(note: TanpuraNote.pa, volume: 0.28, pan: -0.5),
      StringConfig(note: TanpuraNote.sa, volume: 0.22, pan: -0.2),
      StringConfig(note: TanpuraNote.sa, volume: 0.26, pan: 0.2),
      StringConfig(note: TanpuraNote.sa, volume: 0.24, pan: 0.5),
    ],
  });

  /// Create a standard Pa-Sa-Sa-Sa configuration
  factory TanpuraConfig.standard({
    Scale scale = Scale.c,
    TanpuraOctave octave = TanpuraOctave.middle,
    double masterVolume = 0.85,
    double tempo = 0.4,
  }) {
    return TanpuraConfig(
      scale: scale,
      octave: octave,
      masterVolume: masterVolume,
      tempo: tempo,
      strings: const [
        StringConfig(note: TanpuraNote.pa, volume: 0.28, pan: -0.5),
        StringConfig(note: TanpuraNote.sa, volume: 0.22, pan: -0.2),
        StringConfig(note: TanpuraNote.sa, volume: 0.26, pan: 0.2),
        StringConfig(note: TanpuraNote.sa, volume: 0.24, pan: 0.5, fineTune: -12), // Kharaj (low octave)
      ],
    );
  }

  /// Create a Ma-Sa-Sa-Sa configuration
  factory TanpuraConfig.withMa({
    Scale scale = Scale.c,
    TanpuraOctave octave = TanpuraOctave.middle,
    double masterVolume = 0.85,
    double tempo = 0.4,
  }) {
    return TanpuraConfig(
      scale: scale,
      octave: octave,
      masterVolume: masterVolume,
      tempo: tempo,
      strings: const [
        StringConfig(note: TanpuraNote.ma, volume: 0.28, pan: -0.5),
        StringConfig(note: TanpuraNote.sa, volume: 0.22, pan: -0.2),
        StringConfig(note: TanpuraNote.sa, volume: 0.26, pan: 0.2),
        StringConfig(note: TanpuraNote.sa, volume: 0.24, pan: 0.5, fineTune: -12),
      ],
    );
  }

  /// Create a Ni-Sa-Sa-Sa configuration
  factory TanpuraConfig.withNi({
    Scale scale = Scale.c,
    TanpuraOctave octave = TanpuraOctave.middle,
    double masterVolume = 0.85,
    double tempo = 0.4,
  }) {
    return TanpuraConfig(
      scale: scale,
      octave: octave,
      masterVolume: masterVolume,
      tempo: tempo,
      strings: const [
        StringConfig(note: TanpuraNote.ni, volume: 0.28, pan: -0.5),
        StringConfig(note: TanpuraNote.sa, volume: 0.22, pan: -0.2),
        StringConfig(note: TanpuraNote.sa, volume: 0.26, pan: 0.2),
        StringConfig(note: TanpuraNote.sa, volume: 0.24, pan: 0.5, fineTune: -12),
      ],
    );
  }

  TanpuraConfig copyWith({
    Scale? scale,
    TanpuraOctave? octave,
    double? masterVolume,
    double? tempo,
    List<StringConfig>? strings,
  }) {
    return TanpuraConfig(
      scale: scale ?? this.scale,
      octave: octave ?? this.octave,
      masterVolume: masterVolume ?? this.masterVolume,
      tempo: tempo ?? this.tempo,
      strings: strings ?? this.strings,
    );
  }

  Map<String, dynamic> toMap() {
    return {
      'scaleIndex': scale.semitoneOffset,
      'octave': octave.value,
      'masterVolume': masterVolume,
      'tempo': tempo,
      'strings': strings.map((s) => s.toMap()).toList(),
    };
  }
}
