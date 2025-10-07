/**
 * @file animations.cpp
 * @brief Implementation of LED animations for the Word Clock.
 *
 * All blocking delay() calls have been replaced with vTaskDelay() to make
 * them suitable for a FreeRTOS environment.
 */

#include "animations.h"
#include <Arduino.h> // For random()
#include "word_layout.h"
#include "config.h"

// --- Helper for rainbowSentences ---
static bool firstWord = true;

// --- Full-Display Animations (RTOS-Friendly) ---

void indicateNumber(CRGB* leds, uint8_t num, CHSV color) {
    // Fade out over ~500ms
    for (int i = 0; i < 50; i++) {
        fadeToBlackBy(leds, NUM_LEDS, 16);
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    FastLED.clear();

    // Select and display the word for the given number
    const Word* word_to_show = nullptr;
    switch (num) {
        case 1: word_to_show = W_ONE; break;
        case 2: word_to_show = W_TWO; break;
        case 3: word_to_show = W_THREE; break;
        case 4: word_to_show = W_FOUR; break;
        case 5: word_to_show = W_FIVE; break;
        // Cases 6 through 12 can be added if needed
    }

    if (word_to_show) {
        // Use a simple, solid color for the indicator
        for (int i = 0; i < word_to_show->wordLength; i++) {
            leds[word_to_show->startIndex + i] = color;
        }
    }
    FastLED.show();
    
    // Hold for 500ms
    vTaskDelay(pdMS_TO_TICKS(500));

    // Fade out again
    for (int i = 0; i < 50; i++) {
        fadeToBlackBy(leds, NUM_LEDS, 16);
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    FastLED.clear();
    FastLED.show();
}

void wifiConnectAnimation(CRGB* leds) {
    uint8_t hue = 0;

    // Fade out existing display
    for (int i = 0; i < 100; i++) {
        fadeToBlackBy(leds, NUM_LEDS, 8);
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    // Rainbow animation for 6 seconds
    for (int i = 0; i < 600; i++) {
        writeAllWords(leds, CHSV(hue++, 255, 255), 10);
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Fade to black
    for (int i = 0; i < 100; i++) {
        fadeToBlackBy(leds, NUM_LEDS, 8);
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    FastLED.clear();
    FastLED.show();
}

// Helper to light up all words for animations

void writeAllWords(CRGB* ledArray, CHSV color, int rate) {
  for (int i = 0; i < numWords; i++) {
    rainbowRipple(clockWords[i], ledArray, color);
  }
  //vTaskDelay(pdMS_TO_TICKS(rate));
}


// --- Per-Word Color Scheme Implementations ---

void rainbowRipple(const Word& w, CRGB* ledArray, CHSV color) {
    uint8_t base = color.hue;
    for (int i = 0; i < w.wordLength; i++) {
        float letterX = w.x + i;
        float letterY = (float)w.y * 1.6;
        float distance = sqrt(pow(letterX, 2) + pow(letterY, 2));
        long dist = (long)(distance * 8);
        dist = dist % 256;
        color.hue = base - (uint8_t)dist;
        ledArray[w.startIndex + i] = color;
    }
}

void randomizedWordColors(const Word& w, CRGB* ledArray, CHSV color) {
    static uint8_t wordHues[23];
    static bool initialized = false;
    if (!initialized) {
        for(int i=0; i<numWords; i++) wordHues[i] = random8();
        initialized = true;
    }

    EVERY_N_SECONDS(1) {
        for (int k = 0; k < numWords; k++) {
            wordHues[k] = random8();
        }
    }

    uint8_t thisWordHue = 0;
    for (int k = 0; k < numWords; k++) {
        if (clockWords[k].startIndex == w.startIndex) {
            thisWordHue = wordHues[k];
            break;
        }
    }
    for (int i = 0; i < w.wordLength; i++) {
        ledArray[w.startIndex + i] = CHSV(thisWordHue, 255, 255);
    }
}

void rainbowSentences(const Word& w, CRGB* ledArray, CHSV color) {
    static uint8_t hueIndex = 0;
    if (firstWord) {
        hueIndex = color.hue;
        firstWord = false;
    }
    for (int i = 0; i < w.wordLength; i++) {
        ledArray[w.startIndex + i] = CHSV(hueIndex, 255, 255);
        hueIndex -= 8;
    }
}

void timeColorChange(const Word& w, CRGB* ledArray, CHSV color) {
    // This animation changes color over 24 hours. The hue is based on the time.
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        unsigned int dayMinutes = (timeinfo.tm_hour * 60) + timeinfo.tm_min;
        CHSV timeColor = CHSV(map(dayMinutes, 0, 1439, 0, 255), 255, 255);
        for (int i = 0; i < w.wordLength; i++) {
            ledArray[w.startIndex + i] = timeColor;
        }
    }
}

void noiseFieldWords(const Word& w, CRGB* ledArray, CHSV color) {
    uint16_t scale = 2000;
    for (int i = 0; i < w.wordLength; i++) {
        uint32_t real_x = (10 * w.x + (10 * i)) * scale;
        uint32_t real_y = 10 * w.y * scale;
        uint32_t real_z = millis() * 20;
        uint8_t noise = inoise16(real_x, real_y, real_z) >> 8;
        ledArray[w.startIndex + i] = CHSV(noise, 255, 255);
    }
}

