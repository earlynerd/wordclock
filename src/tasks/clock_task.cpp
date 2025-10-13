/**
 * @file clock_task.cpp
 * @brief Implements the FreeRTOS task for updating the clock display.
 *
 * This task is responsible for all visual output. It handles commands
 * to change color schemes and triggers the WiFi task for a manual time sync.
 */

#include "clock_task.h"
#include "wifi_task.h"
#include "time_display.h"
#include "animations.h"
#include "../config.h"
#include <FastLED.h>
#include <TimeLib.h>

// --- Shared extern variables ---
extern CRGB leds[];
extern QueueHandle_t clockCommandQueue;
extern int utc_offset_sec; // We now use the UTC offset directly

// --- Task-local state ---
static int colorSchemeIndex = 0;
static uint8_t baseHue = 0;
bool first_run = true;
bool time_valid = false;

// --- Command Handler ---
static void handleCommand(const ClockCommand& cmd) {
    switch (cmd.type) {
        case CommandType::NEXT_COLOR_SCHEME:
            colorSchemeIndex = (colorSchemeIndex + 1) % NUM_COLOR_SCHEMES;
            indicateNumber(leds, colorSchemeIndex + 1 , CHSV(baseHue, 255, 255));
            break;
        case CommandType::PREV_COLOR_SCHEME:
            colorSchemeIndex--;
            if (colorSchemeIndex < 0) colorSchemeIndex = NUM_COLOR_SCHEMES - 1;
            indicateNumber(leds, colorSchemeIndex + 1,  CHSV(baseHue, 255, 255));
            break;
         case CommandType::SHOW_WIFI_ANIMATION:
            wifiConnectAnimation(leds);
            break;
        case CommandType::START_CLOCK_DISPLAY:
            time_valid = true;
            vTaskDelay(pdMS_TO_TICKS(3000));
            break;
    }
}

// --- Main Task Function ---
void taskClockUpdate(void *pvParameters) {
    Serial.println("Clock Task started.");
    ClockCommand receivedCommand;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(25);

    for (;;) {
        // 1. Check for incoming commands without blocking.
        if (xQueueReceive(clockCommandQueue, &receivedCommand, 0) == pdPASS) {
            handleCommand(receivedCommand);
        }

        // 2. Calculate local time.
        // `now()` from TimeLib returns UTC time because our sync provider (RTC) is in UTC.
        time_t now;
        struct tm timeinfo;
        time(&now); // Get current system time (epoch)
        localtime_r(&now, &timeinfo); // Convert to local time structure

        // --- One-time debug print logic ---
        if (first_run) {
            char time_buf[64];
            // Format the time into a human-readable string
            strftime(time_buf, sizeof(time_buf), "%A, %B %d %Y %H:%M:%S %Z", &timeinfo);
            Serial.printf("[Clock Task] First time displayed: %s (Epoch: %ld)\n", time_buf, now);
            first_run = false;
        }
        // ------------------------------------

        // Update the display with the current time and color scheme
        if(time_valid)
        {
            writeTime(timeinfo.tm_hour, timeinfo.tm_min, leds,CHSV(baseHue, 255, 255),(ColorScheme)colorSchemeIndex);
            FastLED.show();
        } 
        else wifiConnectAnimation(leds);
        // Add a single pixel indicator for power/status
        //leds[0] = CRGB(5, 5, 5);
        
        
        vTaskDelay(pdMS_TO_TICKS(20)); // Update rate of 50Hz
    }
}

