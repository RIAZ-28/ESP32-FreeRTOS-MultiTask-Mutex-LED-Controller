# Concurrent Multi-LED Control with FreeRTOS — Mutex-Protected Shared GPIO Register

## What it does
Three LEDs blink independently and asynchronously on an ESP32, each driven by its
own FreeRTOS task with its own timing. A button pauses all three LEDs exactly where
they are mid-cycle (phase-preserving, not just on/off), and resumes each from that
same point on release. A UART interface accepts single-character commands to
override behavior: '1' forces all LEDs on, '0' forces all off, 'b' resumes normal
independent blinking.

## What makes it non-trivial
All three LED pins share the same GPIO_OUT_REG. With three independent tasks each
performing read-modify-write operations on that register concurrently, there's a
real race condition: one task's bit-set/clear can be overwritten mid-operation by
another task writing the same register at the same time. This project uses a mutex
to serialize register access across tasks, preventing corruption of the shared
output state.

Pausing is phase-preserving: each task retains its position within its own blink
cycle rather than just freezing its current on/off value, so a fast-blinking LED
and a slow-blinking LED both resume exactly where they left off, not from a reset
state.

## How it works
- Each LED's blink logic runs as a separate FreeRTOS task with independent timing
- GPIO_OUT_REG writes are wrapped in mutex acquire/release to prevent concurrent
  corruption across tasks
- Button press signals all tasks to suspend, holding current phase state
- Button release resumes each task from its held phase
- UART RX parses single-character commands ('1', '0', 'b') and overrides task
  behavior accordingly

## Hardware / Setup

**Components**
- ESP32 30 pin CP2102 development board
- 3x LED + 220Ω resistors
- 1x push button
- Breadboard
- jumper wires

**Pin Mapping**

| Component | ESP32 Pin |
|-----------|-----------|
| LED 1     | GPIO 2    |
| LED 2     | GPIO 4    |
| LED 3     | GPIO 5    |
| Button    | GPIO 19   |

**Wiring Notes**
- Button configured with external pull-up; reads LOW when pressed
- LEDs wired common cathode to GND through 220Ω resistor from each GPIO pin

**Software**
- ESP-IDF v6.0.1
- FreeRTOS (bundled with ESP-IDF)

**Build & Flash**
\`\`\`
idf.py set-target esp32
idf.py build
idf.py -p [YOUR_PORT] flash monitor
\`\`\`

## Next steps
- Extend UART interface to accept per-LED commands, not just global ones
- Add debounce handling on the button input if not already present
- Consider replacing polling-based UART read with interrupt-driven RX

## Demo
[Short clip: independent blinking, button pause showing phase preservation, UART
commands '1'/'0'/'b' in action]
