#!/bin/bash

# Display menu
echo "==============================="
echo "TANPURA AUDIO ANALYSIS TOOL"
echo "==============================="
echo "Choose an option:"
echo "1. Analysis from file (pull from emulator)"
echo "2. Analysis from YouTube"
echo "3. Analysis from folder (batch process)"
echo "==============================="
read -p "Enter your choice (1, 2, or 3): " choice

case $choice in
  1)
    # Option 1: Analysis from emulator file
    read -p "Enter the audio filename (from emulator): " AUDIO_FILE_NAME
    
    if [ -z "$AUDIO_FILE_NAME" ]; then
      echo "Error: Filename cannot be empty"
      exit 1
    fi
    
    echo "\n==============================="
    echo "1. CHECKING CONNECTED DEVICES"
    
    # Check if any devices are connected
    DEVICE_COUNT=$(adb devices | grep -v "List" | grep -c "device$")
    
    if [ "$DEVICE_COUNT" -eq 0 ]; then
      echo "Error: No devices/emulators connected"
      echo ""
      echo "Please ensure:"
      echo "  1. Your Android device is connected via USB or emulator is running"
      echo "  2. USB debugging is enabled on your device"
      echo "  3. Run 'adb devices' to verify connection"
      echo ""
      echo "To start an emulator, run: emulator -avd <avd_name>"
      exit 1
    fi
    
    echo "Found $DEVICE_COUNT device(s) connected"
    
    echo "2. PULLING AUDIO FILE: $AUDIO_FILE_NAME"
    
    # Add .wav extension if not present
    if [[ ! "$AUDIO_FILE_NAME" =~ \.(wav|mp3)$ ]]; then
      AUDIO_FILE_NAME="${AUDIO_FILE_NAME}.wav"
      echo "Using filename: $AUDIO_FILE_NAME"
    fi
    
    AUDIO_FILE_PATH="/storage/emulated/0/Android/data/com.tanpurax.tanpura.dev/files/$AUDIO_FILE_NAME"
    
    # Check if file exists on device
    adb shell "test -f '$AUDIO_FILE_PATH' && echo 'exists' || echo 'notfound'" | grep -q "exists"
    
    if [ $? -ne 0 ]; then
      echo "Error: File not found on device: $AUDIO_FILE_PATH"
      echo ""
      echo "Checking available files in the directory..."
      adb shell "ls -1 /storage/emulated/0/Android/data/com.tanpurax.tanpura.dev/files/ 2>/dev/null" | head -10
      exit 1
    fi
    
    adb pull "$AUDIO_FILE_PATH" .
    
    if [ $? -ne 0 ]; then
      echo "Error: Failed to pull file from emulator"
      exit 1
    fi
    
    echo "3. ANALYSING AUDIO FILE: $AUDIO_FILE_NAME"
    
    python3 analyze_tanpura.py "$AUDIO_FILE_NAME"
    
    echo "4. ANALYSIS DONE REPORT GENERATED"
    echo "==============================="
    ;;
    
  2)
    # Option 2: Analysis from YouTube
    read -p "Enter the YouTube URL: " YOUTUBE_URL
    
    if [ -z "$YOUTUBE_URL" ]; then
      echo "Error: YouTube URL cannot be empty"
      exit 1
    fi
    
    # Create youtube folder if it doesn't exist
    mkdir -p youtube
    
    echo "\n==============================="
    echo "1. DOWNLOADING AUDIO FROM YOUTUBE"
    
    # Download audio file with youtube suffix to youtube folder
    yt-dlp \
      --no-cache-dir \
      --force-ipv4 \
      -x --audio-format wav \
      --extractor-args "youtube:player_client=android" \
      -o "youtube/%(title)s_youtube.%(ext)s" \
      "$YOUTUBE_URL"
    
    if [ $? -ne 0 ]; then
      echo "Error: Failed to download audio from YouTube"
      exit 1
    fi
    
    # Get the downloaded filename
    AUDIO_FILE_NAME=$(ls -t youtube/*_youtube.wav 2>/dev/null | head -1)
    
    if [ -z "$AUDIO_FILE_NAME" ]; then
      echo "Error: Failed to find downloaded audio file"
      exit 1
    fi
    
    echo "Downloaded file: $AUDIO_FILE_NAME"
    
    echo "2. ANALYSING AUDIO FILE: $AUDIO_FILE_NAME"
    
    python3 analyze_tanpura.py "$AUDIO_FILE_NAME"
    
    echo "3. ANALYSIS DONE REPORT GENERATED"
    echo "==============================="
    ;;
    
  3)
    # Option 3: Analysis from folder
    read -p "Enter the folder path: " FOLDER_PATH
    
    if [ -z "$FOLDER_PATH" ]; then
      echo "Error: Folder path cannot be empty"
      exit 1
    fi
    
    # Expand tilde and make absolute path
    FOLDER_PATH="${FOLDER_PATH/#\~/$HOME}"
    
    if [ ! -d "$FOLDER_PATH" ]; then
      echo "Error: Folder not found: $FOLDER_PATH"
      exit 1
    fi
    
    echo "\n==============================="
    echo "1. CHECKING FOLDER: $FOLDER_PATH"
    
    # Count audio files
    AUDIO_COUNT=$(find "$FOLDER_PATH" -maxdepth 1 -type f \( -iname "*.wav" -o -iname "*.mp3" \) | wc -l)
    
    if [ "$AUDIO_COUNT" -eq 0 ]; then
      echo "Error: No audio files (.wav or .mp3) found in folder"
      exit 1
    fi
    
    echo "Found $AUDIO_COUNT audio file(s)"
    
    echo "2. ANALYSING ALL AUDIO FILES IN FOLDER"
    
    python3 analyze_tanpura.py "$FOLDER_PATH"
    
    echo "3. BATCH ANALYSIS DONE, REPORTS GENERATED"
    echo "==============================="
    ;;
  
  *)
    echo "Error: Invalid choice. Please enter 1, 2, or 3"
    exit 1
    ;;
esac