#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const uint8_t EEPROM_MAGIC_KEY = 0x05;
const uint16_t EEPROM_START_ADDR = 0;

const uint8_t OLED_I2C_ADDR = 0x3C;
const uint8_t DISPLAY_WIDTH = 128;
const uint8_t DISPLAY_HEIGHT = 64;
const int8_t DISPLAY_RESET_PIN = -1;

const uint8_t CHAR_WIDTH = 6;
const uint8_t CHAR_HEIGHT = 8;
const uint8_t TITLE_BAR_HEIGHT = 16;
const uint8_t SUMMARY_BAR_WIDTH = 32;
const uint8_t MARGIN = 4;

const uint8_t BUTTON_COUNT = 3;
const int8_t BUTTON_PINS[BUTTON_COUNT] = { 6, 5, 4 };

const uint32_t WIRE_TIMEOUT_MS = 3000;
const uint8_t INPUT_INTERVAL_MS = 200;

const uint8_t BUZZER_PIN = 12;

const uint16_t INTERACTION_HZ = 2000;
const uint8_t BUZZER_DURATION = 180;  // 90% of INPUT_INTERVAL_MS

const uint8_t TITLE_TEXT_LENGTH = 16;
const uint8_t TASK_COUNT = 10;
const uint8_t VISIBLE_TASK_COUNT = 3;

struct Task {
  char title[TITLE_TEXT_LENGTH];
  bool completed;

  void toggle() {
    completed = !completed;
  }
};

struct Data {
  uint8_t magicKey;
  Task tasks[TASK_COUNT];
};

Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, DISPLAY_RESET_PIN);

Data taskData;

bool isDisplayReady = false;
bool needsRedraw = false;
uint32_t lastInputCheckMs = 0;
uint8_t taskCompletedCount = 0;
uint8_t selectedTaskIndex = 0;

void initButtons();
void initOled();
void loadTaskDataFromEEPROM();
void saveTaskDataToEEPROM();
void setDefaultTask(uint8_t index, const char* title, bool completed);
void handleButtonInputs();
void updateTaskCompletedCount();
uint8_t getScrollStartIndex();
void renderDisplay();
void renderHeader();
void renderTaskList(uint8_t scrollStartIndex);
void renderSummaryBar();
void drawCenteredText(const __FlashStringHelper* text, uint8_t textSize, uint8_t boxX, uint8_t boxY, uint8_t boxWidth, uint8_t boxHeight);
void drawCenteredText(const char* text, uint8_t textSize, uint8_t boxX, uint8_t boxY, uint8_t boxWidth, uint8_t boxHeight);
void drawTextAt(const char* text, uint8_t textSize, uint8_t x, uint8_t y);
void playInteractionSound();

void setup() {
  Wire.begin();
  Wire.setWireTimeout(WIRE_TIMEOUT_MS, true);

  loadTaskDataFromEEPROM();
  updateTaskCompletedCount();

  pinMode(BUZZER_PIN, OUTPUT);
  initButtons();
  initOled();
}

void loop() {
  uint32_t nowMs = millis();

  if (nowMs - lastInputCheckMs >= INPUT_INTERVAL_MS) {
    lastInputCheckMs = nowMs;
    handleButtonInputs();
  }

  if (isDisplayReady && needsRedraw) {
    renderDisplay();
    needsRedraw = false;
  }
}

void initButtons() {
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
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

void setDefaultTask(uint8_t index, const char* title, bool completed) {
  if (index >= TASK_COUNT) {
    return;
  }

  strncpy(taskData.tasks[index].title, title, TITLE_TEXT_LENGTH - 1);
  taskData.tasks[index].title[TITLE_TEXT_LENGTH - 1] = '\0';
  taskData.tasks[index].completed = completed;
}

void loadTaskDataFromEEPROM() {
  EEPROM.get(EEPROM_START_ADDR, taskData);

  if (taskData.magicKey != EEPROM_MAGIC_KEY) {
    taskData.magicKey = EEPROM_MAGIC_KEY;

    setDefaultTask(0, "Pray Fajr", false);
    setDefaultTask(1, "Take Walk", false);
    setDefaultTask(2, "Pray Dhuhr", false);
    setDefaultTask(3, "Take Nap", false);
    setDefaultTask(4, "Pray Asr", false);
    setDefaultTask(5, "Exercise", false);
    setDefaultTask(6, "Pray Magrib", false);
    setDefaultTask(7, "Do Homework", false);
    setDefaultTask(8, "Pray Isha", false);
    setDefaultTask(9, "Read Quran", false);

    saveTaskDataToEEPROM();
  }
}

void saveTaskDataToEEPROM() {
  EEPROM.put(EEPROM_START_ADDR, taskData);
}

void handleButtonInputs() {
  if (digitalRead(BUTTON_PINS[0]) == LOW) {
    playInteractionSound();
    if (selectedTaskIndex > 0) {
      selectedTaskIndex--;
    }
    needsRedraw = true;
  } else if (digitalRead(BUTTON_PINS[1]) == LOW) {
    playInteractionSound();
    taskData.tasks[selectedTaskIndex].toggle();
    updateTaskCompletedCount();
    saveTaskDataToEEPROM();
    needsRedraw = true;
  } else if (digitalRead(BUTTON_PINS[2]) == LOW) {
    playInteractionSound();
    if (selectedTaskIndex < TASK_COUNT - 1) {
      selectedTaskIndex++;
    }
    needsRedraw = true;
  }
}

void updateTaskCompletedCount() {
  uint8_t count = 0;

  for (uint8_t i = 0; i < TASK_COUNT; i++) {
    if (taskData.tasks[i].completed) {
      count++;
    }
  }

  taskCompletedCount = count;
}

uint8_t getScrollStartIndex() {
  int8_t startIndex = static_cast<int8_t>(selectedTaskIndex) - 1;
  int8_t maxStartIndex = static_cast<int8_t>(TASK_COUNT - VISIBLE_TASK_COUNT);

  if (startIndex < 0) {
    return 0;
  }

  if (startIndex > maxStartIndex) {
    return static_cast<uint8_t>(maxStartIndex);
  }

  return static_cast<uint8_t>(startIndex);
}

void renderDisplay() {
  if (!isDisplayReady) {
    return;
  }

  display.clearDisplay();

  renderHeader();
  renderTaskList(getScrollStartIndex());
  renderSummaryBar();

  display.display();
}

void renderHeader() {
  display.drawRect(0, 0, DISPLAY_WIDTH, TITLE_BAR_HEIGHT, SSD1306_WHITE);
  drawCenteredText(F("To-Do List"), 1, 0, 0, DISPLAY_WIDTH, TITLE_BAR_HEIGHT);
}

void renderTaskList(uint8_t scrollStartIndex) {
  const uint8_t contentHeight = DISPLAY_HEIGHT - TITLE_BAR_HEIGHT;
  const uint8_t taskAreaWidth = DISPLAY_WIDTH - SUMMARY_BAR_WIDTH;
  const uint8_t rowHighlightWidth = taskAreaWidth - (MARGIN * 2);

  display.drawRect(0, TITLE_BAR_HEIGHT, DISPLAY_WIDTH, contentHeight, SSD1306_WHITE);

  for (uint8_t i = 0; i < VISIBLE_TASK_COUNT; i++) {
    uint8_t taskIndex = scrollStartIndex + i;
    if (taskIndex >= TASK_COUNT) {
      break;
    }

    const Task& task = taskData.tasks[taskIndex];
    uint8_t rowTopY = TITLE_BAR_HEIGHT + MARGIN + (CHAR_HEIGHT * i * 2);

    char lineBuffer[TITLE_TEXT_LENGTH + 6];
    snprintf(lineBuffer, sizeof(lineBuffer), "%u. %s", taskIndex + 1, task.title);
    drawTextAt(lineBuffer, 1, MARGIN * 2, rowTopY);

    if (taskIndex == selectedTaskIndex) {
      display.drawRect(MARGIN, rowTopY - 1, rowHighlightWidth, CHAR_HEIGHT + 2, SSD1306_WHITE);
    }

    if (task.completed) {
      display.drawFastHLine(MARGIN, rowTopY + (CHAR_HEIGHT / 2), rowHighlightWidth, SSD1306_WHITE);
    }
  }
}

void renderSummaryBar() {
  const uint8_t barX = DISPLAY_WIDTH - SUMMARY_BAR_WIDTH - 1;
  const uint8_t barHeight = DISPLAY_HEIGHT - TITLE_BAR_HEIGHT;

  display.drawRect(barX, TITLE_BAR_HEIGHT, 2, barHeight, SSD1306_WHITE);

  char ratioBuffer[10];
  snprintf(ratioBuffer, sizeof(ratioBuffer), "%u/%u", taskCompletedCount, TASK_COUNT);
  drawCenteredText(ratioBuffer, 1, barX + 2, TITLE_BAR_HEIGHT, SUMMARY_BAR_WIDTH - 1, barHeight);
}

void drawCenteredText(const __FlashStringHelper* text, uint8_t textSize, uint8_t boxX, uint8_t boxY, uint8_t boxWidth, uint8_t boxHeight) {
  uint8_t textLength = strlen_P(reinterpret_cast<PGM_P>(text));
  uint8_t textPixelWidth = CHAR_WIDTH * textSize * textLength;
  uint8_t textPixelHeight = CHAR_HEIGHT * textSize;

  uint8_t cursorX = boxX + ((boxWidth > textPixelWidth) ? (boxWidth - textPixelWidth) / 2 : 0);
  uint8_t cursorY = boxY + ((boxHeight > textPixelHeight) ? (boxHeight - textPixelHeight) / 2 : 0);

  display.setTextSize(textSize);
  display.setCursor(cursorX, cursorY);
  display.print(text);
}

void drawCenteredText(const char* text, uint8_t textSize, uint8_t boxX, uint8_t boxY, uint8_t boxWidth, uint8_t boxHeight) {
  uint8_t textLength = strlen(text);
  uint8_t textPixelWidth = CHAR_WIDTH * textSize * textLength;
  uint8_t textPixelHeight = CHAR_HEIGHT * textSize;

  uint8_t cursorX = boxX + ((boxWidth > textPixelWidth) ? (boxWidth - textPixelWidth) / 2 : 0);
  uint8_t cursorY = boxY + ((boxHeight > textPixelHeight) ? (boxHeight - textPixelHeight) / 2 : 0);

  display.setTextSize(textSize);
  display.setCursor(cursorX, cursorY);
  display.print(text);
}

void drawTextAt(const char* text, uint8_t textSize, uint8_t x, uint8_t y) {
  display.setTextSize(textSize);
  display.setCursor(x, y);
  display.print(text);
}

void playInteractionSound() {
  tone(BUZZER_PIN, INTERACTION_HZ, BUZZER_DURATION);
}
