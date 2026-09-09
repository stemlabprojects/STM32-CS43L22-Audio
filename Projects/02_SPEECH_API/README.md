

````markdown
# STM32 CS43L22 Audio Projects

This repository contains a collection of audio projects developed for the **STM32F407G-DISC1 / STM32F4-Discovery board** using the onboard **CS43L22 audio codec**.

The projects are developed step-by-step, starting from a basic audio output test and progressing toward a reusable speech-generation API.

---

# 1. Repository

GitHub repository:

https://github.com/stemlabprojects/STM32-CS43L22-Audio

Repository purpose:

- Learn STM32 digital audio generation
- Learn I2S audio transmission
- Learn DMA-based continuous audio playback
- Interface STM32F407 with the CS43L22 audio codec
- Generate speech/audio from PCM data
- Convert WAV audio files into C source files
- Build a reusable speech API
- Eventually use sensor values and other embedded data as spoken output

---

# 2. Hardware

## Main Development Board

**STM32F4-Discovery / STM32F407G-DISC1**

Microcontroller:

- STM32F407VGT6
- ARM Cortex-M4
- Floating Point Unit
- 168 MHz maximum CPU frequency

## Audio Codec

**CS43L22**

The STM32F4-Discovery board contains the CS43L22 audio codec.

The codec is used to produce analog audio output through the board's audio output connector.

---

# 3. Audio Architecture

The basic audio path used in these projects is:

```text
PCM Audio Data
      |
      v
STM32F407
      |
      | I2S
      v
CS43L22 Audio Codec
      |
      v
Analog Audio Output
      |
      v
Headphone / Speaker
````

The STM32 generates or stores PCM audio samples and sends them to the CS43L22 using the I2S interface.

DMA is used so that audio data can be transferred continuously without requiring the CPU to manually send every individual sample.

---

# 4. Software Used

The following software/tools were used during development.

## STM32CubeIDE

STM32CubeIDE is the main development environment.

It is used for:

* Creating STM32 projects
* Editing C source files
* Building the firmware
* Debugging
* Flashing the STM32
* Managing the STM32CubeIDE project structure

The project was developed using:

```text
STM32CubeIDE 2.2.0
```

---

## STM32CubeMX

STM32CubeMX functionality is integrated into STM32CubeIDE and is used for configuring the STM32 peripherals.

Important peripherals used by the project include:

* I2S
* DMA
* I2C
* GPIO
* SPI / audio related peripherals depending on project configuration

The `.ioc` file contains the STM32 peripheral configuration.

---

## ARM GNU Toolchain

The project is compiled using the ARM embedded GCC toolchain supplied with STM32CubeIDE.

The compiler used in the current environment is based on:

```text
arm-none-eabi-gcc
GCC 14.3.1
```

The target processor is:

```text
Cortex-M4
```

with:

```text
-mcpu=cortex-m4
-mfpu=fpv4-sp-d16
-mfloat-abi=hard
-mthumb
```

---

## Python

Python is used for preparing audio data for the STM32 project.

A Python utility named:

```text
wav2c.py
```

is used to convert WAV audio files into C source data that can be compiled into the STM32 firmware.

The general purpose is:

```text
WAV file
   |
   v
wav2c.py
   |
   v
C source/header data
   |
   v
STM32 firmware
```

---

# 5. WAV Audio

The speech/audio data used in the project is stored as PCM audio.

The original speech audio used in the current project is based on:

```text
Sample Rate : 8000 Hz
Format      : Unsigned 8-bit PCM
Channels    : Mono
```

The STM32 audio output path uses:

```text
Output Rate : 48000 Hz
Format      : Signed 16-bit PCM
Channels    : Stereo
```

Therefore, the speech data requires conversion before it is sent to the CS43L22.

---

# 6. Audio Rate Conversion

The source speech audio uses:

```text
8000 Hz
```

while the audio output uses:

```text
48000 Hz
```

The ratio is:

```text
48000 / 8000 = 6
```

Therefore, in the current implementation, every 8-kHz source sample is repeated six times to obtain a 48-kHz output stream.

Conceptually:

```text
8 kHz source:

S0 S1 S2 S3 S4 S5 ...

48 kHz output:

S0 S0 S0 S0 S0 S0
S1 S1 S1 S1 S1 S1
S2 S2 S2 S2 S2 S2
...
```

This is a simple sample-rate conversion method.

It is intentionally kept simple for the embedded project.

---

# 7. 8-bit to 16-bit Conversion

The source speech samples are unsigned 8-bit PCM.

The audio output path uses signed 16-bit PCM.

The conversion therefore requires:

```text
Unsigned 8-bit PCM
        |
        v
Center around zero
        |
        v
Signed 16-bit PCM
```

An 8-bit unsigned PCM sample has a nominal center around:

```text
128
```

Therefore the sample is first shifted around zero and then scaled to the 16-bit audio range.

Conceptually:

```text
signed_sample = (unsigned_sample - 128) << 8
```

The exact implementation is contained in the speech/audio source code.

---

# 8. Mono to Stereo Conversion

The speech source is mono.

The CS43L22 playback path is configured for stereo audio.

Therefore the same audio sample is sent to both channels:

```text
Left  = sample
Right = sample
```

Conceptually:

```text
Mono:

S0 S1 S2 S3 ...

Stereo:

L0 R0  L1 R1  L2 R2  L3 R3
S0 S0  S1 S1  S2 S2  S3 S3
```

This produces the same speech signal on the left and right outputs.

---

# 9. DMA Audio Playback

DMA is used for continuous audio playback.

Instead of the CPU manually transmitting every audio sample, DMA transfers the audio buffer to the audio peripheral.

The playback buffer is divided into sections.

Typical flow:

```text
+-------------------------------+
|        Audio Buffer            |
+---------------+---------------+
| First Half    | Second Half   |
+---------------+---------------+
        ^                ^
        |                |
   Half Transfer    Transfer Complete
       callback          callback
```

The callbacks are used to refill or update the audio buffer while playback continues.

This allows continuous audio playback without stopping the I2S stream.

---

# 10. Project 01 - BEEP

Location:

```text
Projects/01_BEEP
```

The first project is a basic audio output project.

Purpose:

* Verify the STM32F407 audio hardware
* Verify CS43L22 communication
* Verify I2S operation
* Verify DMA operation
* Generate a basic beep/audio signal
* Establish the basic audio playback architecture

This project is intentionally simple and acts as the starting point for the later speech project.

---

# 11. Project 02 - SPEECH_API

Location:

```text
Projects/02_SPEECH_API
```

The second project extends the audio architecture into a reusable speech system.

The purpose is to allow the application to request speech using an API rather than manually handling individual audio files.

Main source files include:

```text
Core/Inc/speech_engine.h
Core/Src/speech_engine.c

Core/Inc/speech_data.h
Core/Src/speech_data.c

Core/Inc/audio_player.h
Core/Src/audio_player.c
```

---

# 12. Speech Engine

The speech engine provides the higher-level interface for playing speech terms.

The application does not need to directly manage:

* PCM sample arrays
* Sample-rate conversion
* Mono-to-stereo conversion
* Audio buffer management
* DMA callbacks
* Speech queue handling

Those operations are handled by the speech/audio layer.

The application can therefore work at a higher level.

Conceptually:

```text
Application
    |
    v
Speech API
    |
    v
Speech Engine
    |
    v
Audio Player
    |
    v
DMA / I2S
    |
    v
CS43L22
```

---

# 13. Speech Terms

The speech system uses a `SpeechTerm_t` enumeration.

Current terms are:

```c
typedef enum
{
    SPEECH_TERM_POINT = 0,
    SPEECH_TERM_TEMPERATURE,
    SPEECH_TERM_HUMIDITY,
    SPEECH_TERM_DEGREE,
    SPEECH_TERM_CELSIUS,
    SPEECH_TERM_PERCENT,
    SPEECH_TERM_IS,
    SPEECH_TERM_MINUS,
    SPEECH_TERM_COUNT

} SpeechTerm_t;
```

These terms are intended to represent words or speech components that can be combined to create spoken sensor information.

Examples:

```text
Temperature
Humidity
Degree
Celsius
Percent
Minus
Point
```

---

# 14. Speech API Concept

The goal of the speech API is to allow the application to provide a sensor value rather than manually selecting individual audio files.

For example:

```c
Speech_Speak(SPEECH_TERM_TEMPERATURE, 23.5);
```

The intended concept is:

```text
Sensor value
     |
     v
Speech API
     |
     +---- "temperature"
     |
     +---- "twenty"
     |
     +---- "three"
     |
     +---- "point"
     |
     +---- "five"
     |
     +---- "degree"
     |
     +---- "celsius"
     |
     v
Audio playback
```

The API can eventually be extended for:

* Temperature
* Humidity
* Pressure
* Wind speed
* Rainfall
* Voltage
* Current
* Battery level
* Other sensor measurements

---

# 15. Decimal Values

The speech system is intended to support real sensor values.

For temperature, one decimal place may be useful.

Example:

```text
23.5 °C
```

could be spoken as:

```text
Temperature
twenty three
point
five
degree
Celsius
```

For values that do not require a decimal component, the point portion can be omitted.

For example:

```text
Humidity = 60 %
```

could be:

```text
Humidity
sixty
percent
```

---

# 16. Negative Values

The speech system also contains:

```text
SPEECH_TERM_MINUS
```

This allows negative sensor values to be represented.

Example:

```text
Temperature = -5.2 °C
```

can conceptually become:

```text
Temperature
minus
five
point
two
degree
Celsius
```

---

# 17. Speech Data

Speech data is stored in:

```text
Core/Inc/speech_data.h
Core/Src/speech_data.c
```

The audio samples are compiled into the firmware as C data.

This avoids requiring a filesystem or SD card during playback.

The basic architecture is:

```text
Audio file
    |
    v
wav2c.py
    |
    v
speech_data.c / speech_data.h
    |
    v
STM32 firmware
```

---

# 18. Why WAV Files Are Converted to C

The STM32F407 does not need to access a WAV file directly during playback.

Instead, the required PCM samples are compiled into the firmware.

Advantages:

* No filesystem required
* No SD card required
* No USB storage required
* Predictable playback
* Fast access to audio data
* Easy integration with the speech engine

The trade-off is that audio data consumes Flash memory.

---

# 19. `wav2c.py`

`wav2c.py` is a Python utility used to convert WAV files into C-compatible audio arrays.

Its purpose is to automate the conversion process.

General workflow:

```text
speech.wav
    |
    v
wav2c.py
    |
    v
speech_data.c
speech_data.h
```

The generated C data can then be included in the STM32 project.

When adding new speech words:

1. Prepare the WAV file.
2. Ensure the audio format is compatible with the project.
3. Run `wav2c.py`.
4. Generate the C audio data.
5. Add/update the generated files.
6. Build the STM32 project.
7. Flash the board.
8. Test the new speech term.

---

# 20. Audio Buffer

The speech engine uses an audio buffer to feed the DMA/I2S playback system.

The current architecture uses:

```text
SPEECH_BUFFER_FRAMES
```

to define the playback buffer size.

The audio buffer is processed in halves.

This allows one portion of the buffer to be transmitted while the other portion is prepared.

---

# 21. Speech Queue

The speech engine supports a queue of speech terms.

The queue allows multiple words to be scheduled for playback.

For example:

```text
Temperature
23
Point
5
Degree
Celsius
```

can be queued as multiple speech items.

The audio engine then plays them sequentially.

Conceptually:

```text
Queue:

[Temperature]
      |
      v
[Twenty]
      |
      v
[Three]
      |
      v
[Point]
      |
      v
[Five]
      |
      v
[Degree]
      |
      v
[Celsius]
```

---

# 22. Silence Between Speech Terms

A short silence can be inserted between speech terms.

The speech engine contains a configurable gap:

```c
#define SPEECH_GAP_MS
```

This is used to separate words.

The purpose is to prevent two adjacent audio samples from sounding continuously connected.

The gap can be adjusted depending on the quality of the generated speech.

---

# 23. Important Audio Consideration

The quality of the final voice depends on the original speech audio.

For example, if two different speech-generation sources produce different WAV files, but the embedded conversion/playback process reduces them to the same effective characteristics, the output may sound more similar than expected.

The important factors include:

* Original speech engine
* Original sample rate
* Original bit depth
* PCM encoding
* Sample-rate conversion
* Amplitude scaling
* Mono/stereo conversion
* Audio buffer handling
* Playback timing

Therefore, when comparing different speech sources, it is important to compare the original WAV files and the converted PCM data separately.

---

# 24. Current Audio Conversion Architecture

The current architecture is approximately:

```text
          WAV Speech File
                 |
                 |
        8000 Hz / 8-bit
                 |
                 v
            wav2c.py
                 |
                 v
          C PCM Audio Data
                 |
                 v
       Speech Data / Engine
                 |
                 v
      8-bit unsigned PCM
                 |
                 | sample conversion
                 v
       16-bit signed PCM
                 |
                 | 8 kHz -> 48 kHz
                 v
        48 kHz PCM Stream
                 |
                 | mono -> stereo
                 v
          Stereo PCM
                 |
                 v
               DMA
                 |
                 v
               I2S
                 |
                 v
             CS43L22
                 |
                 v
          Analog Audio
```

---

# 25. Future 16-bit Audio Experiment

The current speech source uses:

```text
8000 Hz
8-bit
```

A future experiment can use:

```text
8000 Hz
16-bit
```

This may improve the preservation of the original audio waveform because the source contains substantially more amplitude resolution.

The architecture would then change from:

```text
8-bit unsigned PCM
```

to:

```text
16-bit PCM
```

The conversion code would need to be modified accordingly.

The important point is that the source audio format and the STM32 playback format must be clearly separated.

---

# 26. STM32 Peripheral Architecture

The project uses STM32 peripherals and drivers generated/configured through STM32CubeIDE/CubeMX.

Important files include:

```text
Core/Src/i2s.c
Core/Src/i2c.c
Core/Src/dma.c
Core/Src/gpio.c
```

and corresponding headers:

```text
Core/Inc/i2s.h
Core/Inc/i2c.h
Core/Inc/dma.h
Core/Inc/gpio.h
```

The generated STM32 HAL drivers are located under:

```text
Drivers/STM32F4xx_HAL_Driver
```

CMSIS files are located under:

```text
Drivers/CMSIS
```

---

# 27. STM32F4-Discovery BSP

The STM32F4-Discovery board support files are used for the CS43L22 audio codec.

The project uses the STM32F4-Discovery audio BSP.

The relevant include path contains:

```text
Drivers/BSP/STM32F4-Discovery
```

The BSP provides the interface required to initialize and control the board's audio codec.

---

# 28. Project Structure

The repository is organized as:

```text
STM32-CS43L22-Audio/
|
+-- Projects/
|   |
|   +-- 01_BEEP/
|   |
|   +-- 02_SPEECH_API/
|
+-- README.md
```

Each project is kept as an independent STM32CubeIDE project.

---

# 29. Project 02 Structure

The speech project contains approximately:

```text
02_SPEECH_API/
|
+-- Core/
|   |
|   +-- Inc/
|   |   +-- audio_player.h
|   |   +-- dma.h
|   |   +-- gpio.h
|   |   +-- i2c.h
|   |   +-- i2s.h
|   |   +-- main.h
|   |   +-- speech_data.h
|   |   +-- speech_engine.h
|   |   +-- spi.h
|   |
|   +-- Src/
|       +-- audio_player.c
|       +-- dma.c
|       +-- gpio.c
|       +-- i2c.c
|       +-- i2s.c
|       +-- main.c
|       +-- speech_data.c
|       +-- speech_engine.c
|       +-- spi.c
|       +-- stm32f4xx_hal_msp.c
|       +-- stm32f4xx_it.c
|       +-- system_stm32f4xx.c
|
+-- Drivers/
|
+-- Middlewares/
|
+-- USB_HOST/
|
+-- F407_CS43L22_AUDIO.ioc
|
+-- F407_CS43L22_AUDIO Debug.launch
|
+-- STM32F407VGTX_FLASH.ld
|
+-- STM32F407VGTX_RAM.ld
```

Build output directories such as `Debug/` are not intended to be committed to the Git repository.

---

# 30. Git Repository

The source projects are maintained using Git.

Remote repository:

```text
https://github.com/stemlabprojects/STM32-CS43L22-Audio.git
```

The repository contains the projects under:

```text
Projects/
```

---

# 31. Git Commands Used

## Check repository status

```bash
git status
```

or:

```bash
git status --short
```

---

## Check remote repository

```bash
git remote -v
```

---

## Check commit history

```bash
git log --oneline -3
```

---

## Add Project 02

From the repository root:

```bash
git add Projects/02_SPEECH_API
```

---

## Commit Project 02

```bash
git commit -m "Add Project 02 speech API"
```

---

## Push changes

```bash
git push origin main
```

---

# 32. Handling Remote Changes

If Git reports:

```text
! [rejected] main -> main (fetch first)
```

the remote repository contains commits that are not present locally.

The remote changes should be integrated before pushing.

A typical procedure is:

```bash
git pull origin main
```

If Git opens the merge commit message editor, save the message and exit.

For the Nano editor:

```text
Ctrl + O
Enter
Ctrl + X
```

Then check:

```bash
git status
```

Finally:

```bash
git push origin main
```

---

# 33. Checking the Final Repository

After pushing, verify:

```bash
git status
```

The expected result should be similar to:

```text
On branch main
Your branch is up to date with 'origin/main'.

nothing to commit, working tree clean
```

The repository can then be checked online.

---

# 34. Building the Project

Open STM32CubeIDE.

Import the project:

```text
Projects/02_SPEECH_API
```

The `.project` and `.cproject` files are included for STM32CubeIDE.

After importing:

1. Select the project.
2. Clean the project if necessary.
3. Build the project.
4. Check the Console for errors.
5. Connect the STM32F4-Discovery board.
6. Flash the firmware.
7. Test the audio output.

---

# 35. Clean Build

If an incremental build produces unexpected results, perform:

```text
Project
    |
    +-- Clean
```

and then:

```text
Project
    |
    +-- Build Project
```

This removes old object files and rebuilds the project.

---

# 36. Important Files

The most important application-level files are:

```text
speech_engine.h
speech_engine.c
speech_data.h
speech_data.c
audio_player.h
audio_player.c
main.c
```

The STM32 configuration is stored in:

```text
F407_CS43L22_AUDIO.ioc
```

The linker configuration is stored in:

```text
STM32F407VGTX_FLASH.ld
STM32F407VGTX_RAM.ld
```

---

# 37. Rebuilding the Project From Scratch

To recreate the project on another computer:

## Step 1 - Install STM32CubeIDE

Install STM32CubeIDE.

Make sure the ARM GNU toolchain is installed with it.

---

## Step 2 - Clone the repository

```bash
git clone https://github.com/stemlabprojects/STM32-CS43L22-Audio.git
```

Enter the repository:

```bash
cd STM32-CS43L22-Audio
```

---

## Step 3 - Open Project 02

In STM32CubeIDE, import:

```text
Projects/02_SPEECH_API
```

---

## Step 4 - Verify the STM32 configuration

Open:

```text
F407_CS43L22_AUDIO.ioc
```

Verify the required peripherals and clock configuration.

---

## Step 5 - Build

Build the project in STM32CubeIDE.

---

## Step 6 - Program the board

Connect the STM32F4-Discovery board and flash the firmware.

---

## Step 7 - Test audio

Connect headphones or the appropriate audio output and verify that the speech output is working.

---

# 38. Adding a New Speech Word

To add another word:

Example:

```text
"pressure"
```

The workflow is:

```text
Create/obtain WAV
       |
       v
Check WAV format
       |
       v
Run wav2c.py
       |
       v
Generate C audio data
       |
       v
Add data to speech_data
       |
       v
Add new speech term
       |
       v
Update speech engine
       |
       v
Build
       |
       v
Flash
       |
       v
Test
```

---

# 39. Sensor Application Example

The final purpose of this speech system is to make sensor projects capable of speaking their measurements.

For example:

```text
Temperature Sensor
       |
       v
    23.5 °C
       |
       v
Speech API
       |
       v
"Temperature twenty three point five degree Celsius"
```

Similarly:

```text
Humidity Sensor
       |
       v
      62 %
       |
       v
Speech API
       |
       v
"Humidity sixty two percent"
```

This makes the audio engine reusable across different embedded projects.

---

# 40. Design Philosophy

The project is intentionally divided into layers.

```text
Application Layer
       |
       v
Speech API
       |
       v
Speech Engine
       |
       v
Audio Player
       |
       v
STM32 HAL / BSP
       |
       v
I2S + DMA
       |
       v
CS43L22
```

This separation is important because the application should not need to know the low-level audio implementation.

For example, the application should eventually be able to do something similar to:

```c
Speech_Speak(SPEECH_TERM_TEMPERATURE, temperature);
```

without knowing:

* Where the PCM samples are stored
* How samples are converted
* How the DMA buffer works
* How I2S works
* How the CS43L22 is configured

---

# 41. Current Project Status

## Project 01 - BEEP

Status:

```text
Working
```

Purpose:

```text
Basic CS43L22 audio output verification
```

---

## Project 02 - SPEECH_API

Status:

```text
Working
```

Current functionality includes:

* PCM speech playback
* 8-kHz source audio
* 48-kHz output
* 8-bit source sample conversion
* 16-bit output samples
* Mono-to-stereo playback
* DMA-based audio playback
* Speech term handling
* Speech queue
* Silence/gap between speech items
* Speech API development
* Sensor-value speech concept

---

# 42. Known Limitations

The current audio conversion uses a simple 8-kHz to 48-kHz sample repetition method.

This is not a high-quality sample-rate conversion algorithm.

For higher-quality audio, future versions may use:

* Proper interpolation
* FIR filtering
* Better sample-rate conversion
* Higher source sample rate
* 16-bit source PCM
* Improved speech-generation sources

The current implementation is primarily intended for learning and embedded-system experimentation.

---

# 43. Future Development

Possible future projects include:

```text
03_SPEECH_SENSOR
```

with direct sensor integration.

Potential sensors:

* Temperature
* Humidity
* Pressure
* Light
* Wind speed
* Rain
* Air quality
* Battery voltage

Potential spoken output:

```text
Temperature twenty three point five degree Celsius.

Humidity sixty two percent.

Pressure one thousand thirteen point two hPa.
```

---

# 44. Future Audio Improvements

Possible future improvements:

1. Move from 8-bit PCM to 16-bit PCM.
2. Test 16-kHz speech.
3. Test higher-quality speech sources.
4. Implement proper sample-rate conversion.
5. Reduce memory usage.
6. Improve speech queue handling.
7. Add asynchronous speech requests.
8. Add priority speech messages.
9. Add sensor-specific APIs.
10. Add support for larger vocabulary.
11. Store speech data externally if Flash becomes insufficient.

---

# 45. Development Environment Used

Current development environment:

```text
Operating System:
Linux

IDE:
STM32CubeIDE 2.2.0

Compiler:
arm-none-eabi-gcc 14.3.1

Target:
STM32F407VGT6

Board:
STM32F4-Discovery / STM32F407G-DISC1

Audio Codec:
CS43L22

Source Audio:
8 kHz / 8-bit / Mono PCM

Output Audio:
48 kHz / 16-bit / Stereo PCM

Audio Interface:
I2S

Data Transfer:
DMA

Audio Conversion Utility:
Python wav2c.py

Version Control:
Git

Repository:
STM32-CS43L22-Audio
```

---

# 46. Important Reproduction Notes

When rebuilding this project in the future, do not change the audio architecture without checking all of the following together:

```text
Source sample rate
Source bit depth
Source encoding
Source channel count
Output sample rate
Output bit depth
Output channel count
I2S configuration
DMA configuration
Audio buffer size
Speech queue logic
Audio callbacks
CS43L22 configuration
```

A change in one part of the audio pipeline can affect the complete playback system.

For example, changing:

```text
8-bit -> 16-bit
```

requires corresponding changes in the PCM conversion and buffer handling.

Similarly:

```text
8000 Hz -> 16000 Hz
```

requires corresponding changes in the sample-rate conversion logic.

---

# 47. Summary

This repository documents the development of an STM32-based audio and speech system using the CS43L22 codec.

The development path is:

```text
Project 01
Basic audio / beep
        |
        v
I2S + DMA
        |
        v
CS43L22 verification
        |
        v
Project 02
Speech playback
        |
        v
WAV -> C conversion
        |
        v
Speech engine
        |
        v
Speech API
        |
        v
Sensor value speech
        |
        v
Future complete voice-enabled
embedded sensor system
```

The main objective is not only to produce sound, but to build a reusable embedded audio architecture that can eventually convert real sensor measurements into spoken information.

---

# 48. Repository

GitHub:

[https://github.com/stemlabprojects/STM32-CS43L22-Audio](https://github.com/stemlabprojects/STM32-CS43L22-Audio)

Projects:

```text
Projects/01_BEEP
Projects/02_SPEECH_API
```

---

# 49. License / Project Note

This project is maintained as part of the STEM Lab Projects collection for learning, experimentation, embedded-system development, and educational use.

The project contains STM32-generated files, STM32 HAL/BSP components, and project-specific application code.

Refer to the respective source files and STMicroelectronics licensing information for the applicable STM32 software components.

````

This is the **complete single `README.md` content**. You can select from the first `# STM32 CS43L22 Audio Projects` line all the way to the final line and paste it directly into:

```text
Projects/README.md
````

or, preferably for the GitHub repository's main page:

```text
README.md
```

at the repository root.
