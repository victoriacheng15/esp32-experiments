# LED Pattern Sequencer

## Overview
Discrete 8-LED pattern sequencer driven by a debounced pushbutton state machine. The firmware uses non-blocking `millis()` timing to execute four discrete visual patterns (Knight Rider scan, progressive bar meter, 8-bit binary counter, and alternating nybble strobe) with deterministic, per-pattern execution rates.

## Physical Hardware Demonstration
[![Watch Physical Hardware Demo](https://img.youtube.com/vi/Axgfo9HQ73s/maxresdefault.jpg)](https://youtu.be/Axgfo9HQ73s)
> *Click thumbnail to view physical breadboard verification (unlisted video).*

## Simulation vs. Physical Silicon Realities
* **Switch Contact Chatter:** In simulation, pushbutton transitions are instantaneous and bounce-free. On physical breadboard hardware, mechanical switch contacts bounce for several milliseconds upon closure. A software debounce timer (`DEBOUNCE_DELAY_MS = 50`) ensures deterministic single-press detection without hardware RC filters.
* **Non-Blocking Observability:** Rather than continuous UART polling loops, the controller emits single-line structured events via `Serial.println()` strictly upon pattern transitions and system initialization.

## Pinout Configuration
| ESP32 GPIO | Peripheral Pin | Direction / Mode | Functional Notes |
| :--- | :--- | :--- | :--- |
| GPIO 23 | Red LED 0 Anode | OUTPUT (Push-Pull) | Series 220Ω current-limiting resistor |
| GPIO 22 | Red LED 1 Anode | OUTPUT (Push-Pull) | Series 220Ω current-limiting resistor |
| GPIO 21 | Yellow LED 2 Anode | OUTPUT (Push-Pull) | Series 220Ω current-limiting resistor |
| GPIO 19 | Yellow LED 3 Anode | OUTPUT (Push-Pull) | Series 220Ω current-limiting resistor |
| GPIO 18 | Green LED 4 Anode | OUTPUT (Push-Pull) | Series 220Ω current-limiting resistor |
| GPIO 5 | Green LED 5 Anode | OUTPUT (Push-Pull) | Series 220Ω current-limiting resistor |
| GPIO 26 | Blue LED 6 Anode | OUTPUT (Push-Pull) | Series 220Ω current-limiting resistor |
| GPIO 25 | Blue LED 7 Anode | OUTPUT (Push-Pull) | Series 220Ω current-limiting resistor |
| GPIO 4 | Pushbutton Terminal (1.l) | INPUT_PULLUP | Active-Low pattern toggle to GND |
| GND | Cathodes, Button GND | Ground Reference | Common ground return bus |

## State Machine Architecture
| State | Timing Interval | LED Outputs | Transition Trigger |
| :--- | :--- | :--- | :--- |
| `SCAN` | 80ms | Bidirectional single-LED sweep | Button press (Falling Edge) |
| `BAR` | 100ms | Progressive 8-LED fill and drain | Button press (Falling Edge) |
| `BINARY` | 150ms | 8-bit binary counter sequence | Button press (Falling Edge) |
| `ALTERNATING` | 250ms | Alternating nybbles (0xAA vs 0x55) | Button press (Cycles to `SCAN`) |

## Wokwi Simulation
The circuit and firmware can be executed in the Wokwi simulator:
1. Open [diagram.json](diagram.json).
2. Press `F1` in VS Code and select `Wokwi: Start Simulator`.
3. Press the pushbutton to cycle through the pattern states.
