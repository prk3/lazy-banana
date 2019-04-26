# About
Arduino Nano based controller for IKEA Bekant desk. Allows for manual height control, saving positions and auto height adjustment with a press of a button.

# Schematic
![Schematic](/schematic.png?s=200)
This schematic is a mix of trainman419's work, gcormier's improvements and my poor electronics skills. Please refer to the original in case you're uncertain about it.

# Parts
| name                                      | amount |
| ----------------------------------------- | ------ |
| Arduino nano clone - Atmega328, 16MHz, 5V | 1      |
| Step down converter LM2596S max 35V       | 1      |
| 4-digit 7 segment display TM1637          | 1      |
| LIN interface/transceiver MCP2003B        | 1      |
| temperature sensor DS18B20                | 1      |
| transistor BC639                          | 1      |
| capacitor 220pF                           | 1      |
| diode 1N4148                              | 5      |
| resistor 100k                             | 1      |
| resistor 10k                              | 3      |
| resistor 4.7k                             | 2      |
| resistor 2.2k                             | 2      |
| resistor 1k                               | 1      |

# Interface
Before testing auto-move, please factory reset the controller to initialize presets in EEPROM. You can do it by holding all three buttons on home screen.

- **SLEEP SCREEN**: Controller switches to this state after some time (15s by default) of inactivity. Press any button to wake up the controller and go to **home screen**.

- **HOME SCREEN**: Shows current height. Pressing buttons A-C will move desk to corresponding preset (**auto move screen**). Holding button A will redirect to **temperature screen**. Holding buttons B or C will redirect to **manual move screen**. Holding all three buttons will factory reset height presets.

- **TEMPERATURE SCREEN**: Shows temperature read from the Dallas sensor. Press any button to exit.

- **AUTO MOVE SCREEN**: Moves the desk to saved position. Press any button to cancel.

- **MANUAL MOVE SCREEN**: Holding B moves desk down, holding C moves desk up. Press button A to exit. Hold button A to save position (**save position screen**).

- **SAVE POSITION SCREEN**: Press button A, B or C to save position to preset 1, 2 or 3. Hold any button to cancel. 

# Code
Each part of controller's functionality is separated into `.cpp` + `.h` pair. All modules are then imported in `main.cpp` and used in a big state machine. Every state has it's id, initialization procedure and a handler (called every 50ms), which reacts to IO changes, performs some actions and redirects to a different state if needed.

All pins are configurable via const variables at the top of header files. Keep in mind that buttons work in INPUT mode and wake pin works in INPUT_PULLUP mode.

The code has been written in vscode with PlatformIO plugin. To make it work with Arduino IDE, move all files from `include`, `src` and `lib/gcormier_megadesk` to one folder named `main` and rename `main.cpp` file to `main.ino`. Make sure you have all dependencies installed.

# Dependencies
This project uses the following Arduino libraries:
1. OneWire - [https://github.com/PaulStoffregen/OneWire](https://github.com/PaulStoffregen/OneWire)
2. DallasTemperature - [https://github.com/milesburton/Arduino-Temperature-Control-Library.git](https://github.com/milesburton/Arduino-Temperature-Control-Library.git)
3. TM1637 [https://github.com/avishorp/TM1637.git](https://github.com/avishorp/TM1637.git)

# TODO
1. Add photos of the controller.
2. Model a case and 3d print it.
3. Record a demo.

# Issues
1. Auto move algorithm emulates UP/DOWN button pressing to reach target height. A better solution would be to keep desk in fine movement mode when it's close to target and slowly approach desired position.
2. Auto move algorithm isn't aware of desk's velocity. For this reason the desk refuses to move between presets that are very close to each other (controller thinks that desk is at full speed and would overshoot the target, thus it doesn't move at all).
3. Motors gradually slow down near min and max heights. This isn't handled by the controller and auto move undershoots targets near min and max.

# Credit
Big thanks to:
1. robin7331([IKEA-Hackant](https://github.com/robin7331/IKEA-Hackant/) project) for inspiring video and detailed explanation of desk's internals
2. trainman419 ([bekant](https://github.com/trainman419/bekant) project) for reproducing behavior of stock controller on Arduino
3. gcormier ([megadesk](https://github.com/gcormier/megadesk) project) for improvements to trainman419's schematic and code
