/**
 * @file button_task.h
 * @brief Header for the Button Handling FreeRTOS task.
 */

#ifndef BUTTON_TASK_H
#define BUTTON_TASK_H

#include <Arduino.h>

/**
 * @brief The main function for the button checking task.
 * @param pvParameters A void pointer to the global AppContext struct.
 */
void taskButtonCheck(void *pvParameters);

#endif // BUTTON_TASK_H
