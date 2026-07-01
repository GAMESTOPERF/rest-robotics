# Shared Libraries

Code in this folder is used by **all three teams**. If you change something here, it affects everyone — test carefully and mention it in your commit message.

---

## What Goes Here

- Utility functions used across multiple robots (motor helpers, sensor wrappers, etc.)
- Reusable `.h` header files
- Example sketches that demonstrate how to use a library

**What does NOT go here:**
- Team-specific competition code (put that in the team's folder)
- Large vendor library dumps (install those via Arduino Library Manager instead)

---

## Examples Subfolder

`examples/` contains reference sketches for hardware your team has used:

| Example | Hardware |
|---------|----------|
| `ELEGOOSmartRobotCarV4.0/` | ELEGOO smart car kit |
| `Mars Explorer Mecanum Robot/` | Mars Explorer mecanum chassis |
| `Motor drivers/` | Generic motor driver (model Y sample) |
| `Yahboom 8 channel IR sensor/` | Line-following sensor module |

---

## Adding a New Shared Library

1. Create a folder with a clear name (e.g., `rest-pid-controller/`)
2. Put your `.h` and `.cpp` files inside
3. Add a short comment at the top of each file explaining what it does and what hardware it's for
4. Update this README with a row in the table above
5. Include a simple example sketch in `examples/`

---

## Installing Libraries Used Here

Some libraries need to be installed in Arduino IDE separately. Open Arduino IDE → **Sketch → Include Library → Manage Libraries** and search for:

- `Adafruit PWM Servo Driver Library`
- `IRremote`
- `LIDARLite`
- `PingTimer`
