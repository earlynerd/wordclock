/**
 * @file time_display.cpp
 * @brief Implements the logic for displaying time on the word clock matrix.
 *
 * This file translates hours and minutes into the specific words that need
 * to be lit up on the LED display.
 */

#include "time_display.h"
#include "word_layout.h"
#include "animations.h"

// --- Helper for animations that progress across a sentence ---
static bool firstWord = true;

void writeWord(const Word& w, CRGB* ledArray, CHSV color, ColorScheme scheme) {
    // This function now acts as a dispatcher to the correct animation function.
    switch (scheme) {
        case RAINBOW_RIPPLE:
            rainbowRipple(w, ledArray, color);
            break;
        case NOISE_FIELD:
            noiseFieldWords(w, ledArray, color);
            break;
        case RANDOMIZED_WORDS:
            randomizedWordColors(w, ledArray, color);
            break;
        case RAINBOW_SENTENCE:
            rainbowSentences(w, ledArray, color);
            break;
        //case TIME_COLOR_CHANGE:
        //    timeColorChange(w, ledArray, color);
        //    break;
        default: // Fallback to a simple solid color
            for (int i = 0; i < w.wordLength; i++) {
                ledArray[w.startIndex + i] = color;
            }
            break;
    }
}


void writeTime(int hours, int minutes, CRGB* ledArray, CHSV color, ColorScheme scheme) {
    firstWord = true; // Reset for sentence-based animations

    // Normalize to 12-hour format
    if (hours >= 12) hours -= 12;
    if (hours == 0) hours = 12;

    fadeToBlackBy(ledArray, NUM_LEDS, 48);
    writeWord(*W_IT, ledArray, color, scheme);
    writeWord(*W_IS, ledArray, color, scheme);

    int roundedMinutes = (minutes / 5) * 5; // Round down to nearest 5 minutes
    int hour_to_display = hours;

    // Adjust hour for "to" times (e.g., "ten to five")
    if (minutes > 34) {
        hour_to_display = (hours % 12) + 1;
    }

    switch (roundedMinutes) {
        case 0:
            writeWord(*W_OCLOCK, ledArray, color, scheme);
            break;
        case 5:
            writeWord(*W_FIVE_MIN, ledArray, color, scheme);
            writeWord(*W_MINUTES, ledArray, color, scheme);
            writeWord(*W_PAST, ledArray, color, scheme);
            break;
        case 10:
            writeWord(*W_TEN_MIN, ledArray, color, scheme);
            writeWord(*W_MINUTES, ledArray, color, scheme);
            writeWord(*W_PAST, ledArray, color, scheme);
            break;
        case 15:
            writeWord(*W_QUARTER, ledArray, color, scheme);
            writeWord(*W_PAST, ledArray, color, scheme);
            break;
        case 20:
            writeWord(*W_TWENTY, ledArray, color, scheme);
            writeWord(*W_MINUTES, ledArray, color, scheme);
            writeWord(*W_PAST, ledArray, color, scheme);
            break;
        case 25:
            writeWord(*W_TWENTY, ledArray, color, scheme);
            writeWord(*W_FIVE_MIN, ledArray, color, scheme);
            writeWord(*W_MINUTES, ledArray, color, scheme);
            writeWord(*W_PAST, ledArray, color, scheme);
            break;
        case 30:
            writeWord(*W_HALF, ledArray, color, scheme);
            writeWord(*W_PAST, ledArray, color, scheme);
            break;
        case 35:
            writeWord(*W_TWENTY, ledArray, color, scheme);
            writeWord(*W_FIVE_MIN, ledArray, color, scheme);
            writeWord(*W_MINUTES, ledArray, color, scheme);
            writeWord(*W_TO, ledArray, color, scheme);
            break;
        case 40:
            writeWord(*W_TWENTY, ledArray, color, scheme);
            writeWord(*W_MINUTES, ledArray, color, scheme);
            writeWord(*W_TO, ledArray, color, scheme);
            break;
        case 45:
            writeWord(*W_QUARTER, ledArray, color, scheme);
            writeWord(*W_TO, ledArray, color, scheme);
            break;
        case 50:
            writeWord(*W_TEN_MIN, ledArray, color, scheme);
            writeWord(*W_MINUTES, ledArray, color, scheme);
            writeWord(*W_TO, ledArray, color, scheme);
            break;
        case 55:
            writeWord(*W_FIVE_MIN, ledArray, color, scheme);
            writeWord(*W_MINUTES, ledArray, color, scheme);
            writeWord(*W_TO, ledArray, color, scheme);
            break;
    }

    switch (hour_to_display) {
        case 1: writeWord(*W_ONE, ledArray, color, scheme); break;
        case 2: writeWord(*W_TWO, ledArray, color, scheme); break;
        case 3: writeWord(*W_THREE, ledArray, color, scheme); break;
        case 4: writeWord(*W_FOUR, ledArray, color, scheme); break;
        case 5: writeWord(*W_FIVE, ledArray, color, scheme); break;
        case 6: writeWord(*W_SIX, ledArray, color, scheme); break;
        case 7: writeWord(*W_SEVEN, ledArray, color, scheme); break;
        case 8: writeWord(*W_EIGHT, ledArray, color, scheme); break;
        case 9: writeWord(*W_NINE, ledArray, color, scheme); break;
        case 10: writeWord(*W_TEN, ledArray, color, scheme); break;
        case 11: writeWord(*W_ELEVEN, ledArray, color, scheme); break;
        case 12: writeWord(*W_TWELVE, ledArray, color, scheme); break;
    }
}

