/// Represents Indian classical music notes (Swaras)
enum TanpuraNote {
  sa('Sa', 'sa.mid', 0),
  reKomal('Re♭', 're.mid', 1),
  re('Re', 're1.mid', 2),
  gaKomal('Ga♭', 'ga.mid', 3),
  ga('Ga', 'ga1.mid', 4),
  ma('Ma', 'ma.mid', 5),
  maTivra('Ma#', 'ma2.mid', 6),
  pa('Pa', 'pa.mid', 7),
  dhaKomal('Dha♭', 'dha.mid', 8),
  dha('Dha', 'dha1.mid', 9),
  niKomal('Ni♭', 'ni.mid', 10),
  ni('Ni', 'ni1.mid', 11);

  final String displayName;
  final String midiFile;
  final int semitoneOffset;

  const TanpuraNote(this.displayName, this.midiFile, this.semitoneOffset);

  /// Get note by semitone offset (0-11)
  static TanpuraNote fromSemitone(int semitone) {
    return TanpuraNote.values.firstWhere(
      (note) => note.semitoneOffset == semitone % 12,
      orElse: () => TanpuraNote.sa,
    );
  }
}

/// Western scale names
enum Scale {
  c('C', 0),
  cSharp('C#', 1),
  d('D', 2),
  dSharp('D#', 3),
  e('E', 4),
  f('F', 5),
  fSharp('F#', 6),
  g('G', 7),
  gSharp('G#', 8),
  a('A', 9),
  aSharp('A#', 10),
  b('B', 11);

  final String displayName;
  final int semitoneOffset;

  const Scale(this.displayName, this.semitoneOffset);

  static Scale fromIndex(int index) {
    return Scale.values[index % 12];
  }
}

/// Octave range for the tanpura
enum TanpuraOctave {
  low(-1, 'Low (Male)'),
  middle(0, 'Middle'),
  high(1, 'High (Female)');

  final int value;
  final String displayName;

  const TanpuraOctave(this.value, this.displayName);
}

/// Common first string configurations
enum FirstStringConfig {
  pa(TanpuraNote.pa, 'Pa - Most common'),
  ma(TanpuraNote.ma, 'Ma - For certain ragas'),
  ni(TanpuraNote.ni, 'Ni - For specific moods');

  final TanpuraNote note;
  final String description;

  const FirstStringConfig(this.note, this.description);
}
