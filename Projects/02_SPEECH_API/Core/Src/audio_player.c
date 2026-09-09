
/**
  ******************************************************************************
  * @file    audio_player.c
  * @brief   Beep-tone player built on top of the STM32F4-Discovery audio BSP.
  *
  *          The CS43L22 codec and I2S/DMA are controlled through the
  *          STM32F4-Discovery BSP.
  *
  *          A complete beep waveform is generated in the DMA buffer with
  *          sample-by-sample fade-in and fade-out. This avoids the audible
  *          stepping caused by changing the amplitude every few milliseconds.
  ******************************************************************************
  */

#include "audio_player.h"
#include <math.h>

/* Private defines -----------------------------------------------------------*/

/*
 * DMA buffer:
 *
 * 4800 samples per channel at 48 kHz = 100 ms.
 *
 * Stereo interleaved:
 * L, R, L, R, ...
 */
#define AUDIO_PLAYER_BUFFER_SAMPLES   9600U

/*
 * Maximum safe amplitude.
 * 16000 is below the int16_t maximum of 32767,
 * leaving some headroom.
 */
#define AUDIO_PLAYER_AMPLITUDE        16000.0f

#define AUDIO_PLAYER_PI               3.14159265f

/*
 * Fade times.
 *
 * These are implemented SAMPLE-BY-SAMPLE,
 * not as discrete volume steps.
 */
#define AUDIO_PLAYER_FADE_IN_MS       5U
#define AUDIO_PLAYER_FADE_OUT_MS      10U


/* Private variables --------------------------------------------------------*/

/*
 * Stereo interleaved PCM buffer.
 *
 * [L0, R0, L1, R1, L2, R2, ...]
 */
static int16_t AudioPlayer_Buffer[AUDIO_PLAYER_BUFFER_SAMPLES * 2U];

/* Current sample rate */
static uint32_t AudioPlayer_SampleRate = AUDIO_FREQUENCY_48K;

/* Indicates that BSP audio has been initialized and DMA started */
static uint8_t AudioPlayer_Started = 0U;


/* Private functions --------------------------------------------------------*/

/**
  * @brief  Fill the complete DMA buffer with silence.
  */
static void AudioPlayer_FillSilence(void)
{
    uint32_t i;

    for (i = 0U;
         i < (AUDIO_PLAYER_BUFFER_SAMPLES * 2U);
         i++)
    {
        AudioPlayer_Buffer[i] = 0;
    }
}


/**
  * @brief  Generate a complete sine-wave tone with smooth envelope.
  *
  *         The envelope is calculated for every sample:
  *
  *         fade-in -> full volume -> fade-out
  *
  *         This avoids the audible "stepping" caused by changing the
  *         complete buffer every few milliseconds.
  *
  * @param  frequency_hz: requested tone frequency
  * @param  duration_ms:  requested tone duration
  */
static void AudioPlayer_FillTone(uint32_t frequency_hz,
                                 uint32_t duration_ms)
{
    uint32_t totalSamples;
    uint32_t fadeInSamples;
    uint32_t fadeOutSamples;
    uint32_t i;

    /*
     * Convert requested duration to number of audio samples.
     */
    totalSamples =
        (uint32_t)(((uint64_t)AudioPlayer_SampleRate *
                    (uint64_t)duration_ms) / 1000ULL);

    /*
     * The current DMA buffer can contain only this many samples.
     */
    if (totalSamples > AUDIO_PLAYER_BUFFER_SAMPLES)
    {
        totalSamples = AUDIO_PLAYER_BUFFER_SAMPLES;
    }

    /*
     * Very short beep protection.
     */
    if (totalSamples < 4U)
    {
        AudioPlayer_FillSilence();
        return;
    }

    /*
     * Convert fade durations to samples.
     */
    fadeInSamples =
        (uint32_t)(((uint64_t)AudioPlayer_SampleRate *
                    (uint64_t)AUDIO_PLAYER_FADE_IN_MS) / 1000ULL);

    fadeOutSamples =
        (uint32_t)(((uint64_t)AudioPlayer_SampleRate *
                    (uint64_t)AUDIO_PLAYER_FADE_OUT_MS) / 1000ULL);

    /*
     * Prevent the two fades from being larger than the complete tone.
     */
    if ((fadeInSamples + fadeOutSamples) >= totalSamples)
    {
        fadeInSamples = totalSamples / 4U;
        fadeOutSamples = totalSamples / 4U;
    }

    /*
     * Generate the waveform.
     */
    for (i = 0U; i < totalSamples; i++)
    {
        float envelope = 1.0f;
        float phase;
        float sampleValue;
        int16_t sample;

        /*
         * -------------------------
         * Fade IN
         * -------------------------
         */
        if ((fadeInSamples > 0U) &&
            (i < fadeInSamples))
        {
            envelope = (float)i / (float)fadeInSamples;
        }

        /*
         * -------------------------
         * Fade OUT
         * -------------------------
         */
        if ((fadeOutSamples > 0U) &&
            (i >= (totalSamples - fadeOutSamples)))
        {
            uint32_t remaining =
                totalSamples - i - 1U;

            envelope = (float)remaining /
                       (float)fadeOutSamples;
        }

        /*
         * Keep envelope inside 0.0 ... 1.0.
         */
        if (envelope < 0.0f)
        {
            envelope = 0.0f;
        }

        if (envelope > 1.0f)
        {
            envelope = 1.0f;
        }

        /*
         * Generate sine-wave phase.
         *
         * We use the requested frequency directly rather than forcing
         * an integer number of cycles into the buffer.
         */
        phase =
            2.0f * AUDIO_PLAYER_PI *
            (float)frequency_hz *
            (float)i /
            (float)AudioPlayer_SampleRate;

        sampleValue =
            AUDIO_PLAYER_AMPLITUDE *
            envelope *
            sinf(phase);

        sample = (int16_t)sampleValue;

        /*
         * Stereo output.
         */
        AudioPlayer_Buffer[2U * i]       = sample;
        AudioPlayer_Buffer[2U * i + 1U] = sample;
    }

    /*
     * Fill everything after the requested tone with silence.
     */
    for (; i < AUDIO_PLAYER_BUFFER_SAMPLES; i++)
    {
        AudioPlayer_Buffer[2U * i]       = 0;
        AudioPlayer_Buffer[2U * i + 1U] = 0;
    }

    /*
     * Force the very end of the tone to zero.
     *
     * This guarantees that the transition to silence is not an abrupt
     * non-zero PCM value.
     */
    AudioPlayer_Buffer[2U * (totalSamples - 1U)]       = 0;
    AudioPlayer_Buffer[2U * (totalSamples - 1U) + 1U] = 0;
}


/* Public functions ---------------------------------------------------------*/

/**
  * @brief  Initialize the audio player.
  */
uint8_t AudioPlayer_Init(uint32_t audioFreq,
                         uint8_t volume,
                         uint16_t outputDevice)
{
    uint8_t status;

    AudioPlayer_SampleRate = audioFreq;

    /*
     * Initialize CS43L22 + I2S audio through the STM32F4-Discovery BSP.
     */
    status = BSP_AUDIO_OUT_Init(outputDevice,
                                volume,
                                audioFreq);

    if (status == 0U)
    {
        /*
         * Start DMA with silence.
         *
         * DMA remains running continuously.
         */
        AudioPlayer_FillSilence();

        BSP_AUDIO_OUT_Play(
            (uint16_t *)AudioPlayer_Buffer,
            AUDIO_PLAYER_BUFFER_SAMPLES * 2U *
            (uint32_t)sizeof(int16_t)
        );

        AudioPlayer_Started = 1U;
    }

    return status;
}


/**
  * @brief  Play a beep with smooth sample-by-sample fade-in/out.
  */
void AudioPlayer_Beep(uint32_t frequency_hz,
                      uint32_t duration_ms)
{
    if (AudioPlayer_Started == 0U)
    {
        return;
    }

    /*
     * Generate the complete waveform with smooth envelope.
     */
    AudioPlayer_FillTone(frequency_hz,
                         duration_ms);

    /*
     * Keep the buffer playing for the requested duration.
     *
     * Since the DMA buffer is 100 ms, durations greater than 100 ms
     * cannot be represented correctly by this single-buffer method.
     */
    if (duration_ms <= 100U)
    {
        HAL_Delay(duration_ms);
    }
    else
    {
        HAL_Delay(100U);
    }

    /*
     * Return to silence.
     */
    AudioPlayer_FillSilence();
}


/**
  * @brief  Stop audio playback and power down the codec.
  */
void AudioPlayer_Stop(void)
{
    /*
     * Full codec power-down.
     *
     * This may produce a click/pop and requires AudioPlayer_Init()
     * before playing another beep.
     */
    BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);

    AudioPlayer_Started = 0U;
}


/**
  * @brief  Change output volume.
  */
void AudioPlayer_SetVolume(uint8_t volume)
{
    BSP_AUDIO_OUT_SetVolume(volume);
}

void AudioPlayer_VowelA(uint32_t duration_ms)
{
    uint32_t total_ms = duration_ms;
    uint32_t elapsed_ms = 0U;

    if (AudioPlayer_Started == 0U)
    {
        return;
    }

    /*
     * One DMA buffer = 100 ms.
     * Generate the vowel in 100 ms chunks and repeat it.
     */
    while (elapsed_ms < total_ms)
    {
        uint32_t chunk_ms = total_ms - elapsed_ms;

        if (chunk_ms > 100U)
        {
            chunk_ms = 100U;
        }

        uint32_t totalSamples =
            (uint32_t)(((uint64_t)AudioPlayer_SampleRate *
                        (uint64_t)chunk_ms) / 1000ULL);

        uint32_t i;

        /*
         * Generate synthetic vowel "A".
         */
        for (i = 0U; i < totalSamples; i++)
        {
            float t =
                (float)i /
                (float)AudioPlayer_SampleRate;

            /*
             * Fundamental + harmonics.
             *
             * Fundamental = 140 Hz
             */
            float sampleValue =
                  1.00f * sinf(2.0f * AUDIO_PLAYER_PI * 140.0f * t)
                + 0.50f * sinf(2.0f * AUDIO_PLAYER_PI * 280.0f * t)
                + 0.30f * sinf(2.0f * AUDIO_PLAYER_PI * 420.0f * t)
                + 0.20f * sinf(2.0f * AUDIO_PLAYER_PI * 560.0f * t)
                + 0.10f * sinf(2.0f * AUDIO_PLAYER_PI * 700.0f * t);

            sampleValue *= 0.35f;

            int16_t sample =
                (int16_t)(sampleValue *
                          AUDIO_PLAYER_AMPLITUDE);

            AudioPlayer_Buffer[2U * i]       = sample;
            AudioPlayer_Buffer[2U * i + 1U] = sample;
        }

        /*
         * Remaining part of buffer = silence.
         */
        for (; i < AUDIO_PLAYER_BUFFER_SAMPLES; i++)
        {
            AudioPlayer_Buffer[2U * i]       = 0;
            AudioPlayer_Buffer[2U * i + 1U] = 0;
        }

        /*
         * Play this chunk.
         */
        HAL_Delay(chunk_ms);

        elapsed_ms += chunk_ms;
    }

    /*
     * Return to silence.
     */
    AudioPlayer_FillSilence();
}



