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
  c, // C = 130.81 Hz (mid octave)
  cSharp, // C# = 138.59 Hz
  d, // D = 146.83 Hz (default)
  dSharp, // D# = 155.56 Hz
  e, // E = 164.81 Hz
  f, // F = 174.61 Hz
  fSharp, // F# = 185.00 Hz
  g, // G = 196.00 Hz
  gSharp, // G# = 207.65 Hz
  a, // A = 220.00 Hz
  aSharp, // A# = 233.08 Hz
  b, // B = 246.94 Hz
}

/// Octave options for Tanpura.
///
/// Determines the octave range of the tanpura.
/// - low: Lower octave (C2-B2 range) - deeper, more bass
/// - mid: Middle octave (C3-B3 range) - standard tanpura range
/// - high: Higher octave (C4-B4 range) - brighter, higher pitch
enum Octave {
  low, // 0 - Lower octave (frequencies / 2)
  mid, // 1 - Middle octave (default)
  high, // 2 - Higher octave (frequencies * 2)
}
