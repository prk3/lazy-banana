#pragma once
#include <inttypes.h>

class Button {
    private:
        uint8_t pin;
        unsigned long riseTime;
        bool oldPressed : 1;
        bool newPressed : 1;
        bool forgotten : 1;

    public:
        /**
         * Construct a button, force contructor call.
         */
        explicit Button(uint8_t pin);

        /**
         * Is the button being held right now?
         */
        bool isPressed() const;

        /**
         * Has the button just been pressed?
         */
        bool wasPressed() const;

        /**
         * Has someone just let his finger off the button?
         */
        bool wasUnpressed() const;

        /**
         * Has the button been held for that long?
         */
        bool wasHeld(unsigned long ms = 1000) const;

        /**
         * Initialize the button.
         */
        void setup();

        /**
         * Read the state of the button and update it accordingly.
         */
        void update();

        /**
         * Pretend that the button is not pressed until it gets pressed again.
         * Super useful if you don't want to carry old actions to new state.
         */
        void forget();
};

// A B C pins

const uint8_t BTN_A_PIN = 8;
const uint8_t BTN_B_PIN = 7;
const uint8_t BTN_C_PIN = 6;

// const references to A B C buttons

extern const Button& BTN_A;
extern const Button& BTN_B;
extern const Button& BTN_C;

/**
 * Sets up all buttons.
 */
void setupButtons();

/**
 * Updates all buttons.
 */
void updateButtons();

/**
 * Forgets all buttons.
 */
void forgetButtons();
