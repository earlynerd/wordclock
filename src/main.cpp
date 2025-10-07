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
#include <TimeLib.h>
#include <sys/time.h>
#include <Preferences.h>
#include <stdlib.h>

// --- Global Handles & Objects ---
RTC_DS3231 rtc;
CRGB leds[NUM_LEDS];
QueueHandle_t clockCommandQueue;
TaskHandle_t clockTaskHandle;
TaskHandle_t buttonTaskHandle;
TaskHandle_t wifiTaskHandle;
TaskHandle_t heapTaskHandle;
Preferences preferences; // For saving timezone

// --- Global Time Variables ---
char time_zone[64] = "UTC";

void log_heap_status()
{
    struct tm timeinfo;
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%A, %B %d %Y %H:%M:%S %Z", &timeinfo);
    Serial.printf("[RAM] %s: ", time_buf);
    Serial.printf("Free Heap: %d bytes | Min Free Heap: %d bytes\n",
                  ESP.getFreeHeap(),
                  ESP.getMinFreeHeap());
}

void taskLogHeap(void *pvParameters)
{
    for (;;)
    {
        log_heap_status();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void setup()
{
    pinMode(BUTTON_1_PIN, INPUT_PULLUP);
    pinMode(BUTTON_2_PIN, INPUT_PULLUP);
    Serial.begin(115200);
    //while (!Serial)
    //    ;
    Serial.println("\n--- Word Clock Starting Up ---");
    preferences.begin(NVS_NAMESPACE, false);
    if(!digitalRead(BUTTON_1_PIN) && !digitalRead(BUTTON_2_PIN))
    {
        preferences.clear();
    }
    

    if (!rtc.begin())
    {
        Serial.println("[ERROR] Couldn't find RTC! Clock will not keep time without power.");
    }
    else
    {
        // Check if the RTC has lost power and its time is invalid
        if (rtc.lostPower())
        {
            Serial.println("[WARN] RTC lost power. Setting time to compile time as a fallback.");
            // This sets the RTC to the time this sketch was compiled
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }

        DateTime now = rtc.now();
        // Sanity check the year to ensure the time is plausible before using it
        if (now.year() < 2024)
        {
            Serial.printf("[WARN] RTC has an invalid time (Year: %d). Waiting for WiFi sync.\n", now.year());
            // Do not set system time from an invalid RTC time
        }
        else
        {
            // The RTC time is valid, so set the system time
            struct timeval tv;
            tv.tv_sec = now.unixtime();
            settimeofday(&tv, NULL);
            Serial.println("System time initialized from hardware RTC.");

            // Apply the last known timezone from persistent storage
            String tz_string = preferences.getString(NVS_TZ_KEY, "");
            if (tz_string.length() > 0)
            {
                strncpy(time_zone, tz_string.c_str(), sizeof(time_zone) - 1);
                setenv("TZ", time_zone, 1);
                tzset();
                Serial.printf("Timezone set from NVS: %s\n", time_zone);
            }
            else
            {
                Serial.println("Timezone not yet known, defaulting to UTC for now.");
            }
        }
    }

    

    FastLED.addLeds<LED_TYPE, DATA_PIN_WC, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();

    clockCommandQueue = xQueueCreate(5, sizeof(ClockCommand));
    if (clockCommandQueue == NULL)
    {
        Serial.println("[ERROR] Could not create the clock command queue!");
    }

    Serial.println("--- Initial Heap Status ---");
    log_heap_status();
    Serial.println("---------------------------");

    xTaskCreatePinnedToCore(taskWiFi, "WiFi Task", 16353, NULL, 1, &wifiTaskHandle, 1);
    xTaskCreatePinnedToCore(taskLogHeap, "Heap Logger", 2048, NULL, 0, &heapTaskHandle, 1);
    xTaskCreatePinnedToCore(taskClockUpdate, "Clock Task", 6000, &clockCommandQueue, 5, &clockTaskHandle, 0);
    xTaskCreatePinnedToCore(taskButtonCheck, "Button Task", 2048, NULL, 3, &buttonTaskHandle, 1);

    Serial.println("Setup complete. Tasks are running.");
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}
