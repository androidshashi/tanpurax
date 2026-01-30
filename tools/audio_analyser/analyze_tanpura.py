#!/usr/bin/env python3
"""
Tanpura Audio Analyzer
Generates a detailed analysis report for tanpura audio files (WAV or MP3).

Usage:
    python3 analyze_tanpura.py <audio_file>

Example:
    python3 analyze_tanpura.py tanpura_1769514216248.wav
    python3 analyze_tanpura.py tanpura_recording.mp3

Output:
    Creates a report file: <audio_file>_analysis.txt
"""

import sys
import os
import numpy as np
from scipy.io import wavfile
from scipy.fft import fft
import warnings
from datetime import datetime
import tempfile

warnings.filterwarnings('ignore')

try:
    from pydub import AudioSegment
    HAS_PYDUB = True
except ImportError:
    HAS_PYDUB = False

# Scale frequencies (Sa) for all 12 scales
SCALE_FREQUENCIES = {
    'C':  130.81, 'C#': 138.59, 'D':  146.83, 'D#': 155.56,
    'E':  164.81, 'F':  174.61, 'F#': 185.00, 'G':  196.00,
    'G#': 207.65, 'A':  220.00, 'A#': 233.08, 'B':  246.94
}

NOTE_NAMES = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']


def freq_to_note(freq):
    """Convert frequency to musical note name."""
    if freq < 20:
        return "?"
    midi = 69 + 12 * np.log2(freq / 440.0)
    note_idx = int(round(midi)) % 12
    octave = int(round(midi)) // 12 - 1
    return f"{NOTE_NAMES[note_idx]}{octave}"


def detect_scale(dominant_freq):
    """Detect which scale the tanpura is tuned to based on dominant frequency."""
    best_match = None
    best_diff = float('inf')
    best_harmonic = 1

    for scale_name, sa_freq in SCALE_FREQUENCIES.items():
        for h in range(1, 13):
            expected = sa_freq * h
            diff = abs(dominant_freq - expected)
            if diff < best_diff:
                best_diff = diff
                best_match = scale_name
                best_harmonic = h

    return best_match, SCALE_FREQUENCIES[best_match], best_harmonic


def convert_mp3_to_wav(mp3_path):
    """Convert MP3 file to temporary WAV file."""
    if not HAS_PYDUB:
        raise ImportError("pydub is required to process MP3 files. Install with: pip3 install pydub")
    
    # Create temporary WAV file
    temp_wav = tempfile.NamedTemporaryFile(suffix='.wav', delete=False)
    temp_wav.close()
    
    # Load MP3 and export as WAV
    audio = AudioSegment.from_mp3(mp3_path)
    audio.export(temp_wav.name, format='wav')
    
    return temp_wav.name


def find_audio_files(folder_path):
    """Find all audio files (.wav and .mp3) in a folder."""
    audio_files = []
    for filename in sorted(os.listdir(folder_path)):
        if filename.lower().endswith(('.wav', '.mp3')):
            audio_files.append(os.path.join(folder_path, filename))
    return audio_files


def ensure_results_folder(base_path):
    """Create a 'results' folder in the given base path if it doesn't exist."""
    results_folder = os.path.join(base_path, 'results')
    os.makedirs(results_folder, exist_ok=True)
    return results_folder


def analyze_single_file(filepath):
    """Analyze a single audio file and return analysis results."""
    temp_wav = None
    try:
        file_ext = os.path.splitext(filepath)[1].lower()
        
        if file_ext == '.mp3':
            if not HAS_PYDUB:
                raise ImportError("pydub is required to process MP3 files")
            temp_wav = convert_mp3_to_wav(filepath)
            analysis_filepath = temp_wav
        elif file_ext == '.wav':
            analysis_filepath = filepath
        else:
            return None
        
        analysis = analyze_wav(analysis_filepath)
        analysis['filepath'] = filepath
        return analysis
    
    except Exception as e:
        print(f"  Error analyzing {os.path.basename(filepath)}: {e}")
        return None
    
    finally:
        if temp_wav and os.path.exists(temp_wav):
            os.unlink(temp_wav)


def generate_summary_report(analyses):
    """Generate a summary report for multiple file analyses."""
    lines = []
    lines.append("=" * 70)
    lines.append("          TANPURA BATCH ANALYSIS SUMMARY REPORT")
    lines.append("=" * 70)
    lines.append(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    lines.append(f"Total Files Analyzed: {len(analyses)}")
    lines.append("")
    lines.append("=" * 70)
    lines.append("                    FILE SUMMARIES")
    lines.append("=" * 70)
    lines.append("")
    
    for i, analysis in enumerate(analyses, 1):
        filename = os.path.basename(analysis['filepath'])
        lines.append(f"{i}. {filename}")
        lines.append(f"   Scale: {analysis['detected_scale']} (Sa = {analysis['base_freq']:.1f} Hz)")
        lines.append(f"   Score: {analysis['total_score']}/100")
        lines.append(f"   Volume Variation: {analysis['volume_variation']:.1f}%")
        lines.append(f"   Strong Harmonics: {analysis['strong_harmonics']}/20")
        
        if analysis['total_score'] >= 85:
            verdict = "EXCELLENT"
        elif analysis['total_score'] >= 70:
            verdict = "GOOD"
        elif analysis['total_score'] >= 55:
            verdict = "ACCEPTABLE"
        else:
            verdict = "NEEDS WORK"
        lines.append(f"   Verdict: {verdict}")
        lines.append("")
    
    lines.append("=" * 70)
    lines.append("                    STATISTICS")
    lines.append("=" * 70)
    lines.append("")
    
    scores = [a['total_score'] for a in analyses]
    avg_score = np.mean(scores)
    max_score = max(scores)
    min_score = min(scores)
    
    lines.append(f"Average Score: {avg_score:.1f}/100")
    lines.append(f"Highest Score: {max_score}/100")
    lines.append(f"Lowest Score: {min_score}/100")
    lines.append("")
    
    # Scale distribution
    scales = {}
    for a in analyses:
        scale = a['detected_scale']
        scales[scale] = scales.get(scale, 0) + 1
    
    lines.append("Scale Distribution:")
    for scale, count in sorted(scales.items()):
        lines.append(f"  {scale}: {count} file(s)")
    
    lines.append("")
    lines.append("=" * 70)
    lines.append("                    END OF SUMMARY")
    lines.append("=" * 70)
    
    return "\n".join(lines)


def analyze_wav(filepath):
    """Analyze a tanpura WAV file and return analysis data."""

    # Read file
    sample_rate, data = wavfile.read(filepath)

    # Convert to mono if stereo
    if len(data.shape) > 1:
        data = data.mean(axis=1)

    # Normalize
    data = data / np.max(np.abs(data))

    duration = len(data) / sample_rate

    # === AMPLITUDE ENVELOPE ANALYSIS ===
    window_size = int(sample_rate * 0.05)  # 50ms windows
    envelope = []
    for i in range(0, len(data) - window_size, window_size):
        chunk = data[i:i+window_size]
        envelope.append(np.sqrt(np.mean(chunk**2)))

    envelope = np.array(envelope)
    rms_min = envelope.min()
    rms_max = envelope.max()
    rms_mean = envelope.mean()
    volume_variation = (rms_max - rms_min) / rms_mean * 100

    # Envelope over time (20 segments)
    envelope_timeline = []
    chunk_size = max(1, len(envelope) // 20)
    for i in range(20):
        start = i * chunk_size
        end = min((i+1) * chunk_size, len(envelope))
        if start < len(envelope):
            avg = envelope[start:end].mean()
            envelope_timeline.append((i * 0.5, avg))

    # === FFT ANALYSIS ===
    segment_start = int(sample_rate * 1)
    segment_length = int(sample_rate * 4)  # 4 second segment
    if segment_start + segment_length > len(data):
        segment_start = 0
        segment_length = min(len(data), int(sample_rate * 4))

    segment = data[segment_start:segment_start + segment_length]
    window = np.hanning(len(segment))
    fft_result = np.abs(fft(segment * window))
    freqs = np.fft.fftfreq(len(segment), 1/sample_rate)

    pos_mask = freqs > 0
    freqs = freqs[pos_mask]
    fft_result = fft_result[pos_mask]

    # Find dominant frequency
    music_mask = (freqs >= 50) & (freqs <= 4000)
    music_freqs = freqs[music_mask]
    music_fft = fft_result[music_mask]
    dominant_idx = np.argmax(music_fft)
    dominant_freq = music_freqs[dominant_idx]

    # Detect scale
    detected_scale, base_freq, dominant_harmonic = detect_scale(dominant_freq)

    # Normalize FFT
    music_fft_norm = music_fft / music_fft.max()

    # === HARMONIC ANALYSIS ===
    harmonics = []
    for h in range(1, 21):
        expected = base_freq * h
        mask = (music_freqs >= expected - 8) & (music_freqs <= expected + 8)
        if mask.any():
            strength = music_fft_norm[mask].max() * 100
        else:
            strength = 0
        harmonics.append({
            'number': h,
            'expected_freq': expected,
            'strength': strength,
            'note': freq_to_note(expected)
        })

    # Top 30 frequency peaks
    peak_indices = np.argsort(fft_result)[-30:][::-1]
    top_peaks = []
    for idx in peak_indices:
        freq = freqs[idx]
        if 50 < freq < 4000:
            mag = fft_result[idx] / fft_result.max() * 100
            # Check if harmonic
            harmonic_num = None
            for h in range(1, 21):
                if abs(freq - base_freq * h) < 10:
                    harmonic_num = h
                    break
            top_peaks.append({
                'freq': freq,
                'note': freq_to_note(freq),
                'magnitude': mag,
                'harmonic': harmonic_num
            })

    # === SCORING ===
    h1 = harmonics[0]['strength']
    h4 = harmonics[3]['strength']
    h7 = harmonics[6]['strength']
    h11 = harmonics[10]['strength']
    h12 = harmonics[11]['strength']
    h17 = harmonics[16]['strength']

    strong_harmonics = sum(1 for h in harmonics if h['strength'] > 5)

    # Calculate scores
    scores = {}

    # Volume stability (30 pts)
    if volume_variation < 40:
        scores['volume'] = (30, "Excellent")
    elif volume_variation < 60:
        scores['volume'] = (22, "Good")
    elif volume_variation < 80:
        scores['volume'] = (15, "Moderate")
    else:
        scores['volume'] = (8, "Needs improvement")

    # Harmonic balance (25 pts)
    if h1 < 20 and h4 > h1:
        scores['balance'] = (25, "Authentic (weak fundamental)")
    elif h1 < 40:
        scores['balance'] = (18, "Good")
    else:
        scores['balance'] = (8, "Fundamental too strong")

    # Harmonic richness (25 pts)
    if strong_harmonics >= 12:
        scores['richness'] = (25, "Excellent jivari")
    elif strong_harmonics >= 8:
        scores['richness'] = (18, "Good")
    elif strong_harmonics >= 5:
        scores['richness'] = (12, "Moderate")
    else:
        scores['richness'] = (5, "Needs more harmonics")

    # Characteristic tones (20 pts)
    char_score = 0
    if h4 > 15: char_score += 5
    if h7 > 10: char_score += 5
    if h11 > 5: char_score += 5
    if h12 > 5: char_score += 5
    scores['characteristic'] = (char_score, "Present" if char_score >= 15 else "Partial")

    total_score = sum(s[0] for s in scores.values())

    return {
        'filepath': filepath,
        'sample_rate': sample_rate,
        'duration': duration,
        'rms_min': rms_min,
        'rms_max': rms_max,
        'rms_mean': rms_mean,
        'volume_variation': volume_variation,
        'envelope_timeline': envelope_timeline,
        'detected_scale': detected_scale,
        'base_freq': base_freq,
        'dominant_freq': dominant_freq,
        'dominant_harmonic': dominant_harmonic,
        'harmonics': harmonics,
        'top_peaks': top_peaks[:20],
        'strong_harmonics': strong_harmonics,
        'scores': scores,
        'total_score': total_score,
        'h1': h1, 'h4': h4, 'h7': h7, 'h11': h11, 'h12': h12, 'h17': h17
    }


def generate_report(analysis):
    """Generate a text report from analysis data."""

    lines = []
    lines.append("=" * 70)
    lines.append("              TANPURA AUDIO ANALYSIS REPORT")
    lines.append("=" * 70)
    lines.append(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    lines.append(f"File: {os.path.basename(analysis['filepath'])}")
    lines.append("")

    # Basic info
    lines.append("-" * 70)
    lines.append("AUDIO FILE INFO")
    lines.append("-" * 70)
    lines.append(f"  Sample Rate:    {analysis['sample_rate']} Hz")
    lines.append(f"  Duration:       {analysis['duration']:.2f} seconds")
    lines.append(f"  Detected Scale: {analysis['detected_scale']} (Sa = {analysis['base_freq']:.2f} Hz)")
    lines.append("")

    # Volume stability
    lines.append("-" * 70)
    lines.append("VOLUME STABILITY")
    lines.append("-" * 70)
    lines.append(f"  RMS Range:         {analysis['rms_min']:.4f} to {analysis['rms_max']:.4f}")
    lines.append(f"  RMS Mean:          {analysis['rms_mean']:.4f}")
    lines.append(f"  Volume Variation:  {analysis['volume_variation']:.1f}%")
    lines.append("")

    # Rating
    if analysis['volume_variation'] < 40:
        lines.append(f"  Rating: EXCELLENT - Very stable drone")
    elif analysis['volume_variation'] < 60:
        lines.append(f"  Rating: GOOD - Minor fluctuations")
    elif analysis['volume_variation'] < 80:
        lines.append(f"  Rating: MODERATE - Noticeable pumping")
    else:
        lines.append(f"  Rating: POOR - Significant pumping effect")
    lines.append("")

    # Amplitude timeline
    lines.append("  Amplitude over time:")
    for time, amp in analysis['envelope_timeline']:
        bars = int(amp * 40)
        lines.append(f"    {time:4.1f}s | {'█' * bars}")
    lines.append("")

    # Harmonic analysis
    lines.append("-" * 70)
    lines.append("HARMONIC SPECTRUM")
    lines.append("-" * 70)
    lines.append(f"  Base Frequency (Sa): {analysis['base_freq']:.2f} Hz")
    lines.append(f"  Dominant Frequency:  {analysis['dominant_freq']:.2f} Hz (H{analysis['dominant_harmonic']})")
    lines.append(f"  Strong Harmonics:    {analysis['strong_harmonics']}/20 above 5%")
    lines.append("")
    lines.append("  Harmonic strengths (Real tanpura: H1 weak, H4/H7/H11/H12 strong):")
    lines.append("  " + "-" * 66)

    for h in analysis['harmonics']:
        marker = ""
        if h['number'] == 1:
            marker = " <-- should be WEAK"
        elif h['number'] in [4, 7]:
            marker = " * characteristic"
        elif h['number'] in [11, 12, 17]:
            marker = " * shimmer"

        if h['strength'] > 3:
            bars = int(h['strength'] / 4)
            lines.append(f"    H{h['number']:2d} | {h['expected_freq']:7.1f} Hz | {'█' * bars:<25} {h['strength']:5.1f}%{marker}")
    lines.append("")

    # Authenticity check
    lines.append("-" * 70)
    lines.append("AUTHENTICITY CHECK")
    lines.append("-" * 70)

    h1_status = "WEAK (authentic)" if analysis['h1'] < 20 else "MODERATE" if analysis['h1'] < 40 else "TOO STRONG"
    lines.append(f"  Fundamental (H1):  {analysis['h1']:5.1f}%  - {h1_status}")
    lines.append("")
    lines.append("  Characteristic harmonics:")
    lines.append(f"    H4:  {analysis['h4']:5.1f}%  {'[OK]' if analysis['h4'] > 15 else '[WEAK]'}")
    lines.append(f"    H7:  {analysis['h7']:5.1f}%  {'[OK]' if analysis['h7'] > 10 else '[WEAK]'}")
    lines.append(f"    H11: {analysis['h11']:5.1f}%  {'[OK]' if analysis['h11'] > 5 else '[WEAK]'}")
    lines.append(f"    H12: {analysis['h12']:5.1f}%  {'[OK]' if analysis['h12'] > 5 else '[WEAK]'}")
    lines.append(f"    H17: {analysis['h17']:5.1f}%  {'[OK]' if analysis['h17'] > 3 else '[WEAK]'}")
    lines.append("")

    # Top frequency peaks
    lines.append("-" * 70)
    lines.append("TOP FREQUENCY PEAKS")
    lines.append("-" * 70)
    for i, peak in enumerate(analysis['top_peaks'][:15]):
        h_marker = f"(H{peak['harmonic']})" if peak['harmonic'] else ""
        lines.append(f"    {i+1:2d}. {peak['freq']:8.2f} Hz  {peak['note']:4s}  {peak['magnitude']:5.1f}%  {h_marker}")
    lines.append("")

    # Overall score
    lines.append("-" * 70)
    lines.append("OVERALL SCORE")
    lines.append("-" * 70)
    lines.append(f"  Volume Stability:    [{analysis['scores']['volume'][0]:2d}/30] {analysis['scores']['volume'][1]}")
    lines.append(f"  Harmonic Balance:    [{analysis['scores']['balance'][0]:2d}/25] {analysis['scores']['balance'][1]}")
    lines.append(f"  Harmonic Richness:   [{analysis['scores']['richness'][0]:2d}/25] {analysis['scores']['richness'][1]}")
    lines.append(f"  Characteristic Tones:[{analysis['scores']['characteristic'][0]:2d}/20] {analysis['scores']['characteristic'][1]}")
    lines.append("  " + "-" * 40)
    lines.append(f"  TOTAL SCORE:         [{analysis['total_score']:2d}/100]")
    lines.append("")

    if analysis['total_score'] >= 85:
        lines.append("  VERDICT: EXCELLENT - Sounds like authentic tanpura!")
    elif analysis['total_score'] >= 70:
        lines.append("  VERDICT: GOOD - Close to authentic tanpura sound")
    elif analysis['total_score'] >= 55:
        lines.append("  VERDICT: ACCEPTABLE - Getting closer, needs refinement")
    else:
        lines.append("  VERDICT: NEEDS WORK - Significant improvements needed")

    lines.append("")
    lines.append("=" * 70)
    lines.append("                    END OF REPORT")
    lines.append("=" * 70)

    return "\n".join(lines)


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 analyze_tanpura.py <audio_file_or_folder>")
        print("Examples:")
        print("  Single file: python3 analyze_tanpura.py tanpura_1769514216248.wav")
        print("  Single file: python3 analyze_tanpura.py tanpura_recording.mp3")
        print("  Folder:      python3 analyze_tanpura.py ./recordings/")
        sys.exit(1)

    path = sys.argv[1]

    if not os.path.exists(path):
        print(f"Error: Path not found: {path}")
        sys.exit(1)

    # Check if path is a directory
    if os.path.isdir(path):
        print(f"Analyzing all audio files in folder: {path}")
        print("="*70)
        
        # Create results folder
        results_folder = ensure_results_folder(path)
        print(f"Results will be saved to: {results_folder}\n")
        
        audio_files = find_audio_files(path)
        
        if not audio_files:
            print(f"No audio files (.wav or .mp3) found in {path}")
            sys.exit(1)
        
        print(f"Found {len(audio_files)} audio file(s)\n")
        
        analyses = []
        for i, filepath in enumerate(audio_files, 1):
            filename = os.path.basename(filepath)
            print(f"[{i}/{len(audio_files)}] Analyzing: {filename}")
            
            analysis = analyze_single_file(filepath)
            
            if analysis:
                analyses.append(analysis)
                
                # Save individual report in results folder
                report = generate_report(analysis)
                base_name = os.path.splitext(os.path.basename(filepath))[0]
                report_file = os.path.join(results_folder, f"{base_name}_analysis.txt")
                
                with open(report_file, 'w') as f:
                    f.write(report)
                
                print(f"  ✓ Score: {analysis['total_score']}/100 - Report: {os.path.basename(report_file)}")
            else:
                print(f"  ✗ Failed to analyze")
            print()
        
        if analyses:
            # Generate summary report in results folder
            summary = generate_summary_report(analyses)
            summary_file = os.path.join(results_folder, "_batch_analysis_summary.txt")
            
            with open(summary_file, 'w') as f:
                f.write(summary)
            
            print("="*70)
            print(f"Batch analysis complete!")
            print(f"Successfully analyzed: {len(analyses)}/{len(audio_files)} files")
            print(f"Summary report saved to: {summary_file}")
            print(f"\nQuick Summary:")
            avg_score = np.mean([a['total_score'] for a in analyses])
            print(f"  Average Score: {avg_score:.1f}/100")
        else:
            print("No files were successfully analyzed.")
            sys.exit(1)
    
    else:
        # Single file analysis
        filepath = path
        file_ext = os.path.splitext(filepath)[1].lower()
        
        if file_ext not in ['.wav', '.mp3']:
            print(f"Error: Unsupported file format: {file_ext}")
            print("Supported formats: .wav, .mp3")
            sys.exit(1)
        
        if file_ext == '.mp3' and not HAS_PYDUB:
            print("Error: pydub is required to process MP3 files.")
            print("Install it with: pip3 install pydub")
            print("Note: You may also need ffmpeg installed on your system.")
            sys.exit(1)
        
        # Create results folder in the same directory as the input file
        file_dir = os.path.dirname(os.path.abspath(filepath))
        results_folder = ensure_results_folder(file_dir)
        
        print(f"Analyzing: {filepath}")
        print("Please wait...")
        
        analysis = analyze_single_file(filepath)
        
        if not analysis:
            print(f"Error: Failed to analyze file")
            sys.exit(1)
        
        report = generate_report(analysis)
        base_name = os.path.splitext(os.path.basename(filepath))[0]
        report_file = os.path.join(results_folder, f"{base_name}_analysis.txt")
        
        with open(report_file, 'w') as f:
            f.write(report)
        
        print(f"\nAnalysis complete!")
        print(f"Report saved to: {report_file}")
        print(f"\nQuick Summary:")
        print(f"  Scale: {analysis['detected_scale']} (Sa = {analysis['base_freq']:.1f} Hz)")
        print(f"  Volume Variation: {analysis['volume_variation']:.1f}%")
        print(f"  Strong Harmonics: {analysis['strong_harmonics']}/20")
        print(f"  Total Score: {analysis['total_score']}/100")


if __name__ == "__main__":
    main()
