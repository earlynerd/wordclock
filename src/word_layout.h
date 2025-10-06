/**
 * @file word_layout.h
 * @brief Defines the structure and layout of words on the clock face.
 */

#ifndef WORD_LAYOUT_H
#define WORD_LAYOUT_H

#include <stdint.h>

// Represents a single word on the clock face
struct Word {
    uint8_t startIndex; // The index of the first LED for this word
    uint8_t wordLength; // How many LEDs are in this word
    uint8_t x;          // X-coordinate on the grid (for animations)
    uint8_t y;          // Y-coordinate on the grid (for animations)
};

// Extern declarations make these variables available to other files.
// The actual data is defined in word_layout.cpp.
extern const Word* W_IT;
extern const Word* W_IS;
extern const Word* W_TEN_MIN;
extern const Word* W_HALF;
extern const Word* W_QUARTER;
extern const Word* W_TWENTY;
extern const Word* W_FIVE_MIN;
extern const Word* W_MINUTES;
extern const Word* W_PAST;
extern const Word* W_TO;
extern const Word* W_ONE;
extern const Word* W_TWO;
extern const Word* W_THREE;
extern const Word* W_FOUR;
extern const Word* W_FIVE;
extern const Word* W_SIX;
extern const Word* W_SEVEN;
extern const Word* W_EIGHT;
extern const Word* W_NINE;
extern const Word* W_TEN;
extern const Word* W_ELEVEN;
extern const Word* W_TWELVE;
extern const Word* W_OCLOCK;

extern const Word clockWords[];
extern const uint8_t numWords;

#endif // WORD_LAYOUT_H
