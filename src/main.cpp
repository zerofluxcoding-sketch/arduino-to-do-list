#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const uint8_t SCREEN_ADDR = 0x3C;
const uint8_t SCREEN_WIDTH = 128;
const uint8_t SCREEN_HEIGHT = 64;
const int8_t OLED_RESET = -1;

const uint8_t TEXT_WIDTH = 6;
const uint8_t TEXT_HEIGHT = 8;
const uint8_t HEADER_HEIGHT = 16;

const uint8_t BTN_AMOUNT = 3;
const int8_t BTN_PINS[BTN_AMOUNT] = { 6, 5, 4 };

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool oledConnected = false;

const uint8_t DEBOUNCE_MS = 100;

struct TodoItem {
  char itemName[12];
  bool isCompleted;

  void toggle() {
    isCompleted = !isCompleted;
  }
};

// Populating the todo list
const uint8_t TODO_LIST_LENGTH = 10;
TodoItem todoList[TODO_LIST_LENGTH] = {
  {"Pray Fajr", false},
  {"Take Walk", false},
  {"Pray Dhuhr", false},
  {"Take Nap", false},
  {"Pray Asr", false},
  {"Exercise", false},
  {"Pray Magrib", false},
  {"Do Homework", false},
  {"Pray Isha", false},
  {"Read Quran", false}
};
uint8_t todoListIndex = 0;
const uint8_t MAX_ITEM_DISPLAY = 3;

void initOled();
void handleButtonInputs();
void renderDisplay();

void setup() {
  Wire.begin();
  Wire.setWireTimeout(3000, true);

  for (uint8_t i = 0; i < BTN_AMOUNT; i++) {
    pinMode(BTN_PINS[i], INPUT_PULLUP);
  }

  initOled();
}

void loop() {
  handleButtonInputs();
  renderDisplay();

  delay(DEBOUNCE_MS);
}

void initOled() {
  if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDR)) {
    oledConnected = true;
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    renderDisplay();
  }
}

void handleButtonInputs() {
  if (digitalRead(BTN_PINS[0]) == LOW) {
    if (todoListIndex > 0) todoListIndex--;
  } else if (digitalRead(BTN_PINS[1]) == LOW) {
    TodoItem &item = todoList[todoListIndex];
    item.toggle();
  } else if (digitalRead(BTN_PINS[2]) == LOW) {
    if (todoListIndex < TODO_LIST_LENGTH - 1) todoListIndex++;
  }
}

void renderDisplay() {
  if (!oledConnected) return;

  display.clearDisplay();

  display.setCursor(0, 0);
  display.println(F("To-Do List"));

  int8_t targetIndex = todoListIndex - 1;
  if (targetIndex < MAX_ITEM_DISPLAY - 2) {
    targetIndex = 0;
  } else if (targetIndex > TODO_LIST_LENGTH - MAX_ITEM_DISPLAY) {
    targetIndex = 7;
  }

  for (uint8_t i = 0; i < MAX_ITEM_DISPLAY; i++) {
    TodoItem item = todoList[targetIndex + i];

    uint8_t yOffset = HEADER_HEIGHT + 10 * i;
    display.setCursor(0, yOffset);
    display.println(item.itemName);

    if (targetIndex + i == todoListIndex) {
      display.drawRect(0, yOffset, SCREEN_WIDTH, 10, SSD1306_WHITE);
    }

    if (item.isCompleted) {
      display.drawFastHLine(0, yOffset + TEXT_HEIGHT / 2, SCREEN_WIDTH, WHITE);
    }
  }

  display.display();
}
