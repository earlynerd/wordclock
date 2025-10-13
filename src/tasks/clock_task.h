/**
 * @file clock_task.h
 * @brief Header for the Clock Update FreeRTOS task.
 */

#ifndef CLOCK_TASK_H
#define CLOCK_TASK_H

#include <Arduino.h>

/**
 * @brief The main function for the clock update task.
 * @param pvParameters A void pointer to the global AppContext struct.
 */
void taskClockUpdate(void *pvParameters);

#endif // CLOCK_TASK_H
