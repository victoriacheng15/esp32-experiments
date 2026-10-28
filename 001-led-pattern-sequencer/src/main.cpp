#include <Arduino.h>

// GPIO Pin Definitions matching diagram.json
constexpr uint8_t LED_PINS[8] = { 23, 22, 21, 19, 18, 5, 26, 25 };
constexpr uint8_t PIN_BUTTON  = 4;

// Timing Constants (milliseconds)
constexpr uint32_t DEBOUNCE_DELAY_MS = 50;

// Sequencer Patterns
enum class Pattern : uint8_t {
    SCAN = 0,
    BAR,
    BINARY,
    ALTERNATING,
    COUNT
};

// State Variables
Pattern currentPattern = Pattern::SCAN;
uint32_t lastStepTime  = 0;
uint8_t patternStep    = 0;

// Pushbutton Debounce Tracking Variables
int lastRawButtonReading = HIGH;
int stableButtonState    = HIGH;
uint32_t lastDebounceTime = 0;

void applyLedMask(uint8_t mask) {
    for (uint8_t i = 0; i < 8; ++i) {
        digitalWrite(LED_PINS[i], (mask & (1 << i)) ? HIGH : LOW);
    }
}

uint8_t computePatternMask(Pattern pattern, uint8_t step) {
    switch (pattern) {
        case Pattern::SCAN: {
            // Bidirectional ping-pong sweep across 8 LEDs (14 steps total)
            constexpr uint8_t scanLut[14] = {
                0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
                0x40, 0x20, 0x10, 0x08, 0x04, 0x02
            };
            return scanLut[step % 14];
        }

        case Pattern::BAR: {
            // Progressive fill (0-7) followed by progressive drain (8-15)
            uint8_t phase = step % 16;
            if (phase < 8) {
                return static_cast<uint8_t>((1U << (phase + 1)) - 1U);
            } else {
                return static_cast<uint8_t>(0xFFU ^ ((1U << (phase - 7)) - 1U));
            }
        }

        case Pattern::BINARY:
            // 8-bit binary counter progression
            return step;

        case Pattern::ALTERNATING:
            // High-frequency alternating nybble toggle
            return (step % 2 == 0) ? 0xAA : 0x55;

        default:
            return 0x00;
    }
}

uint16_t getPatternStepCount(Pattern pattern) {
    switch (pattern) {
        case Pattern::SCAN:        return 14;
        case Pattern::BAR:         return 16;
        case Pattern::BINARY:      return 256;
        case Pattern::ALTERNATING: return 2;
        default:                   return 1;
    }
}

uint32_t getPatternIntervalMs(Pattern pattern) {
    switch (pattern) {
        case Pattern::SCAN:        return 80;
        case Pattern::BAR:         return 100;
        case Pattern::BINARY:      return 150;
        case Pattern::ALTERNATING: return 250;
        default:                   return 100;
    }
}

const char* getPatternName(Pattern pattern) {
    switch (pattern) {
        case Pattern::SCAN:        return "SCAN (Ping-Pong)";
        case Pattern::BAR:         return "BAR (Fill/Drain)";
        case Pattern::BINARY:      return "BINARY (8-Bit Counter)";
        case Pattern::ALTERNATING: return "ALTERNATING (Nybble Strobe)";
        default:                   return "UNKNOWN";
    }
}

void transitionToPattern(Pattern newPattern) {
    currentPattern = newPattern;
    patternStep = 0;
    lastStepTime = millis();

    String msg = String("[STATE] Pattern: ") + getPatternName(currentPattern) + " (Interval: " + String(getPatternIntervalMs(currentPattern)) + " ms)";
    Serial.println(msg);
}

bool checkButtonPress() {
    int currentRaw = digitalRead(PIN_BUTTON);
    bool pressDetected = false;

    if (currentRaw != lastRawButtonReading) {
        lastDebounceTime = millis();
    }
    lastRawButtonReading = currentRaw;

    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY_MS) {
        if (currentRaw != stableButtonState) {
            stableButtonState = currentRaw;

            // Falling edge indicates physical closure to GND (Active-Low)
            if (stableButtonState == LOW) {
                pressDetected = true;
            }
        }
    }

    return pressDetected;
}

void updateSequencer(uint32_t now) {
    if (now - lastStepTime >= getPatternIntervalMs(currentPattern)) {
        lastStepTime = now;

        uint8_t mask = computePatternMask(currentPattern, patternStep);
        applyLedMask(mask);

        uint16_t stepCount = getPatternStepCount(currentPattern);
        patternStep = (stepCount > 0) ? ((patternStep + 1) % stepCount) : 0;
    }
}

void setup() {
    Serial.begin(115200);

    // Configure LED output pins
    for (uint8_t i = 0; i < 8; ++i) {
        pinMode(LED_PINS[i], OUTPUT);
        digitalWrite(LED_PINS[i], LOW);
    }

    // Active-Low pushbutton with internal pull-up
    pinMode(PIN_BUTTON, INPUT_PULLUP);

    Serial.println("[SYSTEM] 001-led-pattern-sequencer Initialized");
    transitionToPattern(Pattern::SCAN);
}

void loop() {
    uint32_t now = millis();

    // 1. Process asynchronous pushbutton input
    if (checkButtonPress()) {
        uint8_t nextIndex = (static_cast<uint8_t>(currentPattern) + 1) % static_cast<uint8_t>(Pattern::COUNT);
        Serial.println("[EVENT] Pushbutton pressed: Cycling pattern");
        transitionToPattern(static_cast<Pattern>(nextIndex));
    }

    // 2. Advance non-blocking pattern sequencer
    updateSequencer(now);
}