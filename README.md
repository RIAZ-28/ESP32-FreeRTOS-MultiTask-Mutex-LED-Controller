# ESP32-FreeRTOS-MultiTask-Mutex-LED-Controller

## What it does

Three LEDs blink independently on an ESP32, with each LED controlled by its own
FreeRTOS task and its own timing interval.

A UART interface accepts single-character commands:

* `'1'` forces all LEDs on
* `'0'` forces all LEDs off
* `'b' enables normal independent blinking

A push button pauses blinking while pressed.

This was the **initial implementation** of the project and was built to explore
FreeRTOS task creation, task priorities, mutex-based synchronization, UART
communication, and direct GPIO register manipulation.

## What makes it non-trivial

The three LEDs are controlled by three independent FreeRTOS tasks. Each task
operates on the shared `GPIO_OUT_REG`, so concurrent read-modify-write operations
can create a race condition.

A FreeRTOS mutex is therefore used around GPIO register access to ensure that only
one task modifies the shared GPIO output register at a time.

Each LED task also has a different execution period:

* LED 1 → 500 ms
* LED 2 → 1000 ms
* LED 3 → 1500 ms

This produces independent blinking behavior while demonstrating how multiple
FreeRTOS tasks can run with different timing and priorities.

## Initial limitation

While the LEDs followed their assigned timing correctly, the system felt
**slow to respond to changes in control input**.

The main reason was the relatively long `vTaskDelay()` used inside each LED task:

```c
vTaskDelay(500 / portTICK_PERIOD_MS);
vTaskDelay(1000 / portTICK_PERIOD_MS);
vTaskDelay(1500 / portTICK_PERIOD_MS);
```

Because each LED task could remain delayed for a relatively long interval,
changes to the global `mode` or `paused` state were not always acted upon
immediately.

For example, an LED task waiting for its next 1500 ms cycle could take a
significant amount of time before checking the updated control state.

This became the motivation for redesigning the task architecture.

## How it works

* Each LED has its own dedicated FreeRTOS task
* Each task uses a different delay interval for blinking
* `GPIO_OUT_REG` access is protected using a FreeRTOS mutex
* A UART task reads single-character commands and updates the global mode
* A button task monitors the pause input
* LED tasks continuously evaluate the current mode and pause state
* GPIO is controlled through direct register access

## Hardware / Setup

**Components**

* ESP32 30 pin CP2102 development board
* 3x LED + 220Ω resistors
* 1x push button
* Breadboard
* jumper wires

**Pin Mapping**

| Component | ESP32 Pin |
| --------- | --------- |
| LED 1     | GPIO 2    |
| LED 2     | GPIO 4    |
| LED 3     | GPIO 5    |
| Button    | GPIO 19   |

**Wiring Notes**

* Button configured with external pull-up; reads LOW when pressed
* LEDs wired common cathode to GND through 220Ω resistor from each GPIO pin

**Software**

* ESP-IDF
* FreeRTOS (bundled with ESP-IDF)

**Build & Flash**

```bash
idf.py set-target esp32
idf.py build
idf.py -p port7 flash monitor
```

## Lessons learned

This implementation helped identify an important real-time systems concept:

**Task delay directly affects how frequently a task can react to changing system
state.**

Although separate tasks made the LED behavior easy to understand, using long
blocking delays was not ideal when the same tasks were also responsible for
reacting to external commands.

This led to the second implementation, which uses a reusable task architecture
and shorter scheduling intervals.

## Next steps

* Improve responsiveness further using FreeRTOS notifications or event groups
* Replace polling-based UART input with an event-driven approach
* Add proper button debounce handling
* Add per-LED UART control
* Compare task-based timing with FreeRTOS software timers

## Demo

[Short clip: three LEDs blinking at different rates, UART commands, and button
pause behavior]
