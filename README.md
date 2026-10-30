# ESP32 Experiments

A collection of hands-on experiments exploring the ESP32 and understanding how hardware works. Each project covers wiring circuits, writing firmware, and observing physical component behavior.

---

## Bench Notes: Simulation vs. Hardware

* **Simulation (Wokwi):** Quick logic checks and circuit testing in a digital environment.
* **Physical Bench:** Testing on a breadboard to observe contact bounce and electrical behavior.

---

## Experiment Matrix

| Index | Project | Architecture / Concepts | Hardware |
| :---: | :--- | :--- | :--- |
| 001 | [LED Pattern Sequencer](001-led-pattern-sequencer/README.md) | Non-blocking FSM timing, bitwise GPIO masking, software switch debouncing | 8x LEDs, 8x 220Ω Resistors, 1x Pushbutton |
| 002 | [ADC Potentiometer EMA Filter & Shift Register](002-adc-pot-ema-filter/README.md) | Discrete EMA filtering, 74HC595 serial expansion, dual-rate I2C LCD telemetry | 1x Potentiometer, 1x 74HC595, 8x LEDs, 1x LCD1602 |

