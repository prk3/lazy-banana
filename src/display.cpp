#include "display.h"
#include <math.h>

///////////////////////////// private

TM1637Display display(CLK, DIO);


///////////////////////////// public

uint8_t displayBuffer[4] = {};

void setupDisplay() {
    display.setBrightness(BRIGHTNESS);
}

void clearDisplayBuffer() {
    displayBuffer[0] = EMPTY;
    displayBuffer[1] = EMPTY;
    displayBuffer[2] = EMPTY;
    displayBuffer[3] = EMPTY;
    setColonInDisplayBuffer(false);
}

void setColonInDisplayBuffer(boolean on) {
    if (on) {
        displayBuffer[1] = displayBuffer[1] | 0b10000000;
    } else {
        displayBuffer[1] = displayBuffer[1] & 0b01111111;
    }
}

void drawIntegerInDisplayBuffer(unsigned int number) {
    if (number > 9999)  {
        number = 9999;
    }

    uint8_t digit;
    digit = (number / 1000);
    displayBuffer[0] = (digit == 0) && (number < 1000) ? EMPTY : NUMBERS[digit];

    digit = (number / 100) % 10;
    displayBuffer[1] = (digit == 0) && (number < 100) ? EMPTY : NUMBERS[digit];

    digit = (number / 10) % 10;
    displayBuffer[2] = (digit == 0) && (number < 10) ? EMPTY : NUMBERS[digit];

    digit = (number) % 10;
    displayBuffer[3] = NUMBERS[digit];
}

void drawFloatInDisplayBuffer(float number) {
    number *= 100.0;
    int integer = int(round(number));

    if (integer > 9999) {
        integer = 9999;
    }

    uint8_t digit;
    digit = (integer / 1000);
    displayBuffer[0] = digit > 0 ? NUMBERS[digit] : EMPTY;

    digit = (integer / 100)  % 10;
    displayBuffer[1] = NUMBERS[digit];

    digit = (integer / 10) % 10;
    displayBuffer[2] = NUMBERS[digit];

    digit = (integer) % 10;
    displayBuffer[3] = NUMBERS[digit];

    setColonInDisplayBuffer(true);
}

void sendBufferToDisplay() {
    display.setSegments(displayBuffer, 4, 0);
}
