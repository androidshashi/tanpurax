# TanpuraX 🎵

TanpuraX is a **procedural audio synthesis–based Tanpura application** built using **Flutter** and a **custom native audio engine**.  
The project focuses on **high-quality, low-latency sound generation** using Android NDK instead of pre-recorded audio loops.

> ⚠️ This is a **private repository**. The codebase is not intended for open-source distribution.

---

## 🎯 Project Goal

- Create a **studio-quality Tanpura app**
- Use **procedural sound synthesis** instead of static audio files
- Achieve **low-latency and glitch-free playback**
- Support **long-duration continuous playback**
- Maintain **musical pitch accuracy** (Sa / Pa / variations)

---

## 🧠 Architecture Overview

Flutter UI (Dart)  
↓  
Platform Channels (JNI)  
↓  
Native Audio Engine (C++)  
↓  
Oboe (AAudio / OpenSL ES)  
↓  
Android Audio Hardware

---

## 🛠 Tech Stack

### Frontend

- Flutter
- Dart

### Native / Audio

- Android NDK
- C++
- Oboe (Low-latency audio)
- JNI

### Build Tools

- Gradle
- CMake

---

## 🔊 Audio Engine Highlights

- Procedural waveform generation
- Real-time frequency control
- Sample-accurate timing
- Dedicated audio thread
- Designed for long, uninterrupted playback

---

## 📁 Project Structure (Simplified)

tanpurax/  
├── android/  
│ ├── app/  
│ └── tanpura_engine/  
│ ├── audio_engine.cpp  
│ ├── tanpura_engine.cpp  
│ ├── CMakeLists.txt  
│ └── oboe/  
├── lib/  
│ ├── main.dart  
│ └── ui/  
├── assets/  
└── README.md

---

## 🚀 Build & Run

### Prerequisites

- Flutter SDK
- Android Studio
- Android NDK
- CMake

### Run

flutter pub get  
flutter run

---

## 🔒 Repository Access

This repository is **private** and intended only for:

- Internal development
- Personal experimentation
- Commercial product development

Redistribution, copying, or reuse without permission is prohibited.

---

## 📌 Current Status

- Native audio engine working
- Audio thread stable
- Pitch presets & UI tuning in progress
- iOS audio engine planned

---

## 📜 License

All rights reserved.  
This project is proprietary and confidential.

---

## 👤 Author

Shashi Kumar  
Senior Mobile App Developer  
Flutter • Android • Audio Systems
