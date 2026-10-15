# 001-fsm-mode-controller

## Overview
Non-blocking Finite State Machine (FSM) mode controller cycling through four operational states (`IDLE`, `TRAFFIC`, `STROBE`, `SOLID`) driven by a software-debounced momentary pushbutton. The firmware completely avoids blocking `delay()` calls by utilizing non-blocking timestamp comparisons with `millis()`, ensuring deterministic input responsiveness and telemetry reporting.

<!--
## Physical Hardware Demonstration
[![Watch Physical Hardware Demo](https://img.youtube.com/vi/YOUR_VIDEO_ID/maxresdefault.jpg)](https://www.youtube.com/watch?v=YOUR_VIDEO_ID)
> *Click thumbnail to view physical breadboard verification (unlisted video).*
-->

## Simulation vs. Physical Silicon Realities
* **Contact Bouncing:** In simulation, pushbutton switches transition between binary states instantaneously. In physical silicon, mechanical leaf springs exhibit contact chatter for 5ms to 30ms during closure and release. A 50ms software qualification window (`DEBOUNCE_DELAY_MS`) guarantees that only stabilized state transitions trigger state advances.
* **Floating Gate Susceptibility:** An open CMOS input pin has gigohm-level input impedance and will float unpredictably due to stray electrostatic fields. Configuring `INPUT_PULLUP` engages the internal ~45kΩ pull-up resistor, asserting a stable 3.3V (`HIGH`) baseline until grounded by switch closure.

## Pinout Configuration
| ESP32 GPIO | Peripheral Pin | Direction / Mode | Functional Notes |
| :--- | :--- | :--- | :--- |
| GPIO 23 | Red LED Anode | OUTPUT (Push-Pull) | Series 220Ω current-limiting resistor |
| GPIO 22 | Yellow LED Anode | OUTPUT (Push-Pull) | Series 220Ω current-limiting resistor |
| GPIO 21 | Green LED Anode | OUTPUT (Push-Pull) | Series 220Ω current-limiting resistor |
| GPIO 19 | Tactile Pushbutton (1.l) | INPUT_PULLUP | Active-Low switch closure to GND |
| GND | Cathodes & Button (2.l) | Ground Reference | Common ground return bus |

## State Machine Architecture
| State | Timing Interval | LED Outputs (R, Y, G) | Transition Trigger |
| :--- | :--- | :--- | :--- |
| `STATE_IDLE` | Static | OFF, OFF, OFF | Button press (Falling Edge) |
| `STATE_TRAFFIC` | 800ms periodic step | Sequential cycle (Red -> Yellow -> Green) | Button press (Falling Edge) |
| `STATE_STROBE` | 100ms periodic toggle | Synchronous all-LED toggle | Button press (Falling Edge) |
| `STATE_SOLID` | Static | ON, ON, ON | Button press (Cycles to `IDLE`) |

## Wokwi Simulation
The circuit and firmware can be executed in the Wokwi simulator:
1. Open [diagram.json](diagram.json).
2. Press `F1` in VS Code and select `Wokwi: Start Simulator`.
3. Click the pushbutton to step through state machine modes and observe UART output at 115200 baud.
