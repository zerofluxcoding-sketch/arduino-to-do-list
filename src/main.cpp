#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const uint8_t OLED_I2C_ADDR = 0x3C;
const uint8_t DISPLAY_WIDTH = 128;
const uint8_t DISPLAY_HEIGHT = 64;
const int8_t DISPLAY_RESET_PIN = -1;

const uint8_t CHAR_WIDTH = 6;
const uint8_t CHAR_HEIGHT = 8;
const uint8_t TITLE_BAR_HEIGHT = 16;

const uint8_t BUTTON_COUNT = 3;
const int8_t BUTTON_PINS[BUTTON_COUNT] = { 6, 5, 4 };

Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, DISPLAY_RESET_PIN);

bool isDisplayReady = false;
bool needsRedraw = false;

uint32_t lastInputCheckMs = 0;
const uint8_t INPUT_INTERVAL_MS = 100;

struct TodoItem {
  char title[12];
  bool completed;

  void toggle() {
    completed = !completed;
  }
};

// Populating the todo list
const uint8_t TASK_COUNT = 10;
TodoItem tasks[TASK_COUNT] = {
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
uint8_t selectedTaskIndex = 0;
const uint8_t VISIBLE_TASK_COUNT = 3;

void initOled();
void handleButtonInputs();
void renderDisplay();
void printCenteredText(const __FlashStringHelper* label, uint8_t textSize, uint8_t areaWidth, uint8_t boxTopY, bool alignCenterX, bool alignCenterY);
void printCenteredText(char* label, uint8_t textSize, uint8_t areaWidth, uint8_t boxTopY, bool alignCenterX, bool alignCenterY);

void setup() {
  Wire.begin();
  Wire.setWireTimeout(3000, true);

  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }

  initOled();
}

void loop() {
  uint32_t nowMs = millis();
  if (nowMs - lastInputCheckMs >= INPUT_INTERVAL_MS) {
    lastInputCheckMs = nowMs;
    handleButtonInputs();
  }

  if (needsRedraw) {
    renderDisplay();
    needsRedraw = false;
  }
}

void initOled() {
  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    isDisplayReady = true;
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    renderDisplay();
  }
}

void handleButtonInputs() {
  if (digitalRead(BUTTON_PINS[0]) == LOW) {
    if (selectedTaskIndex > 0) selectedTaskIndex--;
    needsRedraw = true;
  } else if (digitalRead(BUTTON_PINS[1]) == LOW) {
    TodoItem &item = tasks[selectedTaskIndex];
    item.toggle();
    needsRedraw = true;
  } else if (digitalRead(BUTTON_PINS[2]) == LOW) {
    if (selectedTaskIndex < TASK_COUNT - 1) selectedTaskIndex++;
    needsRedraw = true;
  }
}

void renderDisplay() {
  if (!isDisplayReady) return;

  display.clearDisplay();

  display.setCursor(0, 0);
  printCenteredText(F("To-Do List"), 1, DISPLAY_WIDTH, TITLE_BAR_HEIGHT, true, true);

  int8_t scrollStartIndex = selectedTaskIndex - 1;
  if (scrollStartIndex < VISIBLE_TASK_COUNT - 2) {
    scrollStartIndex = 0;
  } else if (scrollStartIndex > TASK_COUNT - VISIBLE_TASK_COUNT) {
    scrollStartIndex = 7;
  }

  for (uint8_t i = 0; i < VISIBLE_TASK_COUNT; i++) {
    TodoItem item = tasks[scrollStartIndex + i];

    uint8_t rowTopY = TITLE_BAR_HEIGHT + 10 * i;
    printCenteredText(item.title, 1, DISPLAY_WIDTH, rowTopY, true, false);

    if (scrollStartIndex + i == selectedTaskIndex) {
      display.drawRect(0, rowTopY, DISPLAY_WIDTH, 10, SSD1306_WHITE);
    }

    if (item.completed) {
      display.drawFastHLine(0, rowTopY + CHAR_HEIGHT / 2, DISPLAY_WIDTH, WHITE);
    }
  }

  display.display();
}

void printCenteredText(const __FlashStringHelper* label, uint8_t textSize, uint8_t areaWidth, uint8_t boxTopY, bool alignCenterX, bool alignCenterY) {
  uint8_t textLength = strlen_P((PGM_P)label);
  uint8_t cursorY, cursorX;

  if (alignCenterX) {
    cursorX = (areaWidth - (CHAR_WIDTH * textSize * textLength)) / 2;
  } else if (!alignCenterX) {
    cursorX = areaWidth;
  }

  if (alignCenterY) {
    cursorY = (boxTopY - (CHAR_HEIGHT * textSize)) / 2;
  } else if (!alignCenterX) {
    cursorY = boxTopY;
  }

  display.setTextSize(textSize);
  display.setCursor(cursorX, cursorY);
  display.println(label);
}

void printCenteredText(char* label, uint8_t textSize, uint8_t areaWidth, uint8_t boxTopY, bool alignCenterX, bool alignCenterY) {
  uint8_t textLength = strlen(label);

  uint8_t cursorX = alignCenterX ?
    (areaWidth - (CHAR_WIDTH * textSize * textLength)) / 2 : areaWidth;

  uint8_t cursorY = alignCenterY ?
    (boxTopY - (CHAR_HEIGHT * textSize)) / 2 : boxTopY;

  display.setTextSize(textSize);
  display.setCursor(cursorX, cursorY);
  display.println(label);
}
