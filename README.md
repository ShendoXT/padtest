# PadTest
### Gamepad test application for PlayStation 1

![padtestscreen](https://raw.githubusercontent.com/ShendoXT/padtest/master/images/screenshot.png)
## Supported controllers:
* Digital (SCPH-1080) controller
* DualShock analog (SCPH-1200) controller
* NeGcon-compatible controller (`5A23h`, including Ultra Racer)
* PlayStation Mouse
* Fishing controller (`5AE5h`, tested with Bass Landing #8670)

## Requirements:
* A way to run homebrew on PlayStation, be it modchip, cart, swap method or FreePSXBoot.
* (For developers) Working PSXSDK toolchain to compile the software. You can download it here: http://unhaut.epizy.com/psxsdk/?i=1.

## How to compile:
Run "make res" to compile resources and then "make" to build the software.

### Usage:
Connect a controller of your choice to either port and test it's buttons.

Analog controllers should automatically switch to analog "red led mode".
To test rumble press L3 for big motor and R3 for small motor.

Fishing controllers may first appear as a Digital pad. Press `Start + Select` to poll for extended functionality. If fishing data is confirmed, PadTest displays analog stick, motion axes, and reel rotation speed.

NeGcon-compatible controllers display twist plus I/II/L analog values. Ultra Racer-style devices also show their extra shoulder buttons.

This software is intended to be ran on the actual PlayStation 1 or PSone console.
Since it's using direct memory access to SIO ports it may not work on emulators or other consoles (PlayStation 2).

### Advanced Debug Mode

PadTest includes a hidden runtime debug dashboard for reverse-engineering unusual controllers. Connect the controller being inspected to port 1 and use a normal controller in port 2 to operate the debug UI.

Press `L1 + R1 + Triangle` on port 2 to enter or exit debug mode. Debug mode hides the normal header/footer so the raw tools have more screen space.

Raw packet view controls:

* `L1` / `R1`: cycle probe command.
* `Cross`: fire the selected probe once and briefly hold the response.
* `Circle`: toggle streaming the selected probe.
* `Select`: clear changed-byte and min/max history.
* `Triangle`: switch to the bus log view.

Bus log view controls:

* `Up` / `Down`: scroll one entry at a time.
* `Left` / `Right`: page eight entries at a time.
* `Cross`: clear the log.
* `Triangle`: return to the raw packet view.

The log stores the most recent 128 port 1 controller bus transactions and pauses capture while the log view is open so screenshots stay stable.

See `docs/bass-landing-8670-protocol.md` for reverse-engineering notes on the Bass Landing #8670 and the `5AE5h` fishing controller ID.

## FreePSXBoot
Included in the release is a UPX compressed executable.<br>
It is identical in functionality but is smaller (37 Kb) because it is compressed.<br>
It can be used with FreePSXBoot and ran directly on boot as it fits on a MemoryCard.
Make sure to use -fastload option while building the memory card image.
