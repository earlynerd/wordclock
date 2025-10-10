/**
 * @file animations.h
 * @brief Function prototypes and definitions for LED animations.
 */
#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <FastLED.h>
#include "word_layout.h"
#include "config.h"

// Enum to define the different color schemes available.
enum ColorScheme {
    RAINBOW_RIPPLE,
    NOISE_FIELD,
    RANDOMIZED_WORDS,
    RAINBOW_SENTENCE,
    TIME_COLOR_CHANGE,
    NUM_COLOR_SCHEMES // Helper to get the count of schemes
};

// --- Animation Functions ---
// These are called by writeWord based on the selected color scheme.
void rainbowRipple(const Word& w, CRGB* ledArray, CHSV color);
void noiseFieldWords(const Word& w, CRGB* ledArray, CHSV color);
void randomizedWordColors(const Word& w, CRGB* ledArray, CHSV color);
void rainbowSentences(const Word& w, CRGB* ledArray, CHSV color);
void timeColorChange(const Word& w, CRGB* ledArray, CHSV color);

// --- Full-Display Animations ---
// These animations take over the display and are RTOS-friendly.
void indicateNumber(CRGB* leds, uint8_t num, CHSV color);
void wifiConnectAnimation(CRGB* leds);
void writeAllWords(CRGB* ledArray, CHSV color, int rate);

#endif // ANIMATIONS_H

