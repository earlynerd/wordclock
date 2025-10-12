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
#include <WiFi.h>
#include "Adafruit_ThinkInk.h"
#include <SPI.h>

// --- Global Handles & Objects ---
RTC_DS3231 rtc;
CRGB leds[NUM_LEDS];
Adafruit_IL0373 display(212, 104, EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY, EPD_SPI);


QueueHandle_t clockCommandQueue;
QueueHandle_t networkEventQueue;



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
    WiFi.onEvent(WiFiEvent);
    if(!digitalRead(BUTTON_1_PIN) && !digitalRead(BUTTON_2_PIN))
    {
        Serial.println("both buttons pressed at boot, NVS clear triggered.");
        preferences.clear();
        preferences.end();
        ClockCommand wificmd;
        wificmd.type = CommandType::CLEAR_WIFI;
        xTaskCreatePinnedToCore(taskWiFi, "WiFi Task", 30000, (void*)&wificmd, 1, NULL, 0);
        vTaskDelay(portMAX_DELAY);
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
    networkEventQueue = xQueueCreate(5, sizeof(NetworkEvent_t));
    if (networkEventQueue == NULL)
    {
        Serial.println("[ERROR] Could not create the network event queue!");
    }
    Serial.println("--- Initial Heap Status ---");
    log_heap_status();
    Serial.println("---------------------------");

    xTaskCreate(taskWiFi, "WiFi Task", 16353, NULL, 1, &wifiTaskHandle);
    xTaskCreate(taskLogHeap, "Heap Logger", 2048, NULL, 0, &heapTaskHandle);
    xTaskCreate(taskClockUpdate, "Clock Task", 6000, &clockCommandQueue, 5, &clockTaskHandle);
    xTaskCreate(taskButtonCheck, "Button Task", 2048, NULL, 3, &buttonTaskHandle);

    Serial.println("Setup complete. Tasks are running.");
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}
