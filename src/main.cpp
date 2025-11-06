/**
 * @file main.cpp
 * @brief Main application entry point for the ESP32 Word Clock.
 *
 * Initializes the central AppContext, hardware, and FreeRTOS tasks.
 * The AppContext is passed to each task to provide access to shared resources.
 */

#include "config.h"
#include "AppContext.h"
#include "tasks/clock_task.h"
#include "tasks/button_task.h"
#include "tasks/wifi_task.h"
#include <time.h>
#include <TimeLib.h>
#include <sys/time.h>
#include <WiFi.h>
#include <esp_sntp.h>

// --- Global Application Context ---
// This is the single global variable that holds all shared state.
AppContext appContext;

// --- Task Handles ---
TaskHandle_t clockTaskHandle;
TaskHandle_t buttonTaskHandle;
TaskHandle_t wifiTaskHandle;
TaskHandle_t heapTaskHandle;
TaskHandle_t epdTaskHandle;

// --- Forward Declarations ---
void log_heap_status();
void taskLogHeap(void *pvParameters);
void WiFiEvent(WiFiEvent_t event);
void SNTPEvent(struct timeval *tv);

void setup()
{
    pinMode(BUTTON_1_PIN, INPUT_PULLUP);
    pinMode(BUTTON_2_PIN, INPUT_PULLUP);
    Serial.begin(115200);
    Serial.println("\n--- Word Clock Starting Up ---");

    // Initialize Preferences from the context
    appContext.preferences.begin(NVS_NAMESPACE, false);

    // Initialize Queues in the context
    appContext.systemCommandQueue = xQueueCreate(5, sizeof(SystemCommand));
    appContext.networkEventQueue = xQueueCreate(5, sizeof(NetworkEvent_t));
    appContext.epdQueue = xQueueCreate(5, 0); // Queue for signaling EPD updates

    if (!appContext.systemCommandQueue || !appContext.networkEventQueue || !appContext.epdQueue)
    {
        Serial.println("[ERROR] Failed to create one or more queues! Halting.");
        while (1)
            ;
    }

    

    // Check for NVS clear command on boot (holding both buttons)
    if (!digitalRead(BUTTON_1_PIN) && !digitalRead(BUTTON_2_PIN))
    {
        Serial.println("Both buttons pressed at boot, clearing NVS and WiFi credentials.");
        appContext.preferences.clear();
        NetworkEvent_t evt = NetworkEvent_t::CLEAR_WIFI;
        xQueueSend(appContext.networkEventQueue, &evt, 0);
    }

    // Initialize LED Strip using the 'leds' array in the context
    FastLED.addLeds<LED_TYPE, DATA_PIN_WC, COLOR_ORDER>(appContext.leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();

    Serial.println("--- Initial Heap Status ---");
    log_heap_status(); // Log once at startup for immediate feedback
    Serial.println("---------------------------");

    // Create Tasks, passing a pointer to the global AppContext to each one
    xTaskCreatePinnedToCore(task_epd, "Epaper Task", 16535, &appContext, 2, &epdTaskHandle, 0);
    vTaskDelay(5000);
    
    //vTaskDelay(10000);
    xTaskCreatePinnedToCore(taskLogHeap, "Heap Logger", 2048, NULL, 0, &heapTaskHandle, 1);
    xTaskCreatePinnedToCore(taskClockUpdate, "Clock Task", 4096, &appContext, 5, &clockTaskHandle, 1);
    xTaskCreatePinnedToCore(taskButtonCheck, "Button Task", 2048, &appContext, 3, &buttonTaskHandle, 1);
    //vTaskDelay(30000);
    // WiFi Event Handler Setup
    WiFi.onEvent(WiFiEvent);
    sntp_set_time_sync_notification_cb(SNTPEvent);
    xTaskCreatePinnedToCore(taskWiFi, "WiFi Task", 16535, &appContext, 1, &wifiTaskHandle, 0);
    Serial.println("Setup complete. Tasks are running.");

    // Trigger initial WiFi connection process
    NetworkEvent_t bootEvt = NetworkEvent_t::WIFI_BOOT;
    xQueueSend(appContext.networkEventQueue, &bootEvt, portMAX_DELAY);
}

void loop()
{
    // The main loop is empty because we are using FreeRTOS tasks for all operations.
    vTaskDelay(portMAX_DELAY);
}

// --- Heap Logging Helper Function ---
// This contains just the logging logic without any loops.
void log_heap_status()
{
    Serial.printf("[RAM] Free Heap: %u bytes | Min Free Heap: %u bytes\n",
                  ESP.getFreeHeap(),
                  ESP.getMinFreeHeap());
}

// --- Heap Logging Task ---
// A simple periodic task to monitor memory usage.
void taskLogHeap(void *pvParameters)
{
    for (;;)
    {
        log_heap_status();
        vTaskDelay(pdMS_TO_TICKS(15000)); // Log every 15 seconds
    }
}

// --- WiFi Event Handler ---
// This is an Interrupt Service Routine (ISR), so we must use ISR-safe FreeRTOS functions.
void WiFiEvent(WiFiEvent_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    NetworkEvent_t evt;

    switch (event)
    {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        evt = WIFI_EVENT_CONNECTED;
        xQueueSendFromISR(appContext.networkEventQueue, &evt, &xHigherPriorityTaskWoken);
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        evt = WIFI_EVENT_DISCONNECTED;
        xQueueSendFromISR(appContext.networkEventQueue, &evt, &xHigherPriorityTaskWoken);
        break;
    default:
        break;
    }
    // If a higher priority task was woken by the queue send, yield to it.
    if (xHigherPriorityTaskWoken)
    {
        portYIELD_FROM_ISR();
    }
}

void SNTPEvent(struct timeval *tv)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    NetworkEvent_t evt;
    evt = NetworkEvent_t::SNTP_SYNC;
    xQueueSendFromISR(appContext.networkEventQueue, &evt, &xHigherPriorityTaskWoken);
}