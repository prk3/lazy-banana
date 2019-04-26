#include "desk.h"
#include "lin.h"

// this file is a big cherry pick of gcormier/megadesk/Code/megadesk.ino
// the code has been reformated, refactored and cut down to only handle desk movement
// check out lib/gcormier_megadesk/LICENSE


/////////////////////////////////////////// private

#define FINE_MOVEMENT_VALUE   100 // Based on protocol decoding

#define LIN_CMD_IDLE          252
#define LIN_CMD_RAISE         134
#define LIN_CMD_LOWER         133
#define LIN_CMD_FINE          135
#define LIN_CMD_FINISH        132
#define LIN_CMD_PREMOVE       196

#define LIN_MOTOR_BUSY        2
#define LIN_MOTOR_BUSY_FINE   3

// idle code, different value for different desk variants
// smaller Bekant variant: 96
// bigger Bekant variant: 0
const uint8_t LIN_MOTOR_IDLE = 96;

// values of 4 constants below depend on moveLoop calling frequency and load on your desk
// please adjust these values if you experience auto move overshooting/undershotting

// distance the desk is expected to travel between moveLoop function calls when moving UP
// in my case it's between 42 and 45
const uint16_t DISTANCE_UP_PER_UPDATE = 43;

// distance the desk is expected to travel between moveLoop function calls when moving DOWN
// in my case between 39 and 42
const uint16_t DISTANCE_DOWN_PER_UPDATE = 40;

// distance the desk travels after unpressing UP button, in Bekant's height units
// in my case between 78 and 84
const uint16_t BRAKING_UP_DISTANCE = 81;

// distance the desk travels after unpressing DOWN button, in Bekant's height units
// in my case between 87 and 93
const uint16_t BRAKING_DOWN_DISTANCE = 90;


// Changing these might be a really bad idea. They are sourced from
// decoding the OEM controller limits. If you really need a bit of extra travel
// you can fiddle with SAFETY, it's an extra buffer of a few units.
const uint16_t SAFETY = 20;
const uint16_t DANGER_MAX_HEIGHT = 6777 - SAFETY;
const uint16_t DANGER_MIN_HEIGHT = 162 + SAFETY;

enum class DeskState: uint8_t {
    OFF,
    STARTING,
    UP,
    DOWN,
    STOPPING1,
    STOPPING2,
    STOPPING3,
    STOPPING4,
};

enum class MoveDirection: uint8_t {
    UP,
    DOWN,
    NONE,
};

struct MoveLoopData {
    uint16_t enc_a;
    uint16_t enc_b;
    uint16_t enc_target;
};

struct DeskData {
    DeskState state;
    MoveDirection direction;
    uint16_t height;
    bool motorsIdling;
};

DeskData deskData {
    DeskState::OFF,
    MoveDirection::NONE,
    DANGER_MIN_HEIGHT,
    true,
};

Lin lin(Serial, TX_PIN);

unsigned long t = 0;

void delay_until(unsigned long ms) {
    unsigned long end = t + (1000 * ms);
    unsigned long d = end - micros();

    // crazy long delay; probably negative wrap-around
    // just return
    if (d > 1000000) {
        t = micros();
        return;
    }

    if (d > 15000) {
        unsigned long d2 = (d-15000) / 1000;
        delay(d2);
        d = end - micros();
    }
    delayMicroseconds(d);
    t = end;
}

template <typename T>
T difference(T a, T b) {
    return a > b ? a - b : b - a;
}

// Really weird startup sequence, sourced from the controller.
void linInit() {
    uint8_t resp[8];

    auto sendInitPacket = [&resp](uint8_t a1 = 255, uint8_t a2 = 255, uint8_t a3 = 255, uint8_t a4 = 255) {
        uint8_t packet[8] = { a1, a2, a3, a4, 255, 255, 255, 255 };

        // Custom checksum formula for the initialization
        int chksum = a1 + a2 + a3 + a4;
        chksum = chksum % 255;
        chksum = 255 - chksum;

        lin.send(60, packet, 8, 2, chksum);
        delay(3);
        return lin.recv(61, resp, 8, 2);
    };

    // Brief stabilization delay
    delay(150);

    sendInitPacket(255, 7);
    sendInitPacket(255, 7);
    sendInitPacket(255, 1, 7);
    sendInitPacket(208, 2, 7);

    uint8_t initA;
    for (initA = 0; sendInitPacket(initA, 2, 7) == 0; initA++) {}

    sendInitPacket(initA, 6,  9, 0);
    sendInitPacket(initA, 6, 12, 0);
    sendInitPacket(initA, 6, 13, 0);
    sendInitPacket(initA, 6, 10, 0);
    sendInitPacket(initA, 6, 11, 0);
    sendInitPacket(initA, 4,  0, 0);

    uint8_t initB;
    for (initB = initA + 1; sendInitPacket(initB, 2, 0, 0) == 0; initB++) {}

    sendInitPacket(initB, 6,  9, 0);
    sendInitPacket(initB, 6, 12, 0);
    sendInitPacket(initB, 6, 13, 0);
    sendInitPacket(initB, 6, 10, 0);
    sendInitPacket(initB, 6, 11, 0);
    sendInitPacket(initB, 4,  1, 0);

    for (uint8_t initC = initB + 1; initC < 8; initC++) {
        sendInitPacket(initC, 2, 1, 0);
    }

    sendInitPacket(208, 1, 7, 0);
    sendInitPacket(208, 2, 7, 0);

    delay(15);

    uint8_t magicPacket[3] = { 246, 255, 191 };
    lin.send(18, magicPacket, 3, 2);

    delay(5);
}

void readDeskData(MoveLoopData &loopData) {
    uint8_t empty[3] = { 0, 0, 0 };
    uint8_t node_a[4] = { 0, 0, 0, 0 };
    uint8_t node_b[4] = { 0, 0, 0, 0 };

    // Send PID 17
    lin.send(17, empty, 3, 2);
    delay_until(5);

    // Recv from PID 09
    lin.recv(9, node_b, 3, 2);
    delay_until(5);

    // Recv from PID 08
    lin.recv(8, node_a, 3, 2);
    delay_until(5);

    // Send PID 16, 6 times
    for (uint8_t i = 0; i < 6; i++) {
        lin.send(16, 0, 0, 2);
        delay_until(5);
    }

    // Send PID 1
    lin.send(1, 0, 0, 2);
    delay_until(5);

    loopData.enc_a = node_a[0] | (node_a[1] << 8);
    loopData.enc_b = node_b[0] | (node_b[1] << 8);
    loopData.enc_target = loopData.enc_a;

    deskData.height = loopData.enc_a;
    deskData.motorsIdling = node_a[2] == LIN_MOTOR_IDLE && node_b[2] == LIN_MOTOR_IDLE;
}

void updateStateAndDirection(ManualCommand command) {

    switch (deskData.state) {
        case DeskState::OFF: {
            if (command != ManualCommand::NONE && deskData.motorsIdling) {

                // final height if we let one more loop of UP/DOWN movement
                uint16_t oneLoopStopHeight = command == ManualCommand::UP
                    ? deskData.height + DISTANCE_UP_PER_UPDATE   + BRAKING_UP_DISTANCE
                    : deskData.height - DISTANCE_DOWN_PER_UPDATE - BRAKING_DOWN_DISTANCE;

                if (command == ManualCommand::UP && oneLoopStopHeight < DANGER_MAX_HEIGHT) {
                    deskData.state = DeskState::STARTING;
                    deskData.direction = MoveDirection::UP;
                }
                else if (command == ManualCommand::DOWN && oneLoopStopHeight > DANGER_MIN_HEIGHT) {
                    deskData.state = DeskState::STARTING;
                    deskData.direction = MoveDirection::DOWN;
                }
            }
            break;
        }
        case DeskState::STARTING: {
            switch (deskData.direction) {
                case MoveDirection::UP:
                    deskData.state = DeskState::UP;
                    break;
                case MoveDirection::DOWN:
                    deskData.state = DeskState::DOWN;
                    break;
                case MoveDirection::NONE:
                    // this should never happen, do some generic shit
                    deskData.state = DeskState::STOPPING4;
                    break;
            }
            break;
        }
        case DeskState::UP: {
            // final height if we let one more loop of UP
            uint16_t oneLoopStopHeight = deskData.height + DISTANCE_UP_PER_UPDATE + BRAKING_UP_DISTANCE;

            if (command != ManualCommand::UP || oneLoopStopHeight >= DANGER_MAX_HEIGHT) {
                deskData.state = DeskState::STOPPING1;
            }
            break;
        }
        case DeskState::DOWN: {
            // final height if we let one more loop of DOWN
            uint16_t oneLoopStopHeight = deskData.height - DISTANCE_DOWN_PER_UPDATE + BRAKING_DOWN_DISTANCE;

            if (command != ManualCommand::DOWN || oneLoopStopHeight <= DANGER_MIN_HEIGHT) {
                deskData.state = DeskState::STOPPING1;
            }
            break;
        }
        case DeskState::STOPPING1: {
            deskData.state = DeskState::STOPPING2;
            break;
        }
        case DeskState::STOPPING2: {
            deskData.state = DeskState::STOPPING3;
            break;
        }
        case DeskState::STOPPING3: {
            deskData.state = DeskState::STOPPING4;
            break;
        }
        case DeskState::STOPPING4: {
            if (deskData.motorsIdling) {
                deskData.state = DeskState::OFF;
                deskData.direction = MoveDirection::NONE;
            }
            break;
        }
    }
}

void sendAppropriateCommand(const MoveLoopData &loopData) {
    uint8_t cmd[3] = { 0, 0, 0 };
    uint16_t enc_target = 0;

    // Send PID 18
    switch (deskData.state) {
        case DeskState::OFF:
            cmd[2] = LIN_CMD_IDLE;
            break;

        case DeskState::STARTING:
            cmd[2] = LIN_CMD_PREMOVE;
            break;

        case DeskState::UP:
            enc_target = min(loopData.enc_a, loopData.enc_b);
            cmd[2] = LIN_CMD_RAISE;
            break;

        case DeskState::DOWN:
            enc_target = max(loopData.enc_a, loopData.enc_b);
            cmd[2] = LIN_CMD_LOWER;
            break;

        case DeskState::STOPPING1:
        case DeskState::STOPPING2:
        case DeskState::STOPPING3:
            enc_target = min(loopData.enc_a, loopData.enc_b);

            if (deskData.direction == MoveDirection::UP) {
                enc_target += FINE_MOVEMENT_VALUE;
            }
            else if (deskData.direction == MoveDirection::DOWN) {
                enc_target -= FINE_MOVEMENT_VALUE;
            }

            cmd[2] = LIN_CMD_FINE;
            break;

        case DeskState::STOPPING4:
            enc_target = max(loopData.enc_a, loopData.enc_b);
            cmd[2] = LIN_CMD_FINISH;
            break;
    }

    cmd[0] = enc_target & 0xFF;
    cmd[1] = enc_target >> 8;
    lin.send(18, cmd, 3, 2);
}

////////////////////////////////////// public

void setupDesk() {
    lin.begin(19200);
    linInit();

    delay(150);
    manualMoveLoop(ManualCommand::NONE); // to read height
}

uint16_t getDeskHeight() {
    // min desk height in mm + position value offset to 0 for min height * arbitrary unit to mm ratio
    // adjust first value to your desk's minimum heigh
    return 662 + (deskData.height - 120) * 0.09837;
}

uint16_t getDeskArbitraryHeight() {
    return deskData.height;
}

void manualMoveLoop(ManualCommand user_cmd) {
    MoveLoopData loopData;
    readDeskData(loopData);
    updateStateAndDirection(user_cmd);
    sendAppropriateCommand(loopData);
}

void autoMoveLoop(uint16_t targetHeight) {
    MoveLoopData loopData;
    readDeskData(loopData);
    ManualCommand command = ManualCommand::NONE;

    // we are below target height
    if (deskData.height < targetHeight) {
        uint16_t stopHeight = deskData.height + BRAKING_UP_DISTANCE; // final height if we stop now
        uint16_t oneLoopStopHeight = deskData.height + DISTANCE_UP_PER_UPDATE + BRAKING_UP_DISTANCE; // final height if we stop later

        // if one more loop will bring us closer to target position
        if (difference(oneLoopStopHeight, targetHeight) < difference(stopHeight, targetHeight)) {
            command = ManualCommand::UP;
        }
    }

    // we are above target height
    else if (deskData.height > targetHeight) {
        uint16_t stopHeight = deskData.height - BRAKING_DOWN_DISTANCE; // final height if we stop now
        uint16_t oneLoopStopHeight = deskData.height - DISTANCE_DOWN_PER_UPDATE - BRAKING_DOWN_DISTANCE; // final height if we stop later

        // if one more loop will bring us closer to target position
        if (difference(oneLoopStopHeight, targetHeight) < difference(stopHeight, targetHeight)) {
            command = ManualCommand::DOWN;
        }
    }

    // this function will ignore command if final height is outside min-max range
    updateStateAndDirection(command);
    sendAppropriateCommand(loopData);
}

bool isDeskMoving() {
    return deskData.state != DeskState::OFF;
}
