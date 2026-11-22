#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const uint8_t SCREEN_ADDR = 0x3C;
const uint8_t SCREEN_WIDTH = 128;
const uint8_t SCREEN_HEIGHT = 64;
const int8_t OLED_RESET = -1;

const uint8_t BTN_AMOUNT = 3;
const int8_t BTN_PINS[BTN_AMOUNT] = { 6, 5, 4 };

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool oledConnected = false;

const uint8_t DEBOUNCE_MS = 50;

void initOled();
void updateDisplay();

void setup() {
  Wire.begin();
  Wire.setWireTimeout(3000, true);

  for (uint8_t i = 0; i < BTN_AMOUNT; i++) {
    pinMode(BTN_PINS[i], INPUT_PULLUP);
  }

  initOled();
}

void loop() {
  for (uint8_t i = 0; i < BTN_AMOUNT; i++) {
    if (digitalRead(BTN_PINS[i]) == LOW) {
      updateDisplay();
    }
  }

  delay(DEBOUNCE_MS);
}

void initOled() {
  if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDR)) {
    oledConnected = true;
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(3);
    updateDisplay();
  }
}

void updateDisplay() {
  if (!oledConnected) return;

  display.clearDisplay();
  display.setCursor(0, 0);
  
  for (uint8_t i = 0; i < BTN_AMOUNT; i++) {
    if (digitalRead(BTN_PINS[i]) == LOW) {
      display.println(BTN_PINS[i]);
    }
  }

  display.display();
}
