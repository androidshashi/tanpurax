# TanpuraX 🎵

TanpuraX is a **procedural audio synthesis–based Tanpura application** built using **Flutter** and a **custom native audio engine**.
This repository follows a **Flutter-first monorepo structure**, where the main app and the audio engine package live together.

> ⚠️ This is a **private repository**. This codebase is proprietary and not intended for open-source use.

---

## 🎯 Project Objective

- Build a **high-quality Tanpura app**
- Generate sound **procedurally** (no recorded loops)
- Achieve **low-latency, stable playback**
- Support **continuous long-duration sessions**
- Maintain **accurate musical pitch**

---

## 🧠 Architecture Overview

Flutter App (UI)  
↓  
Flutter Plugin / Dart API  
↓  
Platform Channels (JNI)  
↓  
Native Audio Engine (C++)  
↓  
Oboe (AAudio / OpenSL ES)

---

## 🛠 Technology Stack

### Flutter / Dart

- Flutter
- Dart

### Native Audio

- Android NDK
- C++
- Oboe
- JNI

### Tooling

- Gradle
- CMake
- Flutter tooling

---

## 📁 Monorepo Structure (Actual)

```
tanpurax/
├── apps/
│   └── tanpura/                 # Main Flutter application
│       ├── android/
│       ├── ios/
│       ├── lib/
│       ├── test/
│       ├── pubspec.yaml
│       └── README.md
│
├── packages/
│   └── tanpura_engine/          # Flutter plugin + native audio engine
│       ├── android/             # Android plugin & NDK code
│       ├── lib/                 # Dart API
│       ├── example/             # Example Flutter app
│       ├── test/
│       ├── pubspec.yaml
│       ├── CHANGELOG.md
│       └── LICENSE
│
├── docs/                         # Internal documentation
│
└── README.md                     # Root documentation
```

---

## 🔊 tanpura_engine Package

- Flutter plugin exposing Tanpura controls
- Native Android audio engine (C++ / Oboe)
- Real-time waveform synthesis
- Dedicated audio thread
- Designed for long-running playback

---

## 🚀 Running the App

### Prerequisites

- Flutter SDK
- Android Studio
- Android NDK
- CMake

### Steps

```bash
cd apps/tanpura
flutter pub get
flutter run
```

---

## 🔒 Repository Policy

This repository is **private** and intended only for:

- Personal development
- Internal experimentation
- Commercial product development

**No redistribution or reuse without explicit permission.**

---

## 📌 Project Status

- ✅ Monorepo structure finalized
- ✅ Flutter app integrated with audio engine
- ✅ Native audio thread stable
- 🚧 UI refinement
- 🚧 iOS native engine (planned)

---

## 📜 License

All rights reserved.  
This project is proprietary and confidential.

---

## 👤 Author

Shashi Kumar  
Senior Mobile App Developer  
Flutter • Android • Audio Systems

---

## 📝 Notes

- Audio is fully procedural (no samples)
- Optimized for correctness and latency
- Structure is intentionally scalable

---
