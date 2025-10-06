/**
 * @file wifi_task.cpp
 * @brief Implements the background task for WiFi and automatic time sync.
 *
 * Fetches timezone via HTTPS, saves it to NVS, syncs time via NTP, and
 * updates the hardware RTC with the correct UTC time. Now forces a system-wide
 * timezone update.
 */

#include "wifi_task.h"
#include "../config.h"
#include "../certs.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiProvisioner.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <RTClib.h>
#include "clock_task.h"
#include "time.h"
#include <Preferences.h>
#include <stdlib.h> // Required for setenv

// --- Externs for shared resources ---
extern RTC_DS3231 rtc;
extern QueueHandle_t clockCommandQueue;
extern TaskHandle_t clockTaskHandle;
extern char time_zone[50];
extern Preferences preferences;

static bool getTimezoneFromAPI(char* tz_buffer, size_t buffer_size) {
    WiFiClientSecure client;
    client.setCACert(root_ca_worldtimeapi);
    HTTPClient http;
    bool success = false;
    http.setTimeout(10000);
    Serial.println("[Time Sync] Attempting to fetch timezone from WorldTimeAPI...");

    if (http.begin(client, TIME_API_URL)) {
        http.setConnectTimeout(10000);
        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            JsonDocument doc;
            if (deserializeJson(doc, payload).code() == DeserializationError::Ok) {
                const char* tz = doc["timezone"];
                if (tz) {
                    strncpy(tz_buffer, tz, buffer_size - 1);
                    tz_buffer[buffer_size - 1] = '\0';
                    Serial.printf("[Time Sync] Successfully fetched timezone: %s\n", tz_buffer);
                    success = true;
                }
            }
        } else {
            Serial.printf("[Time Sync] HTTPS GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        Serial.printf("[Time Sync] HTTPS client failed to begin.\n");
    }
    return success;
}


void taskWiFi(void *pvParameters) {
    bool isManualSync = (pvParameters != NULL);
    Serial.printf("[WiFi Task] Started. Manual Sync: %s\n", isManualSync ? "Yes" : "No");

    if (!isManualSync) {
        WiFi.mode(WIFI_STA);
        WiFi.begin();
        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WiFi Task] Could not connect. Starting provisioning portal.");
        WiFiProvisioner provisioner;
        provisioner.startProvisioning();
    } 
    
    Serial.printf("[WiFi Task] Successfully connected to WiFi! IP: %s\n", WiFi.localIP().toString().c_str());

    ClockCommand cmd = {CommandType::SHOW_WIFI_ANIMATION};
    xQueueSend(clockCommandQueue, &cmd, 0);

    Serial.println("[WiFi Task] Suspending display task to free RAM for HTTPS.");
    vTaskSuspend(clockTaskHandle);
    vTaskDelay(pdMS_TO_TICKS(1000));

    bool tz_success = false;
    for (int i = 0; i < 115 && !tz_success; i++) {
        tz_success = getTimezoneFromAPI(time_zone, sizeof(time_zone));
        if (!tz_success) vTaskDelay(pdMS_TO_TICKS(2000));
    }

    Serial.println("[WiFi Task] Resuming display task.");
    vTaskResume(clockTaskHandle);

    if (tz_success) {
        preferences.putString(NVS_TZ_KEY, time_zone);
        Serial.printf("[WiFi Task] Timezone '%s' saved to NVS.\n", time_zone);

        Serial.printf("[WiFi Task] Configuring system time for timezone '%s' using NTP.\n", time_zone);
        configTzTime(time_zone, NTP_SERVER_1, NTP_SERVER_2);
        
        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 10000)) {
            Serial.println("[WiFi Task] System time synced via NTP.");
            
            // --- THIS IS THE FIX ---
            // Force the entire C library to adopt the new timezone setting.
            setenv("TZ", time_zone, 1);
            tzset();
            // ---------------------

            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%A, %B %d %Y %H:%M:%S %Z", &timeinfo);
            Serial.printf("[WiFi Task] The current local time is now: %s\n", time_buf);

            if (rtc.begin()) {
                time_t now_utc;
                time(&now_utc);
                rtc.adjust(DateTime(now_utc));
                Serial.println("[WiFi Task] RTC has been updated with correct UTC time.");
            }
        } else {
            Serial.println("[WiFi Task] Failed to sync time from NTP server.");
        }
    } else {
        Serial.println("[WiFi Task] Failed to get timezone after all retries.");
    }

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("[WiFi Task] WiFi disabled. Task finished.");
    vTaskDelete(NULL);
}

