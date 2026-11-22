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

struct TodoItem {
  char itemName[21];
  bool isCompleted;

  void toggle() {
    isCompleted = !isCompleted;
  }
};

// Populating the todo list
TodoItem item0 = {"Pray Fajr", false};
TodoItem item1 = {"Take Walk", false};
TodoItem item2 = {"Pray Dhuhr", false};
TodoItem item3 = {"Take Nap", false};
TodoItem item4 = {"Pray Asr", false};
TodoItem item5 = {"Exercise", false};
TodoItem item6 = {"Pray Magrib", false};
TodoItem item7 = {"Do Homework", false};
TodoItem item8 = {"Pray Isha", false};
TodoItem item9 = {"Read Quran", false};

const uint8_t TODO_LIST_LENGTH = 10;
TodoItem todoList[TODO_LIST_LENGTH] = {
  item0, item1, item2, item3, item4,
  item5, item6, item7, item8, item9
};

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
