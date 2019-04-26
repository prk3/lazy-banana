#include <sleep.h>
#include <avr/sleep.h>

//////////////////////////// private

void afterWake() {
    sleep_disable();
    detachInterrupt(digitalPinToInterrupt(WAKE_PIN));
}


//////////////////////////// public

void setupSleep() {
    pinMode(WAKE_PIN, INPUT_PULLUP);
}

void sleep() {
    sleep_enable();
    attachInterrupt(digitalPinToInterrupt(WAKE_PIN), afterWake, LOW);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();
    sleep_bod_disable();
    sei();
    sleep_cpu();
}
