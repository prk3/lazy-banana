#include "state.h"

///////////////////////////// public

StateChanger::StateChanger():
     state(), changed(false), forgetButtons(true) {}

void StateChanger::operator()(StateIdentifier state, bool forgetButtons) {
    this->changed = true;
    this->forgetButtons = forgetButtons;
    this->state = state;
}

bool StateChanger::wasChanged() const {
    return this->changed;
}

bool StateChanger::wereButtonsForgotten() const {
    return this->forgetButtons;
}

StateIdentifier StateChanger::newState() const {
    return this->state;
};
