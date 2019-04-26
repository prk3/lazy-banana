#include <inttypes.h>

/**
 * Type used as state identifier.
 */
using StateIdentifier = uint8_t;

/**
 * State initializer = void function, takes no arguments.
 */
using StateInitializer = void (*)();

/**
 * Functor which allows changing state inside state handlers.
 */
class StateChanger {
    private:
        StateIdentifier state;
        bool changed: 1;
        bool forgetButtons: 1;

    public:
        StateChanger();
        bool wasChanged() const;
        bool wereButtonsForgotten() const;
        StateIdentifier newState() const;

        /**
         * You can request state change (and button reset) by calling StateChanger instance.
         */
        void operator()(StateIdentifier state, bool forgetButtons = true);
};

/**
 * State handler = void function, takes reference to StateChanger.
 */
using StateHandler = void (*)(StateChanger& changeState);

/**
 * Full state object.
 */
struct State {
    StateIdentifier id;
    StateInitializer initialize;
    StateHandler handle;
};
