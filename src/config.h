/**
 * @file config.h
 * @brief Central configuration file for the Word Clock.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- EPD pins and configuration ---
#define EPD_BUSY    16
#define EPD_CS      17
#define EPD_DC      21
#define EPD_SCK     5
#define EPD_MOSI    18
#define EPD_MISO    19
#define EPD_RESET   -1
#define SRAM_CS     -1
#define EPD_SPI &SPI // primary SPI
#define FLEXIBLE_213
#define EPD_DEBUG



// --- LED Strip Configuration ---
#define DATA_PIN_WC    12
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    58
#define BRIGHTNESS  100 // Lowered for longevity and comfort

// --- Hardware Pins ---
#define BUTTON_1_PIN 14
#define BUTTON_2_PIN 15

// --- Button Timing Configuration (in milliseconds) ---
#define SHORT_PRESS_TIME 100
#define LONG_PRESS_TIME  1000
#define BUTTON_POLL_RATE_MS 20

// --- WiFi & Time Configuration ---
#define WIFI_PROV_SSID  "WordClock-Setup"
#define TIME_API_URL    "https://worldtimeapi.org/api/ip"
#define NTP_SERVER_1    "pool.ntp.org"
#define NTP_SERVER_2    "time.nist.gov"

// --- Non-Volatile Storage (NVS) Keys ---
// Used to save the timezone between reboots
#define NVS_NAMESPACE "word_clock"
#define NVS_TZ_KEY    "timezone"
\


#endif // CONFIG_H

