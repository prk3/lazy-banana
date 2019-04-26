#include "buttons.h"
#include <Arduino.h>

/////////////////////////////////////// private

Button buttons[3] = {
    Button(BTN_A_PIN),
    Button(BTN_B_PIN),
    Button(BTN_C_PIN),
};


////////////////////////////////////// public

const Button& BTN_A = buttons[0];
const Button& BTN_B = buttons[1];
const Button& BTN_C = buttons[2];

Button::Button(uint8_t pin):
    pin(pin), riseTime(0), oldPressed(false), newPressed(false), forgotten(false) {}

bool Button::isPressed() const {
    return this->newPressed;
}

bool Button::wasPressed() const {
    return !this->oldPressed && this->newPressed;
}

bool Button::wasUnpressed() const {
    return this->oldPressed && !this->newPressed;
}

bool Button::wasHeld(unsigned long ms) const {
    if (!isPressed()) {
        return false;
    }

    unsigned long now = millis();

    // overflow
    if (this->riseTime > now) {
        return now > ms; // held for at least ms
    }

    return now - this->riseTime > ms;
}

void Button::setup() {
    pinMode(this->pin, INPUT);
}

void Button::update() {
    bool pressRead = digitalRead(pin) > 0;

    if (this->forgotten) {
        if (!pressRead) {
            this->forgotten = false;
        }
    } else {
        this->oldPressed = this->newPressed;
        this->newPressed = pressRead;
        if (wasPressed()) {
            this->riseTime = millis();
        }
    }
}

void Button::forget() {
    this->forgotten = true;
    this->oldPressed = false;
    this->newPressed = false;
}

void setupButtons() {
    for (Button& button: buttons) {
        button.setup();
    }
}

void updateButtons() {
    for (Button& button: buttons) {
        button.update();
    }
}

void forgetButtons() {
    for (Button& button: buttons) {
        button.forget();
    }
}
