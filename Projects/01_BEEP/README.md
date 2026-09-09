# STM32 CS43L22 Beep

A simple STM32F407 audio project that generates a **1 kHz sine-wave beep** and outputs it through the **CS43L22 audio codec** using **I2S + DMA**.

This project is intended as a starting point for developing audio applications with the STM32F407 and CS43L22 codec.

---

## 1. Project Overview

The project demonstrates the complete digital audio path:

```text
STM32F407
   │
   ├── I2C1 ───────────────► CS43L22 control/configuration
   │
   └── I2S3 + DMA ─────────► CS43L22 digital audio data
                                  │
                                  ▼
                             Headphone Output
```

The application initializes the CS43L22 codec, starts I2S audio playback using DMA, generates a sine-wave tone in memory, and sends the audio samples to the codec.

### Current demonstration

```text
Frequency : 1 kHz
Duration  : 80 ms
Volume    : 70
Sample rate: 48 kHz
Output    : Headphone
```

The main loop repeats the beep with delays between tones.

---

# 2. Hardware

## MCU

* **Microcontroller:** STM32F407VGTx
* **Device:** STM32F407VGT6
* **Family:** STM32F4
* **Package:** LQFP100

## Audio Codec

* **Codec:** CS43L22
* **Control interface:** I2C
* **Audio interface:** I2S
* **Output:** Headphone output

The project uses the STM32F4-Discovery audio BSP to communicate with and control the CS43L22.

---

# 3. Development Environment

The project was generated/configured using:

* **STM32CubeIDE**
* **STM32CubeMX configuration:** 6.18.1
* **STM32Cube firmware package:** STM32Cube FW_F4 V1.28.3
* **Compiler:** GCC
* **HAL:** STM32F4 HAL drivers

The CubeMX configuration is stored in:

```text
F407_CS43L22_AUDIO.ioc
```

---

# 4. Clock Configuration

The STM32F407 system clock is generated from an external **8 MHz HSE** oscillator.

### Main PLL

| Parameter |   Value |
| --------- | ------: |
| HSE       |   8 MHz |
| PLLM      |       8 |
| PLLN      |     336 |
| PLLP      |       2 |
| PLLQ      |       7 |
| SYSCLK    | 168 MHz |

The main clock tree is:

```text
HSE
8 MHz
 │
 ▼
PLL
 │
 ├── SYSCLK = 168 MHz
 │
 ├── HCLK   = 168 MHz
 │
 ├── APB1   = 42 MHz
 │
 └── APB2   = 84 MHz
```

### Bus clocks

| Clock                | Frequency |
| -------------------- | --------: |
| SYSCLK               |   168 MHz |
| HCLK / AHB           |   168 MHz |
| APB1                 |    42 MHz |
| APB1 Timer clock     |    84 MHz |
| APB2                 |    84 MHz |
| APB2 Timer clock     |   168 MHz |
| PLLQ / 48 MHz domain |    48 MHz |

---

# 5. I2S Audio Clock

The audio interface uses the dedicated **I2S PLL**.

| Parameter                 |      Value |
| ------------------------- | ---------: |
| I2S clock source          |     PLLI2S |
| PLLI2SN                   |        258 |
| PLLI2SR                   |          3 |
| I2S clock                 |     86 MHz |
| Requested audio frequency |     48 kHz |
| Actual audio frequency    | 47.991 kHz |
| Frequency error           |     -0.01% |

The small difference between the requested and actual sample frequency is due to the available I2S clock divider configuration.

---

# 6. I2S3 Configuration

The CS43L22 receives digital audio through **SPI3 configured as I2S**.

### Configuration

| Parameter       | Setting         |
| --------------- | --------------- |
| Peripheral      | SPI3 / I2S3     |
| Mode            | Master Transmit |
| Standard        | Philips I2S     |
| Data format     | 16-bit          |
| Audio frequency | 48 kHz          |
| MCLK            | Enabled         |
| Clock source    | PLL             |
| Full duplex     | Disabled        |
| Clock polarity  | Low             |

The important audio signals are:

```text
I2S3_WS  → LRCK
I2S3_MCK → MCLK
I2S3_SCK → SCLK
I2S3_SD  → SDIN
```

---

# 7. CS43L22 Connections

## I2C Control Interface

The CS43L22 is configured through I2C1.

| STM32F407 Pin | STM32 Function | CS43L22 Signal |
| ------------- | -------------- | -------------- |
| PB6           | I2C1_SCL       | SCL            |
| PB9           | I2C1_SDA       | SDA            |

Both lines use the configured I2C alternate-function open-drain interface with pull-ups.

---

## I2S Audio Interface

| STM32F407 Pin | STM32 Function | CS43L22 Signal |
| ------------- | -------------- | -------------- |
| PA4           | I2S3_WS        | LRCK           |
| PC7           | I2S3_MCK       | MCLK           |
| PC10          | I2S3_SCK       | SCLK           |
| PC12          | I2S3_SD        | SDIN           |

---

## Codec Reset

| STM32F407 Pin | Function    | CS43L22 |
| ------------- | ----------- | ------- |
| PD4           | GPIO output | RESET   |

The codec reset line is controlled as a normal GPIO output.

---

# 8. DMA Configuration

Audio samples are transmitted using DMA so that the CPU does not have to manually send every sample to the I2S peripheral.

### DMA configuration

| Parameter            | Setting             |
| -------------------- | ------------------- |
| DMA controller       | DMA1                |
| Stream               | DMA1 Stream 5       |
| Channel              | Channel 0           |
| Direction            | Memory → Peripheral |
| Peripheral           | SPI3 TX             |
| Peripheral alignment | Half-word           |
| Memory alignment     | Half-word           |
| Memory increment     | Enabled             |
| Peripheral increment | Disabled            |
| Mode                 | Circular            |
| Priority             | Very High           |
| FIFO                 | Disabled            |

DMA interrupt:

```text
DMA1_Stream5_IRQn
Priority: 0
Subpriority: 0
```

---

# 9. Audio Buffer

The audio player uses a static stereo PCM buffer.

```c
#define AUDIO_PLAYER_BUFFER_SAMPLES   4800U
```

At 48 kHz:

```text
4800 samples / 48000 samples per second
= 0.1 second
= 100 ms
```

Therefore, the buffer represents **100 ms of audio per channel**.

The samples are stored as interleaved stereo data:

```text
L0, R0, L1, R1, L2, R2, ...
```

The buffer contains:

```text
4800 samples × 2 channels
= 9600 int16_t values
```

---

# 10. Beep Generation

The beep is generated digitally in software.

The project creates a sine wave using:

```c
sinf()
```

The phase is calculated from:

```text
phase = 2π × frequency × sample_index / sample_rate
```

The resulting sample is scaled by the configured amplitude.

The maximum amplitude used by the audio player is:

```c
#define AUDIO_PLAYER_AMPLITUDE 16000.0f
```

This is below the maximum signed 16-bit PCM value of 32767, providing headroom.

---

# 11. Smooth Fade-In / Fade-Out

The beep does not simply start and stop at full amplitude.

The project applies a sample-by-sample envelope:

```text
        Full volume
       ┌───────────┐
      /             \
     /               \
────/                 \────
   Fade-in           Fade-out
```

Configured fade times:

```text
Fade-in  = 5 ms
Fade-out = 10 ms
```

This reduces abrupt waveform transitions and helps avoid audible clicks caused by sudden changes in amplitude.

---

# 12. Audio Player Initialization

The application initializes the audio system with:

```c
AudioPlayer_Init(
    AUDIO_FREQUENCY_48K,
    70,
    OUTPUT_DEVICE_HEADPHONE
);
```

Therefore:

```text
Sample rate : 48 kHz
Volume      : 70
Output      : Headphone
```

The underlying STM32F4-Discovery BSP initializes the CS43L22 and starts I2S/DMA playback.

The DMA starts with a silent buffer.

---

# 13. Main Application

The relevant application sequence is:

```c
HAL_Init();

SystemClock_Config();

MX_GPIO_Init();
MX_DMA_Init();
MX_I2C1_Init();
MX_I2S3_Init();
MX_SPI1_Init();
MX_USB_HOST_Init();

AudioPlayer_Init(
    AUDIO_FREQUENCY_48K,
    70,
    OUTPUT_DEVICE_HEADPHONE
);
```

The main loop then generates the beep:

```c
AudioPlayer_Beep(1000, 80);
HAL_Delay(700);
```

The LED is also toggled:

```c
HAL_GPIO_TogglePin(GPIOD, LD6_Pin);
HAL_Delay(500);
```

---

# 14. Current Beep Sequence

The current demonstration therefore performs approximately:

```text
1 kHz beep
    │
    ├── 80 ms tone
    │
    ├── 700 ms delay
    │
    ├── LED toggle
    │
    └── 500 ms delay
         │
         ▼
       Repeat
```

The actual tone duration is **80 ms**, as specified by:

```c
AudioPlayer_Beep(1000, 80);
```

---

# 15. Project Structure

Important project files:

```text
F407_CS43L22_AUDIO/
│
├── Core/
│   ├── Inc/
│   │   ├── audio_player.h
│   │   ├── dma.h
│   │   ├── gpio.h
│   │   ├── i2c.h
│   │   ├── i2s.h
│   │   ├── main.h
│   │   └── ...
│   │
│   ├── Src/
│   │   ├── audio_player.c
│   │   ├── dma.c
│   │   ├── gpio.c
│   │   ├── i2c.c
│   │   ├── i2s.c
│   │   ├── main.c
│   │   └── ...
│   │
│   └── Startup/
│
├── Drivers/
│   └── BSP/
│       └── Components/
│           └── cs43l22/
│
├── Middlewares/
│
├── USB_HOST/
│
├── .ioc
├── .project
├── .cproject
└── ...
```

---

# 16. Important Source Files

### `F407_CS43L22_AUDIO.ioc`

Contains the STM32CubeMX hardware configuration, including:

* MCU selection
* GPIO configuration
* clock configuration
* I2C1
* I2S3
* DMA
* peripheral configuration

### `Core/Src/main.c`

Contains the main application and initialization sequence.

### `Core/Src/audio_player.c`

Contains the custom audio player implementation:

* PCM buffer
* sine-wave generation
* fade-in
* fade-out
* beep duration handling
* volume control
* audio initialization
* audio stop

### `Core/Inc/audio_player.h`

Contains the public audio-player interface.

---

# 17. Opening the Project

1. Start **STM32CubeIDE**.
2. Select an appropriate workspace.
3. Choose:

```text
File → Import
```

4. Select:

```text
General → Existing Projects into Workspace
```

5. Select the project directory.
6. Import:

```text
F407_CS43L22_AUDIO
```

7. Build the project.
8. Connect the STM32F407 hardware/debugger.
9. Program the MCU.
10. Connect headphones/audio output to the CS43L22 output.

---

# 18. Expected Result

After programming the STM32:

* The CS43L22 codec is initialized.
* I2S3 begins transmitting audio using DMA.
* A **1 kHz sine-wave beep** is generated.
* The beep is approximately **80 ms** long.
* Audio is sent to the headphone output.
* The sequence repeats continuously.

---

# 19. Conceptual Audio Pipeline

The complete software/hardware pipeline is:

```text
                    STM32F407
                       │
                       │
                Generate sine wave
                       │
                       ▼
                PCM int16 samples
                       │
                       ▼
                 Stereo buffer
                L0 R0 L1 R1 ...
                       │
                       ▼
                    DMA1
                       │
                       ▼
                  I2S3 / SPI3
                       │
          ┌────────────┼────────────┐
          │            │            │
        MCLK         SCLK         LRCK
          │            │            │
          └────────────┼────────────┘
                       │
                     SDIN
                       │
                       ▼
                   CS43L22
                       │
                       ▼
                Headphone output
```

The CS43L22 configuration/control path is separate:

```text
STM32F407
    │
   I2C1
    │
    ├── PB6 → SCL
    └── PB9 → SDA
    │
    ▼
 CS43L22
```

---

# 20. Purpose of This Project

This project is the **first step in a larger STM32 + CS43L22 audio development series**.

The goal is to establish a known-working audio foundation before moving to more advanced applications such as:

* different tone frequencies
* melodies
* alarms
* generated voice/audio
* WAV/PCM playback
* USB audio
* SD-card audio playback
* real-time audio processing
* microphone input
* full-duplex audio

---

## 21. Next Development Steps

Possible future projects can build on this foundation:

```text
01_BEEP
   │
   ├── 02_TONE_GENERATOR
   │
   ├── 03_MELODY
   │
   ├── 04_WAV_PLAYER
   │
   ├── 05_SD_CARD_AUDIO
   │
   └── 06_USB_AUDIO
```

Each project can remain independent under the repository's `Projects/` directory.

---

## 22. Repository Organization

This repository organizes the work as separate STM32 projects.

```text
STM32-CS43L22-Audio/
│
├── Projects/
│   ├── 01_BEEP/
│   ├── 02_...
│   └── 03_...
│
└── README.md
```

The intention is that each directory under `Projects/` represents a **complete, buildable STM32CubeIDE project**, rather than simply a collection of source-code examples.

---


