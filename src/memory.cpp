#include <memory.h>
#include <EEPROM.h>

///////////////////////////// public

void savePreset(uint8_t presetNumber, uint16_t height) {
    uint8_t b1 = height >> 8;
    uint8_t b2 = height;
    EEPROM.write(PRESET_ADDRESS_OFFSET + presetNumber * 2, b1);
    EEPROM.write(PRESET_ADDRESS_OFFSET + presetNumber * 2 + 1, b2);
}

uint16_t readPreset(uint8_t presetNumber) {
    uint8_t b1 = EEPROM.read(PRESET_ADDRESS_OFFSET + presetNumber * 2);
    uint8_t b2 = EEPROM.read(PRESET_ADDRESS_OFFSET + presetNumber * 2 + 1);
    return (b1 << 8) + b2;
}
