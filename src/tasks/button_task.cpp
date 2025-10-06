/**
 * @file button_task.cpp
 * @brief Implements the FreeRTOS task for polling buttons.
 *
 * Button 1: Short/Long press cycles through color schemes.
 * Button 2: Long press now triggers a manual WiFi time sync.
 */

#include "button_task.h"
#include "clock_task.h"
#include "../config.h"

extern QueueHandle_t clockCommandQueue;

enum class ButtonFSM { IDLE, DEBOUNCING, PRESSED, RELEASED };

static void updateButton(int pin, ButtonFSM& state, unsigned long& pressTime, bool& isLong) {
    bool pressed = !digitalRead(pin);
    switch (state) {
        case ButtonFSM::IDLE:
            if (pressed) { state = ButtonFSM::DEBOUNCING; pressTime = millis(); }
            break;
        case ButtonFSM::DEBOUNCING:
            if (millis() - pressTime > 10) { state = pressed ? ButtonFSM::PRESSED : ButtonFSM::IDLE; isLong = false; }
            break;
        case ButtonFSM::PRESSED:
            if (!pressed) { state = ButtonFSM::RELEASED; }
            else if (!isLong && (millis() - pressTime > LONG_PRESS_TIME)) { isLong = true; }
            break;
        case ButtonFSM::RELEASED: break;
    }
}

void taskButtonCheck(void *pvParameters) {
    Serial.println("Button Task started.");

    ButtonFSM b1_fsm = ButtonFSM::IDLE, b2_fsm = ButtonFSM::IDLE;
    unsigned long b1_time = 0, b2_time = 0;
    bool b1_long = false, b2_long = false;

    for (;;) {
        updateButton(BUTTON_1_PIN, b1_fsm, b1_time, b1_long);
        updateButton(BUTTON_2_PIN, b2_fsm, b2_time, b2_long);

        ClockCommand cmd;
        bool cmd_sent = false;

        // B1 Release: Cycle color schemes
        if (b1_fsm == ButtonFSM::RELEASED) {
            cmd.type = b1_long ? CommandType::PREV_COLOR_SCHEME : CommandType::NEXT_COLOR_SCHEME;
            if(xQueueSend(clockCommandQueue, &cmd, 0) == pdPASS) cmd_sent = true;
            b1_fsm = ButtonFSM::IDLE;
        }

        // B2 Release: Short press is now unassigned, Long press forces sync
        if (b2_fsm == ButtonFSM::RELEASED) {
            if (b2_long) {
                cmd.type = CommandType::FORCE_WIFI_SYNC;
                if(xQueueSend(clockCommandQueue, &cmd, 0) == pdPASS) cmd_sent = true;
            } else {
                Serial.println("Button 2 Short Press: No action assigned.");
            }
            b2_fsm = ButtonFSM::IDLE;
        }

        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_RATE_MS));
    }
}

