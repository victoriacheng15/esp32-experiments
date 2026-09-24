#include <Arduino.h>

// GPIO Pin Definitions
constexpr uint8_t PIN_LED_RED    = 23;
constexpr uint8_t PIN_LED_YELLOW = 22;
constexpr uint8_t PIN_LED_GREEN  = 21;
constexpr uint8_t PIN_BUTTON     = 19;

// Timing Constants (milliseconds)
constexpr uint32_t DEBOUNCE_DELAY_MS   = 50;
constexpr uint32_t TRAFFIC_INTERVAL_MS = 800;
constexpr uint32_t STROBE_INTERVAL_MS  = 100;

// System States
enum class SystemState : uint8_t {
    IDLE = 0,
    TRAFFIC,
    STROBE,
    SOLID,
    COUNT
};

// State Machine Variables
SystemState currentState = SystemState::IDLE;
uint32_t stateTimer = 0;
uint8_t trafficStep = 0;
bool strobeState = false;

// Debounce Tracking Variables
int lastRawButtonReading = HIGH;
int stableButtonState = HIGH;
uint32_t lastDebounceTime = 0;

void setAllLeds(bool red, bool yellow, bool green) {
    digitalWrite(PIN_LED_RED, red ? HIGH : LOW);
    digitalWrite(PIN_LED_YELLOW, yellow ? HIGH : LOW);
    digitalWrite(PIN_LED_GREEN, green ? HIGH : LOW);
}

void transitionToState(SystemState newState) {
    currentState = newState;
    stateTimer = millis();
    trafficStep = 0;
    strobeState = false;

    // Direct entry action
    switch (currentState) {
        case SystemState::IDLE:
            setAllLeds(false, false, false);
            Serial.println("[STATE] -> IDLE (All OFF)");
            break;
        case SystemState::TRAFFIC:
            setAllLeds(true, false, false);
            Serial.println("[STATE] -> TRAFFIC (Sequential)");
            break;
        case SystemState::STROBE:
            setAllLeds(true, true, true);
            strobeState = true;
            Serial.println("[STATE] -> STROBE (Concurrent Blink)");
            break;
        case SystemState::SOLID:
            setAllLeds(true, true, true);
            Serial.println("[STATE] -> SOLID (All ON)");
            break;
        default:
            break;
    }
}

bool checkButtonPress() {
    int currentRaw = digitalRead(PIN_BUTTON);
    bool pressDetected = false;

    // Reset debounce timer if the raw reading bounced
    if (currentRaw != lastRawButtonReading) {
        lastDebounceTime = millis();
    }
    lastRawButtonReading = currentRaw;

    // Evaluate stability once elapsed time exceeds debounce threshold
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY_MS) {
        if (currentRaw != stableButtonState) {
            stableButtonState = currentRaw;

            // Falling edge on active-low input indicates a physical press
            if (stableButtonState == LOW) {
                pressDetected = true;
            }
        }
    }

    return pressDetected;
}

void handleTrafficState(uint32_t now) {
    if (now - stateTimer >= TRAFFIC_INTERVAL_MS) {
        stateTimer = now;
        trafficStep = (trafficStep + 1) % 3;

        switch (trafficStep) {
            case 0:
                setAllLeds(true, false, false);
                break;
            case 1:
                setAllLeds(false, true, false);
                break;
            case 2:
                setAllLeds(false, false, true);
                break;
        }
    }
}

void handleStrobeState(uint32_t now) {
    if (now - stateTimer >= STROBE_INTERVAL_MS) {
        stateTimer = now;
        strobeState = !strobeState;
        setAllLeds(strobeState, strobeState, strobeState);
    }
}

void updateStateMachine() {
    uint32_t now = millis();

    switch (currentState) {
        case SystemState::IDLE:
        case SystemState::SOLID:
            // Static states, no periodic updates required
            break;

        case SystemState::TRAFFIC:
            handleTrafficState(now);
            break;

        case SystemState::STROBE:
            handleStrobeState(now);
            break;

        default:
            transitionToState(SystemState::IDLE);
            break;
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_YELLOW, OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);

    // Active-low input using internal pull-up resistor
    pinMode(PIN_BUTTON, INPUT_PULLUP);

    transitionToState(SystemState::IDLE);
}

void loop() {
    // 1. Process asynchronous inputs (debounced button edge detection)
    if (checkButtonPress()) {
        uint8_t nextIndex = (static_cast<uint8_t>(currentState) + 1) % static_cast<uint8_t>(SystemState::COUNT);
        transitionToState(static_cast<SystemState>(nextIndex));
    }

    // 2. Execute non-blocking state behaviors
    updateStateMachine();
}