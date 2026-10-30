#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// GPIO Pin Definitions matching diagram.json
constexpr uint8_t PIN_POT     = 34; // ADC1 Channel 6 (Potentiometer Wiper)
constexpr uint8_t PIN_DATA    = 5;  // 74HC595 DS (Serial Data, Orange wire)
constexpr uint8_t PIN_LATCH   = 18; // 74HC595 STCP (Storage Latch Clock, Purple wire)
constexpr uint8_t PIN_CLOCK   = 19; // 74HC595 SHCP (Shift Register Clock, Yellow wire)
constexpr uint8_t PIN_LCD_SDA = 23; // I2C Serial Data
constexpr uint8_t PIN_LCD_SCL = 22; // I2C Serial Clock

// Timing Constants (milliseconds)
constexpr uint32_t ADC_SAMPLE_INTERVAL_MS  = 20;   // 50Hz ADC sampling rate
constexpr uint32_t LCD_UPDATE_INTERVAL_MS  = 100;  // 10Hz display refresh rate
constexpr uint32_t UART_REPORT_INTERVAL_MS = 1000; // 1Hz telemetry heartbeat

// ADC and Signal Conditioning Constants
constexpr uint32_t ADC_MAX_COUNTS = 4095;
constexpr uint32_t VREF_MV        = 3300;
constexpr int ADC_DEADBAND_LOW    = 30;   // Rail noise floor near GND
constexpr int ADC_RAIL_HIGH       = 4050; // Physical contact margin near 3V3

// Exponential Moving Average (EMA) Smoothing Factor
// Alpha determines weighting: higher values track faster, lower values filter noise heavily.
// At 50Hz sampling, alpha = 0.15 provides smooth response with ~130ms effective time constant.
constexpr float EMA_ALPHA = 0.15f;

// LCD1602 Display (PCF8574 default I2C address 0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Telemetry State Variables
float filteredAdc = 0.0f;
int rawAdcValue   = 0;
uint32_t rawMilliVolts      = 0;
uint32_t filteredMilliVolts = 0;
uint8_t currentLedMask      = 0x00;

uint32_t lastAdcSampleTime  = 0;
uint32_t lastLcdUpdateTime  = 0;
uint32_t lastUartReportTime = 0;

// Updates the 74HC595 shift register outputs atomically
void updateShiftRegister(uint8_t bitmask) {
    digitalWrite(PIN_LATCH, LOW);
    shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, bitmask);
    digitalWrite(PIN_LATCH, HIGH);
}

// Converts ADC count to progressive 8-bit LED bar graph mask
uint8_t calculateBarGraphMask(int adcVal) {
    if (adcVal <= ADC_DEADBAND_LOW) {
        return 0x00; // All LEDs OFF at minimum
    }

    // Map active travel across 8 progressive levels
    uint32_t clamped = constrain(adcVal, ADC_DEADBAND_LOW, ADC_RAIL_HIGH);
    uint8_t litLeds = static_cast<uint8_t>(((clamped - ADC_DEADBAND_LOW) * 8) / (ADC_RAIL_HIGH - ADC_DEADBAND_LOW) + 1);
    if (litLeds > 8) {
        litLeds = 8;
    }

    return (litLeds >= 8) ? 0xFF : static_cast<uint8_t>((1U << litLeds) - 1U);
}

void sampleAdcAndFilter(uint32_t now) {
    if (now - lastAdcSampleTime < ADC_SAMPLE_INTERVAL_MS) {
        return;
    }
    lastAdcSampleTime = now;

    rawAdcValue = analogRead(PIN_POT);
    rawMilliVolts = (static_cast<uint32_t>(rawAdcValue) * VREF_MV) / ADC_MAX_COUNTS;

    // First sample initialization
    if (filteredAdc == 0.0f && rawAdcValue > 0) {
        filteredAdc = static_cast<float>(rawAdcValue);
    } else {
        // Discrete Exponential Moving Average (EMA) formula:
        // filtered = (alpha * raw) + ((1.0 - alpha) * filtered)
        filteredAdc = (EMA_ALPHA * static_cast<float>(rawAdcValue)) + ((1.0f - EMA_ALPHA) * filteredAdc);
    }

    filteredMilliVolts = (static_cast<uint32_t>(filteredAdc) * VREF_MV) / ADC_MAX_COUNTS;

    // Update 74HC595 shift register output
    uint8_t newMask = calculateBarGraphMask(static_cast<int>(filteredAdc));
    if (newMask != currentLedMask) {
        currentLedMask = newMask;
        updateShiftRegister(currentLedMask);
    }
}

void updateLcdTelemetry(uint32_t now) {
    if (now - lastLcdUpdateTime < LCD_UPDATE_INTERVAL_MS) {
        return;
    }
    lastLcdUpdateTime = now;

    char lineBuffer[17];

    // Line 1: Raw instantaneous voltage
    snprintf(lineBuffer, sizeof(lineBuffer), "RAW: %4u mV", rawMilliVolts);
    lcd.setCursor(0, 0);
    lcd.print(lineBuffer);

    // Line 2: EMA filtered voltage with active bar count
    uint8_t litCount = 0;
    for (uint8_t i = 0; i < 8; ++i) {
        if (currentLedMask & (1U << i)) {
            litCount++;
        }
    }
    snprintf(lineBuffer, sizeof(lineBuffer), "EMA: %4u mV [%u/8]", filteredMilliVolts, litCount);
    lcd.setCursor(0, 1);
    lcd.print(lineBuffer);
}

void reportUartTelemetry(uint32_t now) {
    if (now - lastUartReportTime < UART_REPORT_INTERVAL_MS) {
        return;
    }
    lastUartReportTime = now;

    char logMsg[80];
    snprintf(logMsg, sizeof(logMsg), "[TELEMETRY] RAW=%4u mV (%4d) | EMA=%4u mV (%4.1f) | LED_MASK=0x%02X",
             rawMilliVolts, rawAdcValue, filteredMilliVolts, filteredAdc, currentLedMask);
    Serial.println(logMsg);
}

void setup() {
    Serial.begin(115200);

    // Configure 74HC595 control pins
    pinMode(PIN_DATA, OUTPUT);
    pinMode(PIN_CLOCK, OUTPUT);
    pinMode(PIN_LATCH, OUTPUT);

    // Initialize shift register to all LEDs OFF
    digitalWrite(PIN_LATCH, LOW);
    digitalWrite(PIN_CLOCK, LOW);
    digitalWrite(PIN_DATA, LOW);
    updateShiftRegister(0x00);

    // Configure Potentiometer analog input
    pinMode(PIN_POT, INPUT);

    // Initialize I2C bus on GPIO 23 and 22, then initialize LCD1602
    Wire.begin(PIN_LCD_SDA, PIN_LCD_SCL);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("002-adc-pot-ema");
    lcd.setCursor(0, 1);
    lcd.print("Initializing...");
    delay(500); // Brief power-on splash for visual latching
    lcd.clear();

    Serial.println("[SYSTEM] 002-adc-pot-ema-filter Initialized");
    Serial.println("[SYSTEM] Architecture: 74HC595 Serial Expander + I2C LCD1602 + EMA Filter");
}

void loop() {
    uint32_t now = millis();

    // 1. High-frequency non-blocking ADC sample and filter (50Hz)
    sampleAdcAndFilter(now);

    // 2. Periodic LCD telemetry refresh (10Hz)
    updateLcdTelemetry(now);

    // 3. Periodic structured UART heartbeat (1Hz)
    reportUartTelemetry(now);
}
