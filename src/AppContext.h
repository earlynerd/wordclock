/**
 * @file AppContext.h
 * @brief Defines a central context struct to hold shared application state and resources.
 *
 * This approach eliminates global variables, making dependencies explicit and the
 * application easier to manage and debug. An instance of this struct is created
 * in main.cpp and a pointer to it is passed to each FreeRTOS task.
 */
#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include <FastLED.h>
#include <RTClib.h>
#include <Adafruit_ThinkInk.h>
#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "config.h"

// --- Enum for commands sent to the Clock/Display task ---
enum class SystemCommandType {
    NEXT_COLOR_SCHEME,
    PREV_COLOR_SCHEME,
    SHOW_WIFI_ANIMATION,
    START_CLOCK_DISPLAY,
};

// --- Struct for system commands ---
struct SystemCommand {
    SystemCommandType type;
};

// --- Enum for events sent to the WiFi/Network task ---
typedef enum {
    WIFI_BOOT,              // Initial startup or manual sync request
    WIFI_EVENT_CONNECTED,
    WIFI_EVENT_DISCONNECTED,
    CLEAR_WIFI,             // Command to erase WiFi credentials
    SNTP_SYNC
} NetworkEvent_t;


// --- The main application context struct ---
struct AppContext {
    // Hardware Objects
    RTC_DS3231 rtc;
    CRGB leds[NUM_LEDS];
Adafruit_SSD1680 display;
 //Adafruit_SSD1675 display;
//Adafruit_SSD1675B display;
    //Adafruit_UC8151D display;

    //Adafruit_SSD1680 display;
    Preferences preferences;

    // RTOS Handles
    QueueHandle_t systemCommandQueue;
    QueueHandle_t networkEventQueue;
    QueueHandle_t epdQueue;

    // State Variables
    char time_zone[64] = "UTC";
    int colorSchemeIndex = 0;
    bool time_is_valid = false;
    uint32_t display_offset_x = 0;
    uint32_t display_offset_y = 16;
    const uint32_t maxiumum_offset = 16;
    // Constructor to initialize aggregated objects like the display
    //AppContext() : display(212, 104, EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY, EPD_SPI) {}
    AppContext() : display(250, 122, EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY, EPD_SPI) {}
};

#endif // APP_CONTEXT_H
