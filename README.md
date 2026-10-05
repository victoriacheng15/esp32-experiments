# ESP32 Experiments

A progressive embedded firmware laboratory built on the ESP32 platform, focusing on low-level firmware engineering principles: deterministic non-blocking timing, finite state machine (FSM) architecture, hardware interrupts, signal conditioning, peripheral offloading, and real-time task orchestration.

---

## Verification Methodology: Simulation vs. Physical Bench

* **Simulation (Wokwi):** Logic verification, protocol decoding, and state transition validation in an ideal digital environment prior to flashing.
* **Physical Bench:** Contact chatter, signal integrity, and electrical realities verified via unlisted YouTube video demonstrations.

---

## Experiment Matrix

| Index | Project | Architecture / Concepts | Hardware |
| :---: | :--- | :--- | :--- |
| **001** | [`001-fsm-mode-controller`](001-fsm-mode-controller) | Non-blocking FSM (`millis()`) | 3x LEDs, 1x Button |
| **002** | [`002-active-low-toggle-latch`](002-active-low-toggle-latch) | Active-Low edge detection & latch | 1x LED, 1x Button |
