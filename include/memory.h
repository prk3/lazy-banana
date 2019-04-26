#pragma once
#include <inttypes.h>

/**
 * The address of presets data in EEPROM.
 */
const uint16_t PRESET_ADDRESS_OFFSET = 0;

/**
 * Saves 16-bit height value into preset storage.
 */
void savePreset(uint8_t presetNumber, uint16_t height);

/**
 * Reads 16-bit height value from persistent storage.
 */
uint16_t readPreset(uint8_t presetNumber);
