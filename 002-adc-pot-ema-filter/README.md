# ADC Potentiometer EMA Filter & Shift Register

## Overview
Analog signal conditioning engine featuring non-blocking 50Hz ADC sampling, Exponential Moving Average (EMA) noise filtration, and 3-wire serial expansion via the 74HC595 shift register. The system smooths analog wiper potential and concurrently renders an 8-level progressive visual bar graph on silicon and real-time millivolt telemetry on an I2C LCD1602 display without blocking delays.


## Physical Hardware Demonstration
[![Watch Physical Hardware Demo](https://img.youtube.com/vi/8H-c_i2Vf_g/maxresdefault.jpg)](https://youtu.be/8H-c_i2Vf_g)
> *Click thumbnail to view physical breadboard verification (unlisted video).*


## Simulation vs. Physical Silicon Realities
* **ADC Line Noise & Wiper Jitter:** In simulation, analog potentiometer inputs are mathematically ideal. On physical breadboard hardware, power rail ripple, contact resistance, and ambient electrical coupling generate ADC count fluctuations of ±20 to ±40 counts. A discrete Exponential Moving Average filter (`alpha = 0.15`) smooths high-frequency noise into stable voltage trajectories.
* **Bus Decoupling & Display Flicker:** Continuously clearing an I2C LCD causes visible screen flicker and bus contention. The firmware decouples tasks: high-speed 50Hz ADC acquisition and shift register updates run independently of the 10Hz LCD refresh rate, which overwrites fixed-width characters via `snprintf` without full-screen clears.

## Pinout Configuration
| ESP32 GPIO | Peripheral Pin | Direction / Mode | Functional Notes |
| :--- | :--- | :--- | :--- |
| GPIO 34 | Potentiometer Wiper (SIG) | INPUT (ADC1 Ch 6) | Analog voltage 0V to 3.3V (0-4095 counts) |
| GPIO 5 | 74HC595 DS (Pin 14) | OUTPUT (Push-Pull) | Serial Data Input (Orange) |
| GPIO 18 | 74HC595 STCP (Pin 12) | OUTPUT (Push-Pull) | Storage Register Clock (Output Latch, Purple) |
| GPIO 19 | 74HC595 SHCP (Pin 11) | OUTPUT (Push-Pull) | Shift Register Clock (Yellow) |
| GPIO 23 | LCD1602 SDA | BIDIRECTIONAL (I2C) | Hardware I2C Serial Data |
| GPIO 22 | LCD1602 SCL | OUTPUT (I2C) | Hardware I2C Serial Clock |
| 3V3 | Potentiometer VCC, 74HC595 VCC/MR | Power Rail | 3.3V analog and digital logic power |
| 5V | LCD1602 VCC | Power Rail | 5V display logic and backlight supply |
| GND | Common GND, 74HC595 OE | Ground Reference | Common ground return bus |

## Functional Logic Flow
| Processing Stage | Execution Interval | Data Transformation | Operational Target |
| :--- | :--- | :--- | :--- |
| `ADC Sampling` | 20ms (50Hz) | Read raw ADC1 counts (0-4095) | Acquire analog wiper potential |
| `EMA Filter` | 20ms (50Hz) | `(0.15 * raw) + (0.85 * filtered)` | Suppress high-frequency line chatter |
| `Bar Graph Latch` | On Value Change | Map filtered mV to 8-bit mask (`0x00`-`0xFF`) | Shift serially to 74HC595 and pulse latch |
| `LCD Telemetry` | 100ms (10Hz) | Format `RAW: %4u mV` and `EMA: %4u mV` | Fixed-width update without display clear |
| `UART Observability` | 1000ms (1Hz) | Emit structured `[TELEMETRY]` string | Bench debugging at 115200 baud |

## Wokwi Simulation
The circuit and firmware can be executed in the Wokwi simulator:
1. Open [diagram.json](diagram.json).
2. Press `F1` in VS Code and select `Wokwi: Start Simulator`.
3. Adjust the potentiometer to observe smoothed bar graph transitions and comparative raw vs. filtered telemetry on the LCD.
