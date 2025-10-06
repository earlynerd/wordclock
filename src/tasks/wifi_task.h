/**
 * @file wifi_task.h
 * @brief Header for the WiFi & NTP Sync FreeRTOS task.
 */

#ifndef WIFI_TASK_H
#define WIFI_TASK_H

#include <Arduino.h>

/**
 * @brief The main function for the WiFi management task.
 * @param pvParameters Pass `(void*)true` to indicate a manual sync request.
 */
void taskWiFi(void *pvParameters);

#endif // WIFI_TASK_H

