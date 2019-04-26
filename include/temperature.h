#pragma once
#include <inttypes.h>

/**
 * Temperature sensor pin.
 */
const uint8_t TEMP_PIN = 9;

/**
 * Dallas sensors have unique identifiers.
 * Replace TEMP_SENSOR_ADDRESS with your sensor's id or comment out this define.
 */
#define USE_TEMP_SENSOR_ADDRESS
#ifdef USE_TEMP_SENSOR_ADDRESS
    /**
     * Address of the temperature sensor.
     */
    const uint8_t TEMP_SENSOR_ADDRESS[8] = { 0x28, 0xff, 0x25, 0x55, 0x31, 0x18, 0x02, 0x68 };
#endif

/**
 * Defines precision of the temperature measurement.
 * (more bits - more precision - longer read)
 *
 * resolution  increment  time
 * --------------------------------
 *  9 bit      0.5℃       93.75 ms
 * 10 bit      0.25℃     187.5 ms
 * 11 bit      0.125℃    375 ms
 * 12 bit      0.0625℃   750 ms
 */
const uint8_t RESOLUTION = 12;

/**
 * Sets up temperature sensor.
 */
void setupTemperature();

/**
 * Reads temperature in ℃.
 */
float getTemperature();
