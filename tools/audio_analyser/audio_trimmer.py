import os
import subprocess
from pathlib import Path

def batch_trim_audio(input_folder, output_folder, duration_seconds=60):
    # Setup paths
    src_dir = Path(input_folder)
    dest_dir = Path(output_folder)
    dest_dir.mkdir(parents=True, exist_ok=True)

    # Supported audio extensions
    extensions = ('.wav', '.mp3', '.flac', '.m4a', '.ogg', '.aac')

    print(f"Scanning: {src_dir}...")

    for file_path in src_dir.iterdir():
        if file_path.suffix.lower() in extensions:
            output_file = dest_dir / f"trimmed_{file_path.name}"
            
            # FFmpeg Command
            # -ss 0: Start at beginning
            # -t: Duration in seconds
            # -c copy: Fast trim without re-encoding
            command = [
                'ffmpeg',
                '-i', str(file_path),
                '-ss', '0',
                '-t', str(duration_seconds),
                '-c', 'copy',
                str(output_file),
                '-y', # Overwrite if exists
                '-loglevel', 'error' # Keep console clean
            ]

            try:
                subprocess.run(command, check=True)
                print(f"Done: {file_path.name} -> {output_file.name}")
            except subprocess.CalledProcessError as e:
                print(f"Failed to process {file_path.name}: {e}")

if __name__ == "__main__":
    # Update these paths to your folders
    SOURCE = "./Tanpura_Samples_DSP"
    OUTPUT = "./trimmed_tanpura_files"
    
    batch_trim_audio(SOURCE, OUTPUT)