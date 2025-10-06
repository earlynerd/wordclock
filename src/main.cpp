/**
 * @file main.cpp
 * @brief Main application entry point for the ESP32 Word Clock.
 *
 * Initializes hardware, syncs system time from the RTC on boot using the
 * last known timezone from NVS, and creates the FreeRTOS tasks. Includes
 * robust checks for RTC validity.
 */

#include "config.h"
#include "tasks/clock_task.h"
#include "tasks/button_task.h"
#include "tasks/wifi_task.h"
#include <RTClib.h>
#include <FastLED.h>
#include <time.h>
#include <sys/time.h>
#include <Preferences.h>

// --- Global Handles & Objects ---
RTC_DS3231 rtc;
CRGB leds[NUM_LEDS];
QueueHandle_t clockCommandQueue;
TaskHandle_t clockTaskHandle;
Preferences preferences; // For saving timezone
int utc_offset_seconds;

// --- Global Time Variables ---
char time_zone[64] = "UTC";

void log_heap_status() {
    Serial.printf("[RAM] Free Heap: %d bytes | Min Free Heap: %d bytes\n",
                  ESP.getFreeHeap(),
                  ESP.getMinFreeHeap());
}

void taskLogHeap(void *pvParameters) {
    for (;;) {
        log_heap_status();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("\n--- Word Clock Starting Up ---");

    preferences.begin(NVS_NAMESPACE, false);

    if (!rtc.begin()) {
        Serial.println("[ERROR] Couldn't find RTC! Clock will not keep time without power.");
    } else {
        // Check if the RTC has lost power and its time is invalid
        if (rtc.lostPower()) {
            Serial.println("[WARN] RTC lost power. Setting time to compile time as a fallback.");
            // This sets the RTC to the time this sketch was compiled
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }

        DateTime now = rtc.now();
        // Sanity check the year to ensure the time is plausible before using it
        if (now.year() < 2024) { 
            Serial.printf("[WARN] RTC has an invalid time (Year: %d). Waiting for WiFi sync.\n", now.year());
            // Do not set system time from an invalid RTC time
        } else {
            // The RTC time is valid, so set the system time
            struct timeval tv;
            tv.tv_sec = now.unixtime();
            settimeofday(&tv, NULL);
            Serial.println("System time initialized from hardware RTC.");

            // Apply the last known timezone from persistent storage
            String tz_string = preferences.getString(NVS_TZ_KEY, "");
            if (tz_string.length() > 0) {
                strncpy(time_zone, tz_string.c_str(), sizeof(time_zone) - 1);
                setenv("TZ", time_zone, 1);
                tzset();
                Serial.printf("Timezone set from NVS: %s\n", time_zone);
            } else {
                Serial.println("Timezone not yet known, defaulting to UTC for now.");
            }
        }
    }

    pinMode(BUTTON_1_PIN, INPUT_PULLUP);
    pinMode(BUTTON_2_PIN, INPUT_PULLUP);

    FastLED.addLeds<LED_TYPE, DATA_PIN_WC, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();

    clockCommandQueue = xQueueCreate(5, sizeof(ClockCommand));
    if (clockCommandQueue == NULL) {
        Serial.println("[ERROR] Could not create the clock command queue!");
    }
    
    Serial.println("--- Initial Heap Status ---");
    log_heap_status();
    Serial.println("---------------------------");

    xTaskCreate(taskClockUpdate, "Clock Task", 4096, NULL, 5, &clockTaskHandle);
    xTaskCreate(taskButtonCheck, "Button Task", 2048, NULL, 3, NULL);
    xTaskCreate(taskWiFi, "WiFi Task", 8192, NULL, 1, NULL);
    xTaskCreate(taskLogHeap, "Heap Logger", 2048, NULL, 0, NULL);

    Serial.println("Setup complete. Tasks are running.");
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}

