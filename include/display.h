#pragma once
#include <inttypes.h>
#include <Arduino.h>
#include <TM1637Display.h>

/**
 * CLK pin number.
 */
const uint8_t CLK = PIN5;

/**
 * DIO pi number.
 */
const uint8_t DIO = PIN4;

/**
 * Brightness of the display 0-7.
 */
const uint8_t BRIGHTNESS = 1;

/**
 * Display buffer holding 4 segments.
 */
extern uint8_t displayBuffer[4];

/**
 * Empty segment.
 */
const uint8_t EMPTY = 0;

/**
 * Segments representing digits 0-9.
 */
const uint8_t NUMBERS[10] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,         // 0
            SEG_B | SEG_C,                                 // 1
    SEG_A | SEG_B |         SEG_D | SEG_E |         SEG_G, // 2
    SEG_A | SEG_B | SEG_C | SEG_D |                 SEG_G, // 3
            SEG_B | SEG_C |                 SEG_F | SEG_G, // 4
    SEG_A |         SEG_C | SEG_D |         SEG_F | SEG_G, // 5
    SEG_A |         SEG_C | SEG_D | SEG_E | SEG_F | SEG_G, // 6
    SEG_A | SEG_B | SEG_C,                                 // 7
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G, // 8
    SEG_A | SEG_B | SEG_C | SEG_D |         SEG_F | SEG_G, // 9
};

/**
 * Letter-like segments.
 */
const uint8_t CHAR_A = SEG_A | SEG_B | SEG_C |         SEG_E | SEG_F | SEG_G;
const uint8_t CHAR_b =                 SEG_C | SEG_D | SEG_E | SEG_F | SEG_G;
const uint8_t CHAR_c =                         SEG_D | SEG_E |         SEG_G;
const uint8_t CHAR_C = SEG_A |                 SEG_D | SEG_E | SEG_F;
const uint8_t CHAR_d =         SEG_B | SEG_C | SEG_D | SEG_E |         SEG_G;
const uint8_t CHAR_E = SEG_A |                 SEG_D | SEG_E | SEG_F | SEG_G;
const uint8_t CHAR_F = SEG_A |                         SEG_E | SEG_F | SEG_G;
const uint8_t CHAR_G = SEG_A |         SEG_C | SEG_D | SEG_E | SEG_F;
const uint8_t CHAR_h =                 SEG_C |         SEG_E | SEG_F | SEG_G;
const uint8_t CHAR_H =         SEG_B | SEG_C |         SEG_E | SEG_F | SEG_G;
const uint8_t CHAR_J =         SEG_B | SEG_C | SEG_D;
const uint8_t CHAR_L =         SEG_B | SEG_C | SEG_D;
const uint8_t CHAR_o =                 SEG_C | SEG_D | SEG_E |         SEG_G;
const uint8_t CHAR_O = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;
const uint8_t CHAR_P = SEG_A | SEG_B |                 SEG_E | SEG_F | SEG_G;
const uint8_t CHAR_r =                                 SEG_E |         SEG_G;
const uint8_t CHAR_S = SEG_A |         SEG_C | SEG_D |         SEG_F | SEG_G;
const uint8_t CHAR_u =                 SEG_C | SEG_D | SEG_E;
const uint8_t CHAR_U =         SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;

/**
 * Additional symbol segments.
 */
const uint8_t CHAR_DEGREE = SEG_A | SEG_B | SEG_F | SEG_G;
const uint8_t CHAR_BARS   = SEG_A | SEG_D | SEG_G;

/**
 * Sets up 4-digit 7-segment display.
 */
void setupDisplay();

/**
 * Fills render buffer with empty characters.
 */
void clearDisplayBuffer();

/**
 * Controls ":" in display buffer.
 */
void setColonInDisplayBuffer(bool on);

/**
 * Draws a positive integer between 0 and 9999 in the display buffer.
 */
void drawIntegerInDisplayBuffer(unsigned int number);

/**
 * Draws a floating point number between 0 and 99.99 in the display buffer.
 */
void drawFloatInDisplayBuffer(float number);

/**
 * Sends what's in the display buffer to the display.
 */
void sendBufferToDisplay();
