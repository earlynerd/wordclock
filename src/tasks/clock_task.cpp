/**
 * @file clock_task.cpp
 * @brief Implements the FreeRTOS task for updating the clock display.
 *
 * This task is the main display loop. It waits for commands to change display
 * modes (like color schemes or animations) and continuously updates the
 * time on the LED matrix. It accesses all hardware and state via the AppContext.
 */

#include "clock_task.h"
#include "../AppContext.h"
#include "../time_display.h"
#include "../animations.h"
#include <TimeLib.h>

/**
 * @brief Handles incoming commands from the system command queue.
 * @param context Pointer to the shared application context.
 * @param cmd The command to be processed.
 */
static void handleCommand(AppContext* context, const SystemCommand& cmd) {
    uint8_t baseHue = (millis() / 60) % 256;
    switch (cmd.type) {
        case SystemCommandType::NEXT_COLOR_SCHEME:
            context->colorSchemeIndex = (context->colorSchemeIndex + 1) % NUM_COLOR_SCHEMES;
            indicateNumber(context->leds, context->colorSchemeIndex + 1, CHSV(baseHue, 255, 255));
            break;
        case SystemCommandType::PREV_COLOR_SCHEME:
            context->colorSchemeIndex--;
            if (context->colorSchemeIndex < 0) {
                context->colorSchemeIndex = NUM_COLOR_SCHEMES - 1;
            }
            indicateNumber(context->leds, context->colorSchemeIndex + 1, CHSV(baseHue, 255, 255));
            break;
        case SystemCommandType::SHOW_WIFI_ANIMATION:
            wifiConnectAnimation(context->leds);
            break;
        case SystemCommandType::START_CLOCK_DISPLAY:
            context->time_is_valid = true;
            // Add a delay to show connection success before showing time
            vTaskDelay(pdMS_TO_TICKS(2000));
            break;
    }
}

void taskClockUpdate(void *pvParameters) {
    Serial.println("Clock Task started.");
    auto* context = static_cast<AppContext*>(pvParameters);
    SystemCommand receivedCommand;
    bool first_run = true;

    for (;;) {
        // 1. Check for incoming commands without blocking.
        if (xQueueReceive(context->systemCommandQueue, &receivedCommand, 0) == pdPASS) {
            handleCommand(context, receivedCommand);
        }

        // 2. Update display based on current state
        if (context->time_is_valid) {
            time_t now_utc;
            struct tm timeinfo_local;
            time(&now_utc); // Get current system time (UTC epoch)
            localtime_r(&now_utc, &timeinfo_local); // Convert to local time structure

            // One-time debug print to confirm time is being displayed
            if (first_run) {
                char time_buf[64];
                strftime(time_buf, sizeof(time_buf), "%A, %B %d %Y %H:%M:%S %Z", &timeinfo_local);
                Serial.printf("[Clock Task] First time displayed: %s\n", time_buf);
                first_run = false;
            }

            // Update the display with the current time and color scheme
            uint8_t baseHue = (millis() / 60) % 256; // Slowly cycle hue over time
            writeTime(timeinfo_local.tm_hour, timeinfo_local.tm_min, context->leds, CHSV(baseHue, 255, 255), (ColorScheme)context->colorSchemeIndex);
            
        } else {
            // If time is not valid yet, just keep the LEDs off.
            fadeToBlackBy(context->leds, NUM_LEDS, 10);
        }
        
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(20)); // ~50Hz update rate
    }
}
