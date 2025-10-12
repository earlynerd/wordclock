/**
 * @file wifi_task.h
 * @brief Header for the WiFi & NTP Sync FreeRTOS task.
 */

#ifndef WIFI_TASK_H
#define WIFI_TASK_H

#include <Arduino.h>
#include <WiFi.h>
typedef enum {
    WIFI_BOOT,
    WIFI_EVENT_CONNECTED,
    WIFI_EVENT_DISCONNECTED,
    FORCE_WIFI_SYNC,
    CLEAR_WIFI,
} NetworkEvent_t;
/**
 * @brief The main function for the WiFi management task.
 * @param pvParameters Pass `(void*)true` to indicate a manual sync request.
 */
void taskWiFi(void *pvParameters);
void WiFiEvent(WiFiEvent_t event);
void task_epd(void *pvParameters);

#endif // WIFI_TASK_H

