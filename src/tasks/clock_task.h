/**
 * @file clock_task.h
 * @brief Header for the Clock Update FreeRTOS task.
 */

#ifndef CLOCK_TASK_H
#define CLOCK_TASK_H

#include <Arduino.h>

// Defines the types of commands the clock task can receive.
enum class CommandType {
    NEXT_COLOR_SCHEME,
    PREV_COLOR_SCHEME,
    FORCE_WIFI_SYNC, // Replaces timezone commands
    SHOW_WIFI_ANIMATION,
    START_CLOCK_DISPLAY,
};

// The structure for commands sent to the clock task queue.
struct ClockCommand {
    CommandType type;
};

// The main function for the clock update task.
void taskClockUpdate(void *pvParameters);

#endif // CLOCK_TASK_H

