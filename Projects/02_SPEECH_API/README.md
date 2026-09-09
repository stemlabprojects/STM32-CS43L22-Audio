# Project 02 — STM32 CS43L22 Speech API

## STM32F407 Discovery + CS43L22 Audio Codec

This project is the second project in the STM32 CS43L22 Audio series.

The purpose of this project is to develop a reusable speech playback engine for the **STM32F407 Discovery board** using the onboard **CS43L22 audio codec**.

The project does not perform text-to-speech directly on the STM32.

Instead, speech is generated offline on a computer, converted into raw PCM audio, converted into C arrays, stored in the STM32 firmware, and then played through the CS43L22 codec.

The long-term objective is to use this system for spoken sensor values such as:

> Temperature is 23 point 5 degree celsius.

or:

> Humidity is 65 percent.

---

# 1. Project Series

The GitHub repository contains multiple projects demonstrating the development of the audio system.

```text
STM32-CS43L22-Audio
│
└── Projects
    │
    ├── 01_BEEP
    │
    └── 02_SPEECH_API
