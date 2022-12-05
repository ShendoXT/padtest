# PadTest
### Gamepad test application for PlayStation 1

![padtestscreen](https://raw.githubusercontent.com/ShendoXT/padtest/master/images/screenshot.png)
## Supported controllers:
* Digital (SCPH-1080) controller
* DualShock analog (SCPH-1200) controller
* PlayStation Mouse

## Requirements:
* A way to run homebrew on PlayStation, be it modchip, cart, swap method or FreePSXBoot.
* (For developers) Working PSXSDK toolchain to compile the software. You can download it here: http://unhaut.epizy.com/psxsdk/?i=1.

## How to compile:
Run "make res" to compile resources and then "make" to build the software.

### Usage:
Connect a controller of your choice to either port and test it's buttons.

Analog controllers should automatically switch to analog "red led mode".    
To test rumble press L3 for big motor and R3 for small motor.

This software is intended to be ran on the actual PlayStation 1 or PSone console.
Since it's using direct memory access to SIO ports it may not work on emulators or other consoles (PlayStation 2).

## FreePSXBoot
Included in the release is a UPX compressed executable.<br>
It is identical in functionality but is smaller (37 Kb) because it is compressed.<br>
It can be used with FreePSXBoot and ran directly on boot as it fits on a MemoryCard.
