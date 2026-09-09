#include "speech_engine.h"
#include "speech_data.h"
#include "stm32f4_discovery_audio.h"

/*
 * Speech audio:
 *   Source  : unsigned 8-bit PCM, 8000 Hz, mono
 *   Output  : signed 16-bit PCM, 48000 Hz, stereo
 *
 * Each 8-kHz source sample is repeated 6 times.
 */

#define SPEECH_OUTPUT_RATE          48000U
#define SPEECH_SOURCE_RATE           8000U
#define SPEECH_RESAMPLE_FACTOR          6U

/* Silence between two speech items. */
#define SPEECH_GAP_MS                500U
#define SPEECH_GAP_FRAMES \
    ((SPEECH_OUTPUT_RATE * SPEECH_GAP_MS) / 1000U)

#define SPEECH_BUFFER_FRAMES         480U
#define SPEECH_QUEUE_SIZE             16U

#define SPEECH_ITEM_WORD               0U
#define SPEECH_ITEM_TERM               1U

typedef enum
{
    SPEECH_STATE_STOPPED = 0U,
    SPEECH_STATE_PLAYING,
    SPEECH_STATE_GAP
} SpeechState_t;

typedef struct
{
    uint8_t type;
    uint16_t id;
} SpeechQueueItem_t;

static int16_t speech_buffer[SPEECH_BUFFER_FRAMES * 2U];

static SpeechQueueItem_t speech_queue[SPEECH_QUEUE_SIZE];
static volatile uint8_t speech_queue_head = 0U;
static volatile uint8_t speech_queue_tail = 0U;
static volatile uint8_t speech_queue_count = 0U;

static const uint8_t *speech_data = 0;
static uint32_t speech_length = 0U;
static volatile uint32_t speech_position = 0U;
static volatile uint32_t speech_gap_remaining = 0U;
static volatile SpeechState_t speech_state = SPEECH_STATE_STOPPED;

static uint8_t Speech_LoadNextItem(void);

/* --------------------------------------------------------- */
/* Convert unsigned 8-bit PCM to signed 16-bit audio        */
/* --------------------------------------------------------- */

static int16_t Speech_ConvertSample(uint8_t sample)
{
    return (int16_t)(((int32_t)sample - 128) << 8);
}

/* --------------------------------------------------------- */
/* Select word audio                                         */
/* --------------------------------------------------------- */

static uint8_t Speech_SelectWord(SpeechWord_t word)
{
    switch (word)
    {
        case SPEECH_WORD_ZERO:
            speech_data = word_zero;
            speech_length = word_zero_len;
            break;

        case SPEECH_WORD_ONE:
            speech_data = word_one;
            speech_length = word_one_len;
            break;

        case SPEECH_WORD_TWO:
            speech_data = word_two;
            speech_length = word_two_len;
            break;

        case SPEECH_WORD_THREE:
            speech_data = word_three;
            speech_length = word_three_len;
            break;

        case SPEECH_WORD_FOUR:
            speech_data = word_four;
            speech_length = word_four_len;
            break;

        case SPEECH_WORD_FIVE:
            speech_data = word_five;
            speech_length = word_five_len;
            break;

        case SPEECH_WORD_SIX:
            speech_data = word_six;
            speech_length = word_six_len;
            break;

        case SPEECH_WORD_SEVEN:
            speech_data = word_seven;
            speech_length = word_seven_len;
            break;

        case SPEECH_WORD_EIGHT:
            speech_data = word_eight;
            speech_length = word_eight_len;
            break;

        case SPEECH_WORD_NINE:
            speech_data = word_nine;
            speech_length = word_nine_len;
            break;

        default:
            speech_data = 0;
            speech_length = 0U;
            return 0U;
    }

    return 1U;
}

/* --------------------------------------------------------- */
/* Select term audio                                         */
/* --------------------------------------------------------- */

static uint8_t Speech_SelectTerm(SpeechTerm_t term)
{
    switch (term)
    {
        case SPEECH_TERM_POINT:
            speech_data = term_point;
            speech_length = term_point_len;
            break;

        default:
            speech_data = 0;
            speech_length = 0U;
            return 0U;
    }

    return 1U;
}

/* --------------------------------------------------------- */
/* Clear complete DMA buffer                                 */
/* --------------------------------------------------------- */

static void Speech_FillSilence(void)
{
    uint32_t i;

    for (i = 0U; i < (SPEECH_BUFFER_FRAMES * 2U); i++)
    {
        speech_buffer[i] = 0;
    }
}

/* --------------------------------------------------------- */
/* Queue operations                                           */
/* --------------------------------------------------------- */

static uint8_t Speech_QueuePushWord(SpeechWord_t word)
{
    if (speech_queue_count >= SPEECH_QUEUE_SIZE)
    {
        return 0U;
    }

    speech_queue[speech_queue_tail].type = SPEECH_ITEM_WORD;
    speech_queue[speech_queue_tail].id = (uint16_t)word;

    speech_queue_tail++;

    if (speech_queue_tail >= SPEECH_QUEUE_SIZE)
    {
        speech_queue_tail = 0U;
    }

    speech_queue_count++;

    return 1U;
}

static uint8_t Speech_QueuePushTerm(SpeechTerm_t term)
{
    if (speech_queue_count >= SPEECH_QUEUE_SIZE)
    {
        return 0U;
    }

    speech_queue[speech_queue_tail].type = SPEECH_ITEM_TERM;
    speech_queue[speech_queue_tail].id = (uint16_t)term;

    speech_queue_tail++;

    if (speech_queue_tail >= SPEECH_QUEUE_SIZE)
    {
        speech_queue_tail = 0U;
    }

    speech_queue_count++;

    return 1U;
}

static uint8_t Speech_QueuePop(SpeechQueueItem_t *item)
{
    if (speech_queue_count == 0U)
    {
        return 0U;
    }

    *item = speech_queue[speech_queue_head];

    speech_queue_head++;

    if (speech_queue_head >= SPEECH_QUEUE_SIZE)
    {
        speech_queue_head = 0U;
    }

    speech_queue_count--;

    return 1U;
}

/* --------------------------------------------------------- */
/* Load next queued speech item                               */
/* --------------------------------------------------------- */

static uint8_t Speech_LoadNextItem(void)
{
    SpeechQueueItem_t item;

    if (Speech_QueuePop(&item) == 0U)
    {
        speech_data = 0;
        speech_length = 0U;
        speech_position = 0U;
        return 0U;
    }

    if (item.type == SPEECH_ITEM_WORD)
    {
        if (Speech_SelectWord((SpeechWord_t)item.id) == 0U)
        {
            return Speech_LoadNextItem();
        }
    }
    else
    {
        if (Speech_SelectTerm((SpeechTerm_t)item.id) == 0U)
        {
            return Speech_LoadNextItem();
        }
    }

    speech_position = 0U;
    speech_gap_remaining = 0U;
    speech_state = SPEECH_STATE_PLAYING;

    return 1U;
}

/* --------------------------------------------------------- */
/* Fill part of DMA buffer                                    */
/* --------------------------------------------------------- */

static void Speech_FillBuffer(int16_t *destination,
                              uint32_t frame_count)
{
    uint32_t i;

    /*
     * IMPORTANT:
     *
     * A new speech item is loaded only at the beginning of a
     * DMA-buffer fill, never in the middle of the current fill.
     *
     * This keeps the word -> silence -> word transition clean
     * and avoids changing speech_data/speech_position while
     * constructing the same DMA block.
     */

    if (speech_state == SPEECH_STATE_GAP)
    {
        if (speech_gap_remaining == 0U)
        {
            if (Speech_LoadNextItem() == 0U)
            {
                speech_state = SPEECH_STATE_STOPPED;
                speech_data = 0;
                speech_length = 0U;
                speech_position = 0U;
            }
        }
    }

    for (i = 0U; i < frame_count; i++)
    {
        int16_t sample = 0;

        if (speech_state == SPEECH_STATE_GAP)
        {
            /*
             * Output only digital zero during the complete gap.
             */
            sample = 0;

            if (speech_gap_remaining > 0U)
            {
                speech_gap_remaining--;

                /*
                 * Do NOT load the next item here.
                 * Wait for the next DMA half-buffer callback.
                 */
                if (speech_gap_remaining == 0U)
                {
                    speech_state = SPEECH_STATE_GAP;
                }
            }
        }
        else if (speech_state == SPEECH_STATE_PLAYING)
        {
            uint32_t source_index;

            source_index =
                speech_position / SPEECH_RESAMPLE_FACTOR;

            if (source_index < speech_length)
            {
                sample = Speech_ConvertSample(
                    speech_data[source_index]
                );

                speech_position++;
            }
            else
            {
                /*
                 * Current item has finished.
                 *
                 * From this point to the end of THIS DMA fill,
                 * output only zero.
                 */
                sample = 0;

                /*
                 * Always enter the gap after an item.  If the queue is
                 * empty, the gap is still played and the engine then
                 * remains stopped.  This also guarantees that the DMA
                 * buffer is filled with known zero samples after the
                 * final word.
                 */
                speech_gap_remaining = SPEECH_GAP_FRAMES;
                speech_state = SPEECH_STATE_GAP;
            }
        }
        else
        {
            /*
             * STOPPED = digital silence.
             */
            sample = 0;
        }

        destination[(2U * i) + 0U] = sample;
        destination[(2U * i) + 1U] = sample;
    }
}

/* --------------------------------------------------------- */
/* Start playback                                            */
/* --------------------------------------------------------- */

static void Speech_StartPlayback(void)
{
    if (Speech_LoadNextItem() == 0U)
    {
        speech_state = SPEECH_STATE_STOPPED;
        speech_data = 0;
        speech_length = 0U;
        Speech_FillSilence();
        return;
    }

    Speech_FillBuffer(
        speech_buffer,
        SPEECH_BUFFER_FRAMES
    );
}

/* --------------------------------------------------------- */
/* Initialize speech system                                  */
/* --------------------------------------------------------- */

void Speech_Init(void)
{
    speech_queue_head = 0U;
    speech_queue_tail = 0U;
    speech_queue_count = 0U;

    speech_data = 0;
    speech_length = 0U;
    speech_position = 0U;
    speech_gap_remaining = 0U;
    speech_state = SPEECH_STATE_STOPPED;

    Speech_FillSilence();

    (void)BSP_AUDIO_OUT_Init(
        OUTPUT_DEVICE_BOTH,
        70U,
        AUDIO_FREQUENCY_48K
    );
}

/* --------------------------------------------------------- */
/* Speak a numbered word                                     */
/* --------------------------------------------------------- */

void Speech_SpeakWord(SpeechWord_t word)
{
    if (word >= SPEECH_WORD_COUNT)
    {
        return;
    }

    if (Speech_QueuePushWord(word) == 0U)
    {
        return;
    }

    if (speech_state == SPEECH_STATE_STOPPED)
    {
        Speech_StartPlayback();

        (void)BSP_AUDIO_OUT_Play(
            (uint16_t *)speech_buffer,
            SPEECH_BUFFER_FRAMES * 2U * sizeof(int16_t)
        );
    }
}

/* --------------------------------------------------------- */
/* Speak a term such as "point"                              */
/* --------------------------------------------------------- */

void Speech_SpeakTerm(SpeechTerm_t term)
{
    if (term >= SPEECH_TERM_COUNT)
    {
        return;
    }

    if (Speech_QueuePushTerm(term) == 0U)
    {
        return;
    }

    if (speech_state == SPEECH_STATE_STOPPED)
    {
        Speech_StartPlayback();

        (void)BSP_AUDIO_OUT_Play(
            (uint16_t *)speech_buffer,
            SPEECH_BUFFER_FRAMES * 2U * sizeof(int16_t)
        );
    }
}

/* --------------------------------------------------------- */
/* DMA half-transfer callback                                */
/* --------------------------------------------------------- */

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
    /*
     * DMA keeps running.  Even when speech is stopped, refill the
     * released half with digital zero.  Never leave an old DMA half
     * untouched, otherwise stale audio can be replayed as hiss/noise.
     */
    Speech_FillBuffer(
        &speech_buffer[0],
        SPEECH_BUFFER_FRAMES / 2U
    );
}

/* --------------------------------------------------------- */
/* DMA transfer-complete callback                            */
/* --------------------------------------------------------- */

void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
    /*
     * Always refill the released half.  STOPPED means digital silence,
     * not "leave the DMA buffer untouched".
     */
    Speech_FillBuffer(
        &speech_buffer[SPEECH_BUFFER_FRAMES],
        SPEECH_BUFFER_FRAMES / 2U
    );
}

/* --------------------------------------------------------- */
/* Busy status                                               */
/* --------------------------------------------------------- */

uint8_t Speech_IsBusy(void)
{
    if (speech_state != SPEECH_STATE_STOPPED)
    {
        return 1U;
    }

    if (speech_queue_count != 0U)
    {
        return 1U;
    }

    return 0U;
}


void Speech_Speak(SpeechTerm_t term, float value)
{
    int32_t whole;
    int32_t decimal;

    /* Speak the selected term first */
    Speech_SpeakTerm(term);

    /* Handle negative values */
    if (value < 0.0f)
    {
        Speech_SpeakTerm(SPEECH_TERM_MINUS);
        value = -value;
    }

    /* Integer part */
    whole = (int32_t)value;

    /* Speak integer digits */
    if (whole == 0)
    {
        Speech_SpeakWord(SPEECH_WORD_ZERO);
    }
    else
    {
        int32_t divisor = 1;
        int32_t temp = whole;

        while (temp >= 10)
        {
            temp /= 10;
            divisor *= 10;
        }

        while (divisor > 0)
        {
            uint8_t digit = (uint8_t)(whole / divisor);

            Speech_SpeakWord((SpeechWord_t)digit);

            whole %= divisor;
            divisor /= 10;
        }
    }

    /* One decimal digit only */
    decimal = (int32_t)(value * 10.0f + 0.5f) % 10;

    if (decimal != 0)
    {
        Speech_SpeakTerm(SPEECH_TERM_POINT);
        Speech_SpeakWord((SpeechWord_t)decimal);
    }
}


