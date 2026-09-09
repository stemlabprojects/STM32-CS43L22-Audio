/**
  ******************************************************************************
  * @file    audio_player.h
  * @brief   Simple application-level API on top of the STM32F4-Discovery
  *          audio BSP (stm32f4_discovery_audio.c + cs43l22.c) for generating
  *          beep tones. main.c should only ever call these functions.
  ******************************************************************************
  */

#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4_discovery_audio.h"   /* BSP_AUDIO_OUT_* + OUTPUT_DEVICE_*, AUDIO_FREQUENCY_* */

/**
  * @brief  Initializes the codec + I2S output for beep playback.
  * @note   Call this once after HAL_Init()/SystemClock_Config()/MX_xxx_Init()
  *         (I2S3, I2C1 and their GPIOs must already be configured by CubeMX).
  * @param  audioFreq: e.g. AUDIO_FREQUENCY_48K, AUDIO_FREQUENCY_44K, ...
  * @param  volume:    0 (mute) .. 100 (max)
  * @param  outputDevice: OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
  *                        OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO
  * @retval 0 if the codec initialized correctly, non-zero on error
  */
uint8_t AudioPlayer_Init(uint32_t audioFreq, uint8_t volume, uint16_t outputDevice);

/**
  * @brief  Plays a single-tone beep for a fixed duration with a short
  *         fade-in/fade-out, then returns to silence. Does NOT power down
  *         the codec or restart the DMA - that's what avoids clicks/pops
  *         between beeps.
  * @note   This call blocks for roughly 'duration_ms' (uses HAL_Delay
  *         internally). Requires the I2S3 TX DMA stream to be configured in
  *         CIRCULAR mode in CubeMX, since the fixed-length buffer set up
  *         in AudioPlayer_Init() loops continuously for the whole runtime.
  * @param  frequency_hz: tone frequency, clamped internally to a safe range
  * @param  duration_ms:  how long the tone should sound, in milliseconds
  *                        (including the fade in/out time)
  */
void AudioPlayer_Beep(uint32_t frequency_hz, uint32_t duration_ms);

/**
  * @brief  Fully stops playback and powers the codec down (click expected).
  *         Only call this when you're really done beeping, e.g. before a
  *         low-power mode - AudioPlayer_Init() must be called again
  *         afterwards before any further beeps.
  */
void AudioPlayer_Stop(void);

/**
  * @brief  Changes the output volume.
  * @param  volume: 0 (mute) .. 100 (max)
  */
void AudioPlayer_SetVolume(uint8_t volume);


void AudioPlayer_VowelA(uint32_t duration_ms);


#ifdef __cplusplus
}
#endif

#endif /* AUDIO_PLAYER_H */
