/**
 * @file time_display.h
 * @brief Function prototypes for displaying time on the word clock matrix.
 */
#ifndef TIME_DISPLAY_H
#define TIME_DISPLAY_H

#include <FastLED.h>
#include "word_layout.h"
#include "animations.h" // Needed for ColorScheme enum

/**
 * @brief Displays the given time on the LED matrix using words.
 * @param hours The current hour (0-23).
 * @param minutes The current minute (0-59).
 * @param ledArray Pointer to the CRGB LED array.
 * @param color The base color for the display.
 * @param scheme The color animation scheme to use.
 */
void writeTime(int hours, int minutes, CRGB* ledArray, CHSV color, ColorScheme scheme);

/**
 * @brief Lights up a single word on the LED matrix.
 * @param w The Word struct defining the word to light up.
 * @param ledArray Pointer to the CRGB LED array.
 * @param color The base color for the display.
 * @param scheme The color animation scheme to use.
 */
void writeWord(const Word& w, CRGB* ledArray, CHSV color, ColorScheme scheme);

#endif // TIME_DISPLAY_H

