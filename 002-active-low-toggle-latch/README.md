# 002-active-low-toggle-latch

## Overview
Active-Low Debounced Toggle Latch implementing a software T-flip-flop memory pattern. The controller detects confirmed falling-edge electrical events from a momentary pushbutton, inverts internal latch state memory, and drives a persistent LED output without blocking the main event loop.

<!--
## Physical Hardware Demonstration
[![Watch Physical Hardware Demo](https://img.youtube.com/vi/YOUR_VIDEO_ID/maxresdefault.jpg)](https://www.youtube.com/watch?v=YOUR_VIDEO_ID)
> *Click thumbnail to view physical breadboard verification (unlisted video).*
-->

## Simulation vs. Physical Silicon Realities
* **Contact Bouncing:** Mechanical switches do not produce clean step responses. Metal contacts chatter rapidly for 5ms to 25ms upon physical closure. Without a timing window, a single button depression toggles the latch multiple times indeterminately. A 50ms qualification window (`DEBOUNCE_DELAY_MS`) filters out mechanical noise before latch state inversion is evaluated.
* **Active-Low Topology Advantage:** Wiring inputs active-low with `INPUT_PULLUP` means remote control lines carry ground potential when switched, eliminating the danger of accidental VCC-to-chassis shorts across breadboard wiring harnesses.

## Pinout Configuration
| ESP32 GPIO | Peripheral Pin | Direction / Mode | Functional Notes |
| :--- | :--- | :--- | :--- |
| GPIO 23 | Green LED Anode | OUTPUT (Push-Pull) | Series 220Ω current-limiting resistor |
| GPIO 19 | Tactile Pushbutton (1.l) | INPUT_PULLUP | Active-Low switch closure to GND |
| GND | Cathode & Pushbutton (2.l) | Ground Reference | Common ground return bus |

## Functional Logic Flow
1. **Raw Sample:** Sample GPIO 19 state on every iteration.
2. **Glitch Filter:** If raw state differs from previous raw sample, reset `lastDebounceTime`.
3. **Threshold Qualification:** Once reading remains constant for longer than 50ms, confirm state as stable.
4. **Falling-Edge Detection:** If stable state transitioned from `HIGH` to `LOW`, toggle `ledLatchedState`.
5. **Actuation & Telemetry:** Drive GPIO 23 to match latch memory and emit structured status over UART at 115200 baud.

## Wokwi Simulation
The simulation is preconfigured in [diagram.json](diagram.json):
1. Open [diagram.json](diagram.json).
2. Press `F1` in VS Code and select `Wokwi: Start Simulator`.
3. Click the pushbutton to verify latch state toggling and monitor serial logs.
