#include <Arduino.h>
#include "buttons.h"
#include "display.h"
#include "temperature.h"
#include "desk.h"
#include "memory.h"
#include "state.h"
#include "sleep.h"

/**
 * Time after which controller goes to sleep.
 */
const uint16_t SLEEP_AFTER_S = 15;

unsigned long lastActivityTime = 0;

/**
 * The list of all program states.
 * It's a classic enum, because we want to avoid type errors.
 * ("states.h" doesn't know what states we'll have in the app)
 */
enum S: StateIdentifier {
    MAIN,
    AUTO_MOVE,
    MANUAL_MOVE,
    PRESET_CHOICE,
    PRESET_SAVE,
    FACTORY_RESET,
    TEMPERATURE,
    SLEEP,
};


// Each state requring data persistant across updates has it's own object.

struct {
    uint8_t preset = 0;
    uint16_t targetHeight = 0;
    bool forceStop = false;
    uint8_t finishedCount = 0;
} autoMoveData;

struct {
    uint8_t frame;
} infoStateData;

struct {
    uint8_t preset = 0;
} presetSaveData;

/**
 * Here we define how each state is initialized and handled in loop.
 */
const State states[] = {
    State {
        S::MAIN,
        []() {
            clearDisplayBuffer();
            drawIntegerInDisplayBuffer(getDeskHeight());
            sendBufferToDisplay();
        },
        [](StateChanger& changeState) {
            if (BTN_A.wasUnpressed()) {
                autoMoveData.preset = 0;
                return changeState(S::AUTO_MOVE);
            }
            if (BTN_B.wasUnpressed()) {
                autoMoveData.preset = 1;
                return changeState(S::AUTO_MOVE);
            }
            if (BTN_C.wasUnpressed()) {
                autoMoveData.preset = 2;
                return changeState(S::AUTO_MOVE);
            }

            if (!BTN_A.isPressed() && (BTN_B.wasHeld() || BTN_C.wasHeld())) {
                return changeState(S::MANUAL_MOVE, false);
            }

            if (BTN_A.wasHeld()) {
                if (BTN_B.isPressed() && BTN_C.isPressed()) {
                    return changeState(S::FACTORY_RESET);
                }

                return changeState(S::TEMPERATURE);
            }
        },
    },
    State {
        S::AUTO_MOVE,
        []() {
            autoMoveData.targetHeight = readPreset(autoMoveData.preset);
            autoMoveData.forceStop = false;
            autoMoveData.finishedCount = 0;
        },
        [](StateChanger& changeState) {
            clearDisplayBuffer();
            drawIntegerInDisplayBuffer(getDeskHeight());
            setColonInDisplayBuffer(true);
            sendBufferToDisplay();

            if (BTN_A.wasPressed() || BTN_B.wasPressed() || BTN_C.wasPressed()) {
                autoMoveData.forceStop = true;
            }

            // stops motors like if a button on the original controller was unpressed
            if (autoMoveData.forceStop) {
                if (!isDeskMoving()) {
                    return changeState(S::MAIN);
                }
                manualMoveLoop(ManualCommand::NONE);
            } else {
                // exit when "finished" reported 3 times
                if (!isDeskMoving() && ++autoMoveData.finishedCount > 3) {
                    return changeState(S::MAIN);
                }
                autoMoveLoop(autoMoveData.targetHeight);
            }
        },
    },
    State {
        S::MANUAL_MOVE,
        []() {},
        [](StateChanger& changeState) {
            clearDisplayBuffer();
            drawIntegerInDisplayBuffer(getDeskHeight());
            setColonInDisplayBuffer(true);
            sendBufferToDisplay();

            // change state only if desk is not moving
            if (!isDeskMoving()) {
                if (BTN_A.wasUnpressed()) {
                    return changeState(S::MAIN);
                }

                if (BTN_A.wasHeld()) {
                    return changeState(S::PRESET_CHOICE);
                }
            }

            if (!BTN_A.isPressed() && BTN_B.isPressed()  && !BTN_C.isPressed()) {
                manualMoveLoop(ManualCommand::DOWN);
            }
            else if (!BTN_A.isPressed() && !BTN_B.isPressed() && BTN_C.isPressed()) {
                manualMoveLoop(ManualCommand::UP);
            }
            else {
                manualMoveLoop(ManualCommand::NONE);
            }
        },
    },
    State {
        S::PRESET_CHOICE,
        []() {
            clearDisplayBuffer();
            drawIntegerInDisplayBuffer(123);
            displayBuffer[0] = CHAR_BARS;
            sendBufferToDisplay();
        },
        [](StateChanger& changeState) {
            if (BTN_A.wasUnpressed()) {
                presetSaveData.preset = 0;
                return changeState(S::PRESET_SAVE);
            }
            if (BTN_B.wasUnpressed()) {
                presetSaveData.preset = 1;
                return changeState(S::PRESET_SAVE);
            }
            if (BTN_C.wasUnpressed()) {
                presetSaveData.preset = 2;
                return changeState(S::PRESET_SAVE);
            }

            if (BTN_A.wasHeld() || BTN_B.wasHeld() || BTN_C.wasHeld()) {
                return changeState(S::MANUAL_MOVE);
            }
        },
    },
    State {
        S::PRESET_SAVE,
        []() {
            infoStateData.frame = 0;

            savePreset(presetSaveData.preset, getDeskArbitraryHeight());

            clearDisplayBuffer();
            displayBuffer[presetSaveData.preset + 1] = NUMBERS[presetSaveData.preset + 1];
            sendBufferToDisplay();
        },
        [](StateChanger& changeState) {
            if (++infoStateData.frame > 30) {
                return changeState(S::MAIN);
            }
        },
    },
    State {
        S::FACTORY_RESET,
        []() {
            infoStateData.frame = 0;

            savePreset(0, 400);
            savePreset(1, 6000);
            savePreset(2, 7000);

            clearDisplayBuffer();
            displayBuffer[0] = EMPTY;
            displayBuffer[1] = CHAR_F;
            displayBuffer[2] = CHAR_A;
            displayBuffer[3] = CHAR_C;
            sendBufferToDisplay();
        },
        [](StateChanger& changeState) {
            if (++infoStateData.frame > 30) {
                return changeState(S::MAIN);
            }
        },
    },
    State {
        S::TEMPERATURE,
        []() {
            // clear display
            clearDisplayBuffer();
            sendBufferToDisplay();

            // read temperature, round to 0.1
            float temp10 = getTemperature() * 10.0;
            float tempRounded = round(temp10) / 10.0;

            // display temperature, put ° symbol on 0.01 place
            drawFloatInDisplayBuffer(tempRounded);
            displayBuffer[3] = CHAR_DEGREE;
            sendBufferToDisplay();
        },
        [](StateChanger& changeState) {
            if (BTN_A.wasPressed() || BTN_B.wasPressed() || BTN_C.wasPressed()) {
                return changeState(S::MAIN);
            }
        },
    },
    State {
        S::SLEEP,
        []() {
            clearDisplayBuffer();
            sendBufferToDisplay();
        },
        [](StateChanger& changeState) {
            sleep();
            lastActivityTime = millis();
            return changeState(S::MAIN);
        },
    },
};

/**
 * Returns pointer to state given state id.
 */
const State *findState(StateIdentifier id) {
    for (const State& state: states) {
        if (state.id == id) {
            return &state;
        }
    }
    return nullptr;
}

const State *currentState;


/**
 * Disable unused peripherals, initialize all modules, start the state machine.
 */
void setup() {
    // http://www.fiz-ix.com/2012/11/save-power-by-disabling-arduino-peripherals/
    // please comment out the following 3 lines if you need analog pins
    ADCSRA = ADCSRA & B01111111;
    ACSR = B10000000;
    DIDR0 = DIDR0 | B00111111;

    setupSleep();
    setupButtons();
    setupDisplay();
    setupTemperature();
    setupDesk();

    forgetButtons();
    currentState = findState(S::MAIN);
    currentState->initialize();
}

/**
 * Handle current state, change state if requested, read buttons, check sleep condition.
 * Repeat every 50ms ===FOREVER=== 😱.
 */
void loop() {

    StateChanger changer;
    currentState->handle(changer);
    delay(50);

    if (changer.wasChanged()) {
        const State* newState = findState(changer.newState());
        if (newState) {
            if (changer.wereButtonsForgotten()) {
                forgetButtons();
            }
            currentState = newState;
            currentState->initialize();
        }
    }

    updateButtons();

    // check if controller should go to sleep
    unsigned long now = millis();

    if (isDeskMoving() || BTN_A.isPressed() || BTN_B.isPressed() || BTN_C.isPressed()) {
        lastActivityTime = now;
    } else {
        if (lastActivityTime > now) {
            lastActivityTime = 0;
        }
        if (now - lastActivityTime >= SLEEP_AFTER_S * 1000UL) {
            const State *sleepState = findState(S::SLEEP);
            if (sleepState) {
                currentState = sleepState;
                currentState->initialize();
            }
        }
    }
}
