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
#include <TzDbLookup.h>

// --- Externs for shared resources ---
extern RTC_DS3231 rtc;
extern QueueHandle_t clockCommandQueue;
extern TaskHandle_t clockTaskHandle;
extern char time_zone[64];
extern Preferences preferences;
const String useragent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36";


WiFiProvisioner::Config customCfg(
      WIFI_PROV_SSID,   // Access Point Name
      "Wi this Clock not Fi", // HTML Page Title
      "#0989d8",                     // Theme Color
      R"rawliteral(<svg id="Icons" xmlns="http://www.w3.org/2000/svg" width="5rem" height="5rem" viewBox="0 0 512 512"><defs><style>.cls-3{fill:#6b250c}.cls-4{fill:#7a2b13}.cls-5{fill:#dd8c29}.cls-6{fill:#efb732}.cls-7{fill:#962a11}.cls-8{fill:#f9eaa5}.cls-9{fill:#fad98f}.cls-10{fill:#f39e22}.cls-11{fill:#f9c744}.cls-12{fill:#c44a1a}.cls-13{fill:#decee5}.cls-14{fill:#f4e6f4}.cls-16{fill:#fff}.cls-17{fill:#2e3140}.cls-18{fill:#334353}.cls-20{fill:#ccfdff}</style></defs><circle cx="256" cy="256" r="255.98" style="fill:#0989d8"/><path d="M0 255.76A255.63 255.63 0 0 1 103.44 50.44c113.54-84.25 273.87-60.52 358.12 53a254.76 254.76 0 0 1 49.93 136 76.89 76.89 0 0 0-14.24 6.88c-23.84 14.74-42 41.9-69.91 44.44-19.15 1.75-37.29-9.24-51.1-22.62s-24.87-29.51-39.4-42.12a112.53 112.53 0 0 0-157.7 10c-14.17 15.89-26.53 37.44-47.68 40-28.75 3.44-51.22-31.63-80-28.95-9.3.87-17.72 5.66-26.73 8a54.5 54.5 0 0 1-24.73.69z" style="fill:#29a3fb"/><path class="cls-3" d="M269.55 125.29 220 211.15a12.09 12.09 0 0 1-16.51 4.43L168 195.09a12.1 12.1 0 0 1-4.43-16.52l49.58-85.86A16.57 16.57 0 0 1 232 85a8.47 8.47 0 0 0 9.37-3.53l24-36.76-.29-.17a3.66 3.66 0 0 1-.84-5.68l7.44-7.95a3.66 3.66 0 0 1 4.51-.68l16 9.25a3.68 3.68 0 0 1 1.68 4.25l-3.17 10.41a3.67 3.67 0 0 1-5.35 2.11l-.29-.16-19.9 39.17a8.43 8.43 0 0 0 1.63 9.88 16.57 16.57 0 0 1 2.76 20.15z"/><path class="cls-4" d="M269.55 125.29 220 211.15a12.09 12.09 0 0 1-16.51 4.43l-25.62-14.79a12.1 12.1 0 0 1-4.42-16.52l47.7-82.62a12.47 12.47 0 0 1 17-4.57 8.48 8.48 0 0 0 9.37-3.53l29.2-45.82a.14.14 0 0 0-.05-.19 3.8 3.8 0 0 1-.88-5.9l7-7.5 9.33 5.39a3.68 3.68 0 0 1 1.68 4.25l-3.17 10.41a3.67 3.67 0 0 1-5.35 2.11l-.29-.16-19.83 39.12a8.43 8.43 0 0 0 1.63 9.88 16.57 16.57 0 0 1 2.76 20.15z"/><path class="cls-5" d="m299.69 41.24-1 1.78a3 3 0 0 1-4.17 1.13l-23.55-13.6a3.06 3.06 0 0 1-1.12-4.18l1-1.78a3.06 3.06 0 0 1 4.15-1.12l23.55 13.59a3.06 3.06 0 0 1 1.14 4.18z"/><path class="cls-6" d="m299.69 41.24-1 1.78a3 3 0 0 1-4.17 1.13l-17.85-10.29a3.06 3.06 0 0 1-1.11-4.18l1-1.78a3.05 3.05 0 0 1 4.18-1.12l17.82 10.28a3.06 3.06 0 0 1 1.13 4.18z"/><ellipse class="cls-7" cx="258.65" cy="109.72" rx="6.07" ry="10.17" transform="rotate(-15 258.639 109.74)"/><ellipse class="cls-7" cx="258.89" cy="129.37" rx="6.99" ry="4.32" transform="rotate(-72 258.267 129.647)"/><path class="cls-7" d="M173.42 184.3h24.48a3.77 3.77 0 0 1 3.77 3.77 3.77 3.77 0 0 1-3.77 3.77h-24.48v-7.54z" transform="matrix(.87 .5 -.5 .87 119.16 -68.58)"/><rect class="cls-7" x="203.32" y="197.45" width="14" height="7.54" rx="3.77" transform="rotate(30 210.318 201.225)"/><path class="cls-7" d="M279.58 60.22a3.16 3.16 0 0 1 1.24 4.16l-10.21 20.14a3.16 3.16 0 0 1-4.4 1.3l-1.06-.61A3.15 3.15 0 0 1 264 80.9l11.27-19.53a3.15 3.15 0 0 1 4.31-1.15z"/><path class="cls-6" d="m257.26 146.59-26.45 45.81-56.43-32.58 26.45-45.81 13.08.14 24.3.27 19.05 32.17z"/><path class="cls-5" d="m213.91 114.15-29.66 51.37-9.87-5.7 26.45-45.81 13.08.14z"/><path class="cls-8" d="m251.16 157.15-11.22-6.47c1.49-8.87-2.12-17.82-9.81-22.26s-17.25-3.09-24.18 2.63l-11.22-6.47-8.64 15 11.22 6.42c-1.5 8.87 2.11 17.82 9.81 22.26s17.24 3.09 24.18-2.64l11.22 6.48z"/><path class="cls-9" transform="rotate(30 195.35 134.916)" d="M189.64 126.27h11.4v17.28h-11.4z"/><ellipse class="cls-10" cx="218.62" cy="148.35" rx="16.68" ry="15.35" transform="matrix(.5 -.87 .87 .5 -19.16 263.51)"/><ellipse class="cls-11" cx="218.62" cy="148.35" rx="12.45" ry="11.46" transform="rotate(-60 218.626 148.348)"/><rect class="cls-12" x="214.94" y="141.86" width="12.73" height="3.38" rx="1.69" transform="rotate(30 221.307 143.561)"/><rect class="cls-12" x="207.72" y="147.14" width="21.09" height="3.38" rx="1.69" transform="rotate(30 218.256 148.819)"/><path class="cls-3" d="m331.25 166.23-49.58 85.86a12.08 12.08 0 0 1-16.51 4.43L229.67 236a12.08 12.08 0 0 1-4.42-16.51l49.57-85.87a16.57 16.57 0 0 1 18.83-7.62 8.46 8.46 0 0 0 9.37-3.52l24-36.77-.3-.17a3.67 3.67 0 0 1-.84-5.68l7.44-8a3.68 3.68 0 0 1 4.51-.68l16 9.25a3.68 3.68 0 0 1 1.68 4.25l-3.17 10.42a3.67 3.67 0 0 1-5.34 2.13l-.29-.17-19.86 39.14a8.42 8.42 0 0 0-.8 5.21 8.51 8.51 0 0 0 2.43 4.67 16.58 16.58 0 0 1 2.77 20.15z"/><path class="cls-4" d="m331.25 166.23-49.58 85.86a12.08 12.08 0 0 1-16.51 4.43l-25.62-14.79a12.08 12.08 0 0 1-4.42-16.51l47.7-82.63a12.47 12.47 0 0 1 17-4.57 8.45 8.45 0 0 0 9.37-3.52l29.2-45.83a.16.16 0 0 0-.06-.2 3.8 3.8 0 0 1-.88-5.9l7-7.49 9.33 5.38a3.68 3.68 0 0 1 1.68 4.25l-3.17 10.42a3.67 3.67 0 0 1-5.29 2.1l-.29-.17-19.86 39.14a8.42 8.42 0 0 0-.8 5.21 8.51 8.51 0 0 0 2.43 4.67 16.58 16.58 0 0 1 2.77 20.15z"/><rect class="cls-5" x="342.36" y="58.09" width="8.18" height="33.31" rx="3.06" transform="rotate(-60 346.442 74.745)"/><path class="cls-6" d="m361.38 82.19-1 1.77a3.06 3.06 0 0 1-4.18 1.13L338.36 74.8a3.05 3.05 0 0 1-1.11-4.18l1-1.78a3.07 3.07 0 0 1 4.18-1.12L360.27 78a3.05 3.05 0 0 1 1.11 4.19z"/><ellipse class="cls-7" cx="320.34" cy="150.67" rx="6.07" ry="10.17" transform="rotate(-15 320.325 150.664)"/><ellipse class="cls-7" cx="320.58" cy="170.32" rx="6.99" ry="4.32" transform="rotate(-71.78 320.556 170.318)"/><path class="cls-7" d="M235.11 225.24h24.48a3.77 3.77 0 0 1 3.77 3.77 3.77 3.77 0 0 1-3.77 3.77h-24.48v-7.54z" transform="rotate(30 249.244 229.015)"/><rect class="cls-7" x="265.01" y="238.39" width="14" height="7.54" rx="3.77" transform="rotate(30 272.006 242.156)"/><path class="cls-7" d="M341.27 101.16a3.16 3.16 0 0 1 1.24 4.16l-10.21 20.14a3.16 3.16 0 0 1-4.39 1.31l-1.07-.62a3.15 3.15 0 0 1-1.15-4.31L337 102.32a3.16 3.16 0 0 1 4.27-1.16z"/><path class="cls-6" d="m318.95 187.53-26.45 45.81-56.43-32.58 26.45-45.81 13.09.15 24.29.26 19.05 32.17z"/><path class="cls-5" d="m275.61 155.1-29.66 51.37-9.88-5.71 26.45-45.81 13.09.15z"/><path class="cls-8" d="m312.85 198.1-11.22-6.48c1.49-8.87-2.12-17.82-9.81-22.26s-17.25-3.09-24.18 2.64l-11.22-6.48-8.64 15L259 187c-1.49 8.87 2.11 17.82 9.81 22.26s17.24 3.09 24.18-2.63l11.22 6.47z"/><path class="cls-9" transform="rotate(30 257.038 175.847)" d="M251.34 167.21h11.4v17.28h-11.4z"/><ellipse class="cls-10" cx="280.31" cy="189.29" rx="16.68" ry="15.35" transform="rotate(-60 280.312 189.285)"/><ellipse class="cls-11" cx="280.31" cy="189.29" rx="12.45" ry="11.46" transform="rotate(-60 280.312 189.285)"/><rect class="cls-12" x="276.63" y="182.81" width="12.73" height="3.38" rx="1.69" transform="rotate(30 282.994 184.492)"/><rect class="cls-12" x="269.41" y="188.08" width="21.09" height="3.38" rx="1.69" transform="rotate(30 279.948 189.768)"/><path class="cls-13" d="m359.48 197.68-2.72 26.6-18.12 177.3a36.58 36.58 0 0 1-36.39 32.85H193.73a36.59 36.59 0 0 1-36.4-32.85l-18.11-177.3-2.72-26.6z"/><path class="cls-14" d="m359.48 197.68-20.84 203.9a36.58 36.58 0 0 1-36.39 32.85h-79.79a36.58 36.58 0 0 1-36.4-32.85l-20.84-203.9z"/><path style="fill:#c2aacf" d="m359.48 197.68-2.72 26.6H139.22l-2.72-26.6h222.98z"/><ellipse class="cls-16" cx="314.01" cy="272.52" rx="32.08" ry="17.91" transform="matrix(.09 -1 1 .09 14.31 560.71)"/><ellipse class="cls-16" cx="307.59" cy="339.61" rx="24.54" ry="11.56" transform="rotate(-84.83 307.6 339.614)"/><path class="cls-13" d="M376.11 185.16a12.53 12.53 0 0 1-12.52 12.52H132.4a12.53 12.53 0 1 1 0-25.05h231.19a12.52 12.52 0 0 1 12.52 12.53z"/><path class="cls-14" d="M276.72 178.89a6.26 6.26 0 0 1-6.26 6.26H154.35a6.26 6.26 0 0 1-6.26-6.26 6.26 6.26 0 0 1 6.26-6.26h116.11a6.26 6.26 0 0 1 6.26 6.26zM343.72 178.89a6.26 6.26 0 0 1-6.26 6.26h-41.62a6.26 6.26 0 0 1-6.26-6.26 6.26 6.26 0 0 1 6.26-6.26h41.62a6.26 6.26 0 0 1 6.26 6.26z"/><path class="cls-17" d="M71.9 355.78a15.29 15.29 0 0 0 2.64 3.52c3.56 3.57 8.12 4.77 12.68 4.77 7.36 0 14.74-3.12 18-4.5 10.7-4.51 24.18-12.49 40-23.72 1.65-1.16 3.32-2.36 5-3.58 29.14-21.17 63.42-50.87 97.74-84.68q3.21-3.15 6.41-6.36l-12.56-12.56-.27-.27c-32.27 32.26-65.12 61.36-93.47 82.85q-4 3-7.79 5.81C106 342.1 92.17 345.52 88 345.88a32.06 32.06 0 0 1 2.32-8.47c3.12-8 10.28-21.62 26.51-43.87 7.63-10.46 16.41-21.61 26.1-33.23l-2.55-25-4.94 5.73C121 257.88 108.34 274 98 288.59c-11.22 15.87-19.2 29.34-23.72 40-1.98 4.71-7.51 17.79-2.38 27.19z"/><path class="cls-18" d="M71.9 355.78a18.81 18.81 0 0 0 9.17 2.14c7.36 0 14.74-3.12 18-4.5 10.69-4.51 24.17-12.48 40-23.71 1.64-1.17 3.31-2.36 5-3.59 29.14-21.17 63.42-50.87 97.74-84.67q3.21-3.16 6.41-6.37l-6.41-6.41-.27-.27c-32.27 32.26-65.12 61.36-93.47 82.85q-4 3-7.79 5.81C106 342.1 92.17 345.52 88 345.88a32.06 32.06 0 0 1 2.32-8.47h-.13a4.69 4.69 0 0 1-6.05-6.08c3.09-8 10.24-21.63 26.55-44C116 280 122 272.23 128.5 264.21a31.69 31.69 0 0 0 6.9-23.16C121 257.88 108.34 274 98 288.59c-11.22 15.87-19.2 29.34-23.72 40-1.98 4.71-7.51 17.79-2.38 27.19z"/><path class="cls-18" d="M229.1 228.4v12.83a12.48 12.48 0 0 0 12.47 12.47h12.83a12.48 12.48 0 0 0 12.47-12.47v-13.9a12.48 12.48 0 0 0-12.43-11.4h-12.87a12.47 12.47 0 0 0-12.42 11.4 9.47 9.47 0 0 0-.05 1.07z"/><path d="M229.15 227.33a12.47 12.47 0 0 0 12.42 11.39h12.83a11.44 11.44 0 1 0 0-22.79h-12.83a12.47 12.47 0 0 0-12.42 11.4z" style="fill:#3e4e60"/><path class="cls-17" d="M237.45 231.24v7.15a7 7 0 0 0 7 7h7.15a7 7 0 0 0 7-7v-7.15a5.91 5.91 0 0 0 0-.6 7 7 0 0 0-6.94-6.36h-7.15a7 7 0 0 0-6.93 6.36 5.88 5.88 0 0 0-.13.6z"/><path class="cls-20" d="M194.39 155h19.1a7 7 0 0 1 7 7v10.59h-33.13V162a7 7 0 0 1 7.03-7z"/><path class="cls-16" d="m142.08 172.63 20-20a10.53 10.53 0 0 1 14.9 0l20 20zM212.41 172.63l15.26-15.26a8 8 0 0 1 11.36 0l15.26 15.26z"/><path class="cls-20" d="M283.8 149.51h33a8.73 8.73 0 0 1 8.73 8.73v14.39h-50.47v-14.39a8.73 8.73 0 0 1 8.74-8.73z"/><path class="cls-16" d="m314 172.63 15.27-15.26a8 8 0 0 1 11.35 0l15.27 15.26z"/></svg>)rawliteral", // SVG Logo
      "Custom Provisioner", // Project Title
      "Custom Setup",       // Project Sub-title
      "Lets give up the WiFi password"
      "So that the Clock can go on the internet",                             // Project Information
      "All rights reserved © me", // Footer
                                                      // Text
      "U did it", // Success Message
      "This action will erase all stored settings, including " "API key.", // Reset Confirmation Text
      "API Key",  // Input Field Text
      4,          // Input Field Length
      false,       // Show Input Field
      true        // Show Reset Field
  );
WiFiProvisioner provisioner(customCfg);

static bool getTimezoneFromAPI(char *tz_buffer, size_t buffer_size)
{
    WiFiClientSecure client;
    client.setCACert(root_ca_worldtimeapi);
    HTTPClient http;
    http.setUserAgent(useragent);
    bool success = false;
    http.setTimeout(10000);
    Serial.println("[Time Sync] Attempting to fetch timezone from WorldTimeAPI...");

    if (http.begin(client, TIME_API_URL))
    {
        http.setConnectTimeout(8000);
        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK)
        {
            if (http.getSize() < 4096)      //sanity check protects from hostile payloads
            {
                String payload = http.getString();
                JsonDocument doc;
                if (deserializeJson(doc, payload).code() == DeserializationError::Ok)
                {
                    const char *tz = doc["timezone"];
                    if (tz)
                    {
                        char ianaTimeZone[64];
                        strcpy(ianaTimeZone, tz);
                        char posixTimeZone[64];
                        strcpy( posixTimeZone, TzDbLookup::getPosix(ianaTimeZone));
                        strcpy(tz_buffer,posixTimeZone);
                        //tz_buffer[buffer_size - 1] = '\0';
                        Serial.printf("[Time Sync] Successfully fetched timezone: %s\n", tz_buffer);
                        success = true;
                    }
                    const char *tme = doc["datetime"];
                    if (tme)
                    {

                    }
                }
            }
            
        }
        else
        {
            Serial.printf("[Time Sync] HTTPS GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
    else
    {
        Serial.printf("[Time Sync] HTTPS client failed to begin.\n");
    }
    return success;
}

void taskWiFi(void *pvParameters)
{
    bool isManualSync = (pvParameters != NULL);
    Serial.printf("[WiFi Task] Started. Manual Sync: %s\n", isManualSync ? "Yes" : "No");

    if (!isManualSync)
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin();
        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 10000)
        {
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[WiFi Task] Could not connect. Starting provisioning portal.");
        
        provisioner.startProvisioning();
    }

    Serial.printf("[WiFi Task] Successfully connected to WiFi! IP: %s\n", WiFi.localIP().toString().c_str());

    ClockCommand cmd = {CommandType::SHOW_WIFI_ANIMATION};
    xQueueSend(clockCommandQueue, &cmd, 0);

    //Serial.println("[WiFi Task] Suspending display task to free RAM for HTTPS.");
    //vTaskSuspend(clockTaskHandle);
    //vTaskDelay(pdMS_TO_TICKS(1000));

    bool tz_success = false;
    for (int i = 0; i < 115 && !tz_success; i++)
    {
        tz_success = getTimezoneFromAPI(time_zone, sizeof(time_zone));
        if (!tz_success)
            vTaskDelay(pdMS_TO_TICKS(2000));
    }

    //Serial.println("[WiFi Task] Resuming display task.");
    //vTaskResume(clockTaskHandle);

    if (tz_success)
    {
        preferences.putString(NVS_TZ_KEY, time_zone);
        Serial.printf("[WiFi Task] Timezone '%s' saved to NVS.\n", time_zone);

        Serial.printf("[WiFi Task] Configuring system time for timezone '%s' using NTP.\n", time_zone);
        configTzTime(time_zone, NTP_SERVER_1, NTP_SERVER_2);

        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 10000))
        { // This waits for NTP and sets the system clock to UTC.
            Serial.println("[WiFi Task] System time synced via NTP.");
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%A, %B %d %Y %H:%M:%S %Z", &timeinfo);
            // Force the entire C library for THIS task to adopt the new timezone setting.
            //setenv("TZ", time_zone, 1);
            ///tzset();

            // Now that the timezone is set, get the time again to correctly convert it for logging.
            time_t now;
            time(&now);
            localtime_r(&now, &timeinfo);

            
            
            Serial.printf("[WiFi Task] The current local time is now: %s\n", time_buf);

            if (rtc.begin())
            {
                // 'now' is already the correct UTC epoch time, which is what the RTC needs.
                rtc.adjust(DateTime(now));
                Serial.println("[WiFi Task] RTC has been updated with correct UTC time.");
            }
        }
        else
        {
            Serial.println("[WiFi Task] Failed to sync time from NTP server.");
        }
    }
    else
    {
        Serial.println("[WiFi Task] Failed to get timezone after all retries.");
    }

    //WiFi.disconnect(true);
    //WiFi.mode(WIFI_OFF);
    //Serial.println("[WiFi Task] WiFi disabled. Task finished.");
    //vTaskDelete(NULL);
    vTaskSuspend(NULL);
}
