/*
 * speech_engine.h
 *
 *  Created on: 08-Sept-2026
 *      Author: mishra-ji
 */

#ifndef SPEECH_ENGINE_H
#define SPEECH_ENGINE_H

#include <stdint.h>

/* Numeric words: 0 to 9 */
typedef enum
{
    SPEECH_WORD_ZERO = 0,
    SPEECH_WORD_ONE,
    SPEECH_WORD_TWO,
    SPEECH_WORD_THREE,
    SPEECH_WORD_FOUR,
    SPEECH_WORD_FIVE,
    SPEECH_WORD_SIX,
    SPEECH_WORD_SEVEN,
    SPEECH_WORD_EIGHT,
    SPEECH_WORD_NINE,

    SPEECH_WORD_COUNT
} SpeechWord_t;


/* Fixed speech terms */
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

void Speech_Speak(SpeechTerm_t term, float value);


/* Speech engine API */
void Speech_Init(void);

void Speech_SpeakWord(SpeechWord_t word);

void Speech_SpeakTerm(SpeechTerm_t term);
uint8_t Speech_IsBusy(void);

#endif /* SPEECH_ENGINE_H */
