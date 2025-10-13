/**
 * @file button_task.cpp
 * @brief Implements the FreeRTOS task for polling buttons.
 *
 * This task polls two buttons, debounces them, detects short and long presses,
 * and sends commands/events to the appropriate queues based on user input.
 * It accesses queues via the shared AppContext.
 */

#include "button_task.h"
#include "../AppContext.h"
#include "../config.h"

// FSM for button debouncing and long press detection
enum class ButtonFSM { IDLE, DEBOUNCING, PRESSED, RELEASED };

/**
 * @brief Updates a single button's state machine.
 * @param pin The GPIO pin for the button.
 * @param state Reference to the button's current FSM state.
 * @param pressTime Reference to the timestamp when the button was pressed.
 * @param isLong Reference to a flag indicating if a long press was detected.
 */
static void updateButton(int pin, ButtonFSM& state, unsigned long& pressTime, bool& isLong) {
    bool pressed = !digitalRead(pin); // Active low
    switch (state) {
        case ButtonFSM::IDLE:
            if (pressed) {
                state = ButtonFSM::DEBOUNCING;
                pressTime = millis();
            }
            break;
        case ButtonFSM::DEBOUNCING:
            if (millis() - pressTime > 10) { // 10ms debounce delay
                state = pressed ? ButtonFSM::PRESSED : ButtonFSM::IDLE;
                isLong = false;
            }
            break;
        case ButtonFSM::PRESSED:
            if (!pressed) {
                state = ButtonFSM::RELEASED;
            } else if (!isLong && (millis() - pressTime > LONG_PRESS_TIME)) {
                isLong = true;
                // A "long press active" event could be sent here if needed
            }
            break;
        case ButtonFSM::RELEASED:
            // This state is handled in the main loop and then reset to IDLE.
            break;
    }
}

void taskButtonCheck(void *pvParameters) {
    Serial.println("Button Task started.");
    // Cast the void pointer parameter back to the AppContext type
    auto* context = static_cast<AppContext*>(pvParameters);

    ButtonFSM b1_fsm = ButtonFSM::IDLE, b2_fsm = ButtonFSM::IDLE;
    unsigned long b1_time = 0, b2_time = 0;
    bool b1_long = false, b2_long = false;

    for (;;) {
        updateButton(BUTTON_1_PIN, b1_fsm, b1_time, b1_long);
        updateButton(BUTTON_2_PIN, b2_fsm, b2_time, b2_long);

        // --- Handle Button 1 (Color Scheme) ---
        if (b1_fsm == ButtonFSM::RELEASED) {
            SystemCommand cmd;
            cmd.type = b1_long ? SystemCommandType::PREV_COLOR_SCHEME : SystemCommandType::NEXT_COLOR_SCHEME;
            xQueueSend(context->systemCommandQueue, &cmd, 0);
            b1_fsm = ButtonFSM::IDLE; // Reset FSM after handling
        }

        // --- Handle Button 2 (Force WiFi Sync) ---
        if (b2_fsm == ButtonFSM::RELEASED) {
            if (b2_long) {
                // A long press on button 2 triggers a manual time sync.
                // We send a WIFI_BOOT event to the network task, which contains the logic
                // for connecting and syncing time. This is a clean way to reuse that logic.
                Serial.println("Button 2 Long Press: Forcing WiFi Sync...");
                NetworkEvent_t net_evt = NetworkEvent_t::WIFI_BOOT;
                xQueueSend(context->networkEventQueue, &net_evt, 0);
            } else {
                Serial.println("Button 2 Short Press: No action assigned.");
            }
            b2_fsm = ButtonFSM::IDLE; // Reset FSM after handling
        }

        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_RATE_MS));
    }
}
