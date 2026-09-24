#include <Arduino.h>

// GPIO Pin Definitions matching diagram.json
constexpr uint8_t PIN_LED    = 23;
constexpr uint8_t PIN_BUTTON = 19;

// Debounce timing parameter
constexpr uint32_t DEBOUNCE_DELAY_MS = 50;

// Latch State (Flip-Flop Memory)
bool ledLatchedState = false;

// Debounce Filter Variables
int lastRawButtonReading = HIGH;
int stableButtonState    = HIGH;
uint32_t lastDebounceTime = 0;

void setup() {
    Serial.begin(115200);

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    // Active-Low configuration: Internal pull-up holds pin at 3.3V (HIGH)
    // Pressing the button shorts pin to GND (LOW)
    pinMode(PIN_BUTTON, INPUT_PULLUP);

    Serial.println("[SYSTEM] Active-Low Debounced Toggle Latch Initialized");
    Serial.println("[STATE] LED: OFF");
}

void loop() {
    int currentRawReading = digitalRead(PIN_BUTTON);

    // 1. Detect raw electrical bounce/glitch
    if (currentRawReading != lastRawButtonReading) {
        lastDebounceTime = millis();
    }
    lastRawButtonReading = currentRawReading;

    // 2. Evaluate stability against the debounce window
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY_MS) {
        // Check if the confirmed state has actually changed
        if (currentRawReading != stableButtonState) {
            stableButtonState = currentRawReading;

            // 3. Falling edge detection (transition from HIGH to LOW)
            // On active-low wiring, LOW represents physical closure
            if (stableButtonState == LOW) {
                // Toggle latch state (T-flip-flop)
                ledLatchedState = !ledLatchedState;
                digitalWrite(PIN_LED, ledLatchedState ? HIGH : LOW);

                // Observability logging
                Serial.printf("[EVENT] Valid press detected -> Latch toggled: %s\n", 
                              ledLatchedState ? "ON" : "OFF");
            }
        }
    }
}