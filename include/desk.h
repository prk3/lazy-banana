#pragma once
#include <Arduino.h>

/**
 * Pin for LIN transmitting.
 */
const int TX_PIN = PIN1;

/**
 * Pin for LIN receiving.
 */
const int RX_PIN = PIN0;

/**
 * User command in manual movement mode.
 */
enum class ManualCommand: uint8_t {
    NONE,
    UP,
    DOWN,
};

/**
 * Initializes the desk.
 */
void setupDesk();

/**
 * Returns desks height in mm.
 */
uint16_t getDeskHeight();

/**
 * Returns Bekant position value in internal units.
 */
uint16_t getDeskArbitraryHeight();

/**
 * Communicates with desk motors to handle up/down commands.
 * Call this function every 25-150ms to achieve smooth movement.
 */
void manualMoveLoop(ManualCommand command);

/**
 * Communicates with desk motors to reach certain height.
 * Call this function every 25-150ms to achieve smooth movement.
 */
void autoMoveLoop(uint16_t targetHeight);

/**
 * Whether desk has stopped.
 */
bool isDeskMoving();
