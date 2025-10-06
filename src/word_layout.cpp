/**
 * @file word_layout.cpp
 * @brief Contains the specific data for the word layout on the clock face.
 *
 * This keeps the main code clean from large data definitions.
 */
#include <Arduino.h>
#include "word_layout.h"

// Define all the individual words. Using static keeps them local to this file.
// Grid is 13 letters wide and 8 rows tall, origin in lower left.
static const Word words[] = {
    {1,  1, 0, 7}, // IT
    {2,  1, 3, 7}, // IS
    {3,  2, 6, 7}, // TEN (minutes)
    {5,  2, 9, 7}, // HALF
    {7,  4, 0, 6}, // QUARTER
    {11, 4, 7, 6}, // TWENTY
    {15, 2, 0, 5}, // FIVE (minutes)
    {17, 4, 5, 5}, // MINUTES
    {21, 2, 0, 4}, // PAST
    {23, 1, 4, 4}, // TO
    {24, 2, 7, 4}, // ONE
    {26, 2, 10,4}, // TWO
    {28, 3, 0, 3}, // THREE
    {31, 2, 5, 3}, // FOUR
    {33, 2, 9, 3}, // FIVE
    {35, 2, 0, 2}, // SIX
    {37, 3, 3, 2}, // SEVEN
    {40, 3, 8, 2}, // EIGHT
    {43, 2, 0, 1}, // NINE
    {45, 2, 4, 1}, // TEN
    {47, 4, 7, 1}, // ELEVEN
    {51, 3, 0, 0}, // TWELVE
    {54, 4, 5, 0}  // O'CLOCK
};

// Create pointers to make the time display logic more readable
const Word* W_IT       = &words[0];
const Word* W_IS       = &words[1];
const Word* W_TEN_MIN  = &words[2];
const Word* W_HALF     = &words[3];
const Word* W_QUARTER  = &words[4];
const Word* W_TWENTY   = &words[5];
const Word* W_FIVE_MIN = &words[6];
const Word* W_MINUTES  = &words[7];
const Word* W_PAST     = &words[8];
const Word* W_TO       = &words[9];
const Word* W_ONE      = &words[10];
const Word* W_TWO      = &words[11];
const Word* W_THREE    = &words[12];
const Word* W_FOUR     = &words[13];
const Word* W_FIVE     = &words[14];
const Word* W_SIX      = &words[15];
const Word* W_SEVEN    = &words[16];
const Word* W_EIGHT    = &words[17];
const Word* W_NINE     = &words[18];
const Word* W_TEN      = &words[19];
const Word* W_ELEVEN   = &words[20];
const Word* W_TWELVE   = &words[21];
const Word* W_OCLOCK   = &words[22];


// For iterating over all words in animations
const Word clockWords[] = {
    {1,  1, 0, 7}, {2,  1, 3, 7}, {3,  2, 6, 7}, {5,  2, 9, 7}, {7,  4, 0, 6},
    {11, 4, 7, 6}, {15, 2, 0, 5}, {17, 4, 5, 5}, {21, 2, 0, 4}, {23, 1, 4, 4},
    {24, 2, 7, 4}, {26, 2, 10,4}, {28, 3, 0, 3}, {31, 2, 5, 3}, {33, 2, 9, 3},
    {35, 2, 0, 2}, {37, 3, 3, 2}, {40, 3, 8, 2}, {43, 2, 0, 1}, {45, 2, 4, 1},
    {47, 4, 7, 1}, {51, 3, 0, 0}, {54, 4, 5, 0}
};
const uint8_t numWords = sizeof(clockWords) / sizeof(Word);
