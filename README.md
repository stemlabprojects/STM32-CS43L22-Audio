# STM32 CS43L22 Audio

An STM32F407-based audio development platform using the **CS43L22 audio codec**.

This repository is being developed as a foundation for learning and building embedded audio applications — starting from simple tone generation and progressing toward audio playback, voice generation, and eventually **Text-to-Speech (TTS)** applications.

---

## 🎯 Project Goal

The goal of this project is to build a reliable and reusable audio platform around:

* **STM32F407**
* **CS43L22 audio codec**
* **I2C** for codec configuration
* **I2S** for digital audio transmission
* **DMA** for continuous audio transfer
* **STM32CubeIDE / STM32CubeMX**
* Embedded PCM audio generation and playback

The development approach is intentionally incremental:

```text
Basic Beep
    │
    ▼
Tone Generation
    │
    ▼
Melody / Sound Effects
    │
    ▼
PCM / WAV Playback
    │
    ▼
SD Card / External Audio
    │
    ▼
Voice / Speech Playback
    │
    ▼
Text-to-Speech
```

Each stage will be developed and tested as an independent project.

---

# 🔊 Audio Platform

The central hardware in this repository is the **CS43L22 stereo audio codec**.

The STM32F407 communicates with the codec through two interfaces:

```text
                  STM32F407
                      │
          ┌───────────┴───────────┐
          │                       │
        I2C1                    I2S3
          │                       │
          │                       │
          ▼                       ▼
   Codec Configuration      Digital Audio Data
          │                       │
          └───────────┬───────────┘
                      ▼
                  CS43L22
                      │
                      ▼
               Audio Output
```

### Control interface

**I2C1** is used to configure and control the CS43L22.

### Audio interface

**I2S3** is used to transmit digital PCM audio data to the codec.

### DMA

DMA is used for audio transmission so that the CPU does not need to manually transfer every audio sample.

This provides the foundation for continuous audio playback.

---

# 🧠 Development Philosophy

The repository is organized as a collection of **complete STM32 projects**.

Each project under `Projects/` is intended to be:

* independently buildable
* independently testable
* documented
* based on the same STM32F407 + CS43L22 audio platform where appropriate

This makes it possible to experiment with one audio feature at a time without turning the repository into one large, difficult-to-maintain application.

---

# 📁 Repository Structure

```text
STM32-CS43L22-Audio/
│
├── Projects/
│   │
│   ├── 01_BEEP/
│   │   ├── Core/
│   │   ├── Drivers/
│   │   ├── Middlewares/
│   │   ├── USB_HOST/
│   │   ├── F407_CS43L22_AUDIO.ioc
│   │   ├── .project
│   │   ├── .cproject
│   │   └── README.md
│   │
│   ├── 02_...
│   │
│   ├── 03_...
│   │
│   └── ...
│
└── README.md
```

The root `README.md` describes the overall audio platform.

Each individual project contains its own detailed README describing its particular implementation.

---

# 🚀 Projects

## 01 — BEEP

**Status: Working**

The first project establishes the basic audio path between the STM32F407 and CS43L22.

It demonstrates:

* CS43L22 initialization
* I2C codec configuration
* I2S3 audio transmission
* DMA-based audio transfer
* PCM audio generation
* sine-wave generation
* audio volume control
* fade-in / fade-out
* headphone audio output

### Current demonstration

```text
Waveform       : Sine wave
Frequency      : 1 kHz
Duration       : 80 ms
Sample rate    : 48 kHz
Volume         : 70
Output         : Headphone
Transfer       : I2S3 + DMA
Codec control  : I2C1
```

See:

`Projects/01_BEEP/README.md`

for the complete hardware, clock, peripheral, DMA, audio and software configuration.

---

# ⚙️ Base Hardware Configuration

The initial platform uses the following STM32F407 configuration:

| Parameter         | Configuration |
| ----------------- | ------------- |
| MCU               | STM32F407VGT6 |
| System Clock      | 168 MHz       |
| HSE               | 8 MHz         |
| APB1              | 42 MHz        |
| APB2              | 84 MHz        |
| Audio Codec       | CS43L22       |
| Codec Control     | I2C1          |
| Audio Interface   | I2S3          |
| Audio Transfer    | DMA           |
| Audio Sample Rate | 48 kHz        |
| Audio Output      | Headphone     |

The exact configuration may evolve as new projects are added.

---

# 🔌 Audio Connections

The initial CS43L22 audio interface uses:

| STM32F407 | Function | CS43L22 |
| --------- | -------- | ------- |
| PB6       | I2C1_SCL | SCL     |
| PB9       | I2C1_SDA | SDA     |
| PA4       | I2S3_WS  | LRCK    |
| PC7       | I2S3_MCK | MCLK    |
| PC10      | I2S3_SCK | SCLK    |
| PC12      | I2S3_SD  | SDIN    |
| PD4       | GPIO     | RESET   |

These connections are documented in greater detail in the individual project README.

---

# 🕐 Clock Architecture

The STM32F407 runs with a **168 MHz system clock**.

The initial clock arrangement is:

```text
                    8 MHz HSE
                        │
                        ▼
                       PLL
                        │
                        ▼
                  SYSCLK 168 MHz
                        │
             ┌──────────┴──────────┐
             │                     │
          AHB/HCLK              APB buses
          168 MHz             ┌────┴────┐
                              │         │
                           APB1       APB2
                           42 MHz     84 MHz
```

A dedicated I2S clock is used for the audio interface.

The audio clock configuration is selected to provide approximately **48 kHz audio sampling**.

Detailed clock parameters belong to the individual project documentation because different future applications may use different audio configurations.

---

# 🎵 Planned Development Roadmap

The project will grow progressively.

### Stage 1 — Basic Audio

* [x] CS43L22 initialization
* [x] I2C communication
* [x] I2S communication
* [x] DMA audio transfer
* [x] 1 kHz beep
* [x] Volume control
* [x] Fade-in / fade-out

### Stage 2 — Tone Generation

* [ ] Variable frequency tone
* [ ] Variable duration
* [ ] Multiple waveforms
* [ ] Frequency sweep
* [ ] Musical notes
* [ ] Melody generation

### Stage 3 — Audio Playback

* [ ] PCM playback
* [ ] WAV file playback
* [ ] SD card audio
* [ ] Audio file management
* [ ] Playback controls

### Stage 4 — Voice

* [ ] Speech sample playback
* [ ] Voice prompts
* [ ] Embedded voice library
* [ ] Voice notification system

### Stage 5 — Text-to-Speech

* [ ] Text input
* [ ] Text processing
* [ ] Speech synthesis
* [ ] Audio streaming to CS43L22
* [ ] Embedded TTS experiments

---

# 🧩 Why Separate Projects?

Audio development becomes complicated very quickly.

Instead of putting everything into one application, this repository follows a progressive project structure:

```text
01_BEEP
   │
   ▼
02_TONE
   │
   ▼
03_MELODY
   │
   ▼
04_WAV_PLAYER
   │
   ▼
05_SD_AUDIO
   │
   ▼
06_VOICE
   │
   ▼
07_TEXT_TO_SPEECH
```

Each project should answer one specific engineering question and provide a working foundation for the next project.

This makes debugging and learning much easier.

---

# 🛠️ Development Environment

The projects are primarily developed using:

* **STM32CubeIDE**
* **STM32CubeMX**
* STM32 HAL
* GCC ARM Embedded Toolchain
* Git / GitHub

The exact STM32Cube firmware package and CubeMX configuration are documented within each project.

---

# 📚 Documentation Philosophy

Every project should document enough information to reproduce the project later.

Where applicable, project documentation should include:

* MCU
* board/hardware
* clock configuration
* GPIO configuration
* peripheral configuration
* I2C configuration
* I2S configuration
* DMA configuration
* audio sample rate
* codec configuration
* pin connections
* software architecture
* build instructions
* expected output
* known limitations

The intention is that the project should remain understandable even after a long period away from the development work.

---

# 🔬 Future Applications

Once the basic audio platform is stable, this hardware/software foundation can be used for applications such as:

* alarms
* electronic instruments
* audio indicators
* weather-station voice announcements
* embedded voice interfaces
* speech prompts
* educational audio experiments
* WAV/PCM players
* Text-to-Speech systems

---

# 📌 Current Status

**Current project:** `01_BEEP`

**Status:** Working

The first milestone is complete:

> Generate a digitally synthesized 1 kHz sine-wave beep on the STM32F407 and output it through the CS43L22 using I2S and DMA.

The next projects will build upon this working audio foundation.

---

# 👨‍💻 Project

**Repository:** STM32-CS43L22-Audio

**Organization:** stemlabprojects

This repository is an ongoing embedded-audio development project. Hardware configuration, software architecture and project documentation will evolve as new experiments and applications are added.
