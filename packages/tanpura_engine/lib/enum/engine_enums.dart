/// Musical first-string options for Tanpura.
///
/// Determines the tonal center of the drone.
enum FirstString {
  sa,

  reKomal,
  reShuddh,

  gaKomal,
  gaShuddh,

  maShuddh,
  maTivra,

  pa,

  dhaKomal,
  dhaShuddh,

  niKomal,
  niShuddh,
}

/// Scale/Pitch options for Tanpura.
///
/// Determines the base frequency of Sa.
/// Each value represents a Western note that Sa is tuned to.
enum Scale {
  c,  // C3 = 130.81 Hz
  cSharp, // C#3 = 138.59 Hz
  d,  // D3 = 146.83 Hz (default)
  dSharp, // D#3 = 155.56 Hz
  e,  // E3 = 164.81 Hz
  f,  // F3 = 174.61 Hz
  fSharp, // F#3 = 185.00 Hz
  g,  // G3 = 196.00 Hz
  gSharp, // G#3 = 207.65 Hz
  a,  // A3 = 220.00 Hz
  aSharp, // A#3 = 233.08 Hz
  b,  // B3 = 246.94 Hz
}

/// Octave options for Tanpura.
///
/// Determines the octave shift for all frequencies.
/// Each octave doubles or halves the frequency.
enum Octave {
  octave1,  // -2 octaves (0.25x frequency) - Very low
  octave2,  // -1 octave (0.5x frequency) - Low
  octave3,  // Base octave (1.0x frequency) - Default/Middle
  octave4,  // +1 octave (2.0x frequency) - High
  octave5,  // +2 octaves (4.0x frequency) - Very high
}
