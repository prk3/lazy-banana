#include <Arduino.h>

/**
 * Shorting this pin to ground will wake up Arduino.
 */
const uint8_t WAKE_PIN = PIN2;

/**
 * Sets up wke pin.
 */
void setupSleep();

/**
 * Puts Arduino into sleep mode.
 */
void sleep();
