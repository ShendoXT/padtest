# Bass Landing #8670 Controller Notes

These notes document the current reverse-engineered behavior of the Bass Landing #8670 fishing controller for PlayStation 1. The mapping was captured from real hardware using a modified PadTest build that can log raw controller bus transactions.

## Summary

The controller reports the documented `5AE5h` Fishing/Mode1 ID during normal polling. In that initial protocol state, its controls behave like digital-controller buttons and the analog stick reports as D-pad input. Extended fishing reports were observed after sending the `0x43` config command with byte 3 set to `0x01`, then polling with `0x43` byte 3 set to `0x00`.

In PadTest debug probe names:

| Probe | TX bytes | Observed role |
| --- | --- | --- |
| `POLL42` | `01 42 00 ...` | Standard digital-compatible poll |
| `CFG ON` | `01 43 00 01 ...` | Appears to activate or wake extended reporting |
| `CFGOFF` | `01 43 00 00 ...` | Returns the extended `0xE5` fishing packet after activation |

## Observed Handshake

1. Initial discovery uses the standard 9-byte `POLL42` transaction and returns response type `0xE5` / acknowledgement `0x5A`.
2. PadTest displays this as `Fishing` and then uses the short 5-byte `POLL42` digital poll so the stick remains digital-D-pad-like. Short initial-state polls may answer as `0x41`; once the controller has identified as `0xE5`, PadTest keeps it classified as fishing.
3. Pressing `Start + Select` on a fishing controller sends a 16-byte `CFG ON`.
4. PadTest then polls with 16-byte `CFGOFF` for a short activation window. A `CFGOFF` response with `E5 5A` enables the extended fishing readout.
5. Repeated `CFGOFF` polls keep returning response type `0xE5` and include fishing-specific extended data.

If the controller only identifies as a standard `0x41` digital pad, pressing `Start + Select` on that digital pad attempts the same activation sequence once per press. PadTest switches to the extended fishing readout only if the `CFGOFF` activation window confirms `E5 5A`.

The PadTest debug mode can also reproduce the extended sequence manually:

1. Enter debug mode with `L1 + R1 + Triangle` on port 2.
2. Use `L1`/`R1` to select `CFG ON`, then press `Cross` once.
3. Select `CFGOFF`, then press `Circle` to stream it.
4. Use `Triangle` to open the bus log view for stable screenshots.

## Extended Packet Layout

The extended response is 13 meaningful bytes. Byte numbering below uses zero-based byte offsets as displayed by the raw debug/log views.

| Offset | Meaning | Status |
| --- | --- | --- |
| `0x01` | Response type, observed as `0xE5` | Confirmed |
| `0x02` | Acknowledgement, observed as `0x5A` | Confirmed |
| `0x03`/`0x04` | Digital button word, active-low style | Likely |
| `0x07` | Analog stick X | Observed |
| `0x08` | Analog stick Y | Observed |
| `0x09` | Motion axis X | Observed |
| `0x0A` | Motion axis Z | Observed |
| `0x0B` | Motion axis Y | Observed |
| `0x0C` | Reel rotation speed magnitude | Observed |

The exact polarity of the motion axes still needs verification. The reel byte appears to behave like a rotation speed sensor rather than a simple movement flag. Slow movement may produce low values, while aggressive manual reeling has been observed around `0x1D`.

## Open Questions

- Does `CFG ON` need to be sent once, for multiple frames, or until a specific response is seen?
- Is `CFGOFF` the true steady-state extended poll, or is it a side effect of config mode?
- Are the motion bytes accelerometer axes, gyro-like values, or a mixed sensor packet?
- Does the reel byte encode direction anywhere, or only magnitude/rate?
- Are bytes not listed above meaningful under controls not yet isolated?

## Suggested Capture Checklist

For future verification, capture the extended packet while testing:

- Resting controller on a flat surface.
- Analog stick center and each extreme.
- Rod tilt forward/back/left/right.
- Rod swing/cast motions.
- Reel slow forward, fast forward, and any reverse movement if mechanically possible.
- Each digital button/control pressed alone.

When sharing captures, include the command used (`CFG ON`, `CFGOFF`, etc.), response type, and full RX bytes.
