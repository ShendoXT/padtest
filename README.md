# PadTest
### Gamepad test application for PlayStation 1

![padtestscreen](https://raw.githubusercontent.com/ShendoXT/padtest/master/images/screenshot.bmp)
## Supported controllers:
Regular digital (SCPH-1080) and DualShock analog (SCPH-1200) controllers are supported.

## Requirements:
* PlayStation 1 or PSone console capable of booting burned CDs.
* Working PSXSDK toolchain to compile the software. You can download it here: http://unhaut.epizy.com/psxsdk/?i=1.

## License:
There is no particular licence associated with this project, however I would like to be credited for my work.    
So if you use all or part of this code a mention under "thanks to" would be nice.

## How to compile:
Run "make res" to compile resources and then "make" to build the software.

### Usage:
Burn a bin/cue image to CD and boot it on your console.    
After that connect a controller of your choice to either port and test it's buttons.

Analog controllers should automatically switch to analog "red led mode".    
To test rumble press L3 for big motor and R3 for small motor.

This software is intended to be ran on the actual PlayStation 1 or PSone console.    
Since it's using direct memory access to SIO ports it may not work on emulators or other consoles (PlayStation 2).

### Disclaimer:
This software is provided "as is" without any guarantees or warranty. Usage of this software is at the user's own risk.    
The Author, Shendo won't be held responsible if any harm is caused by using this software.

If you disagree with any of this DO NOT use PadTest.
