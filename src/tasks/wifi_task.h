/**
 * @file wifi_task.h
 * @brief Header for the WiFi & Time Sync and E-Paper Display FreeRTOS tasks.
 */

#ifndef WIFI_TASK_H
#define WIFI_TASK_H

#include <Arduino.h>

/**
 * @brief The main function for the WiFi management and time synchronization task.
 * @param pvParameters A void pointer to the global AppContext struct.
 */
void taskWiFi(void *pvParameters);

/**
 * @brief The main function for the E-Paper Display update task.
 * @param pvParameters A void pointer to the global AppContext struct.
 */
void task_epd(void *pvParameters);

#endif // WIFI_TASK_H
