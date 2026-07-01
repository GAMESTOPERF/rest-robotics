# Mini IVD 2024-25 — Sensor Mission 3: Autonomous Lane Keeping

## Challenge (from Parameters)
Car placed **between two white lane marker lines on dark asphalt at a 45° angle**. On operator initiation, car drives forward autonomously. On detecting the lane marker, car adjusts steering and motor to stay within the lane. Car must travel **at least 20 feet** within the lane. Camera and sensor technology encouraged.

## Hardware
- RC car + Arduino UNO + logic chip (selAB)
- Camera: connected via `SoftwareSerial(10, 11)` — sends `<L,VALUE>` packets (line error)
- Drive ESC: `motorESC` on `escMPin = 9`
- Steering servo: `motorSteer` on `escSPin = 12` (**FIXED — was 10, conflicted with camera RX**)
- 6-channel digital line sensor: pinLL=7, pinL=6, pinC1=5, pinC2=4, pinR=3, pinRR=2 (declared, not used in loop — camera handles detection)
- `selAB = 8`: LOW = Arduino override

## Code Logic (`Sensor_3.ino`)
Camera sends signed line error (pixels from center):
- Negative = line is to the right → steer right (`motorSteer.write(80)`)
- Positive = line is to the left → steer left (`motorSteer.write(100)`)
- Within ±25 deadzone → straight (`motorSteer.write(90)`)

Obstacle check on A0: `< 50` → stop.

Noise filter: weighted running average (`filteredError = (filteredError * 3 + newError) / 4`); jumps > 100px rejected.

## Key Fixes Applied
| Bug | Fix |
|-----|-----|
| `escSPin = 10` conflicted with `SoftwareSerial(10, 11)` camera RX | Moved `escSPin` to pin 12 |
| `rdRCesc = 6` and `pinL = 6` (dead code conflict) | Removed unused RC read pins |
| `motorESC.write(1500)` in stop case — wrong function for ESC | Fixed to `writeMicroseconds(1500)` |

## Pin Conflict Note
If your hardware has camera wired differently, options:
- **Current fix (in code)**: `SoftwareSerial(10,11)`, `escSPin=12`
- **Alternative**: `SoftwareSerial(12,13)`, `escSPin=10` (put camera on different pins)
- Do NOT use hardware Serial (0,1) unless you remove USB debug prints

## Camera Packet Format
`<L,VALUE>\n` where VALUE = signed integer pixel error from center.
Adjust camera firmware to match if using a different camera.

## Files
`mini-ivd-team/2024-2025/Sensor_3/Sensor_3/Sensor_3.ino`
