/**
 * @file button_task.h
 * @brief Header for the Button Handling FreeRTOS task.
 */

#ifndef BUTTON_TASK_H
#define BUTTON_TASK_H

#include <Arduino.h>

// The main function for the button checking task.
void taskButtonCheck(void *pvParameters);

#endif // BUTTON_TASK_H
