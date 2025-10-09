#include <FastLED.h>

// --- Конфигурация ---
#define PIN1        6
#define PIN2        7
#define NUMPIXELS   256
#define BUTTON_PIN  2
#define BUTTON_PIN2 3
#define BUTTON_PIN3 4
#define BUTTON_PIN4 5

CRGB leds1[NUMPIXELS];
CRGB leds2[NUMPIXELS];

// --- Шрифт 8x8 ---
const byte digits[10][8] = {
  {B00111100,B01100110,B01101110,B01110110,B01100110,B01100110,B00111100,B00000000},
  {B00011000,B00111000,B00011000,B00011000,B00011000,B00011000,B00111100,B00000000},
  {B00111100,B01100110,B00000110,B00001100,B00011000,B01100000,B01111110,B00000000},
  {B00111100,B01100110,B00000110,B00011100,B00000110,B01100110,B00111100,B00000000},
  {B00001100,B00011100,B00101100,B01001100,B01111110,B00001100,B00001100,B00000000},
  {B01111110,B01100000,B01111100,B00000110,B00000110,B01100110,B00111100,B00000000},
  {B00111100,B01100110,B01100000,B01111100,B01100110,B01100110,B00111100,B00000000},
  {B01111110,B01100110,B00001100,B00011000,B00011000,B00011000,B00011000,B00000000},
  {B00111100,B01100110,B01100110,B00111100,B01100110,B01100110,B00111100,B00000000},
  {B00111100,B01100110,B01100110,B00111110,B00000110,B01100110,B00111100,B00000000}
};

// --- Цвета ---
CRGB normalColor = CRGB(0,150,0);
CRGB setupColor  = CRGB(0,0,150);
CRGB pauseColor  = CRGB(50,0,0);

// --- Тайминги и переменные ---
const unsigned long longPressTime = 4000;
int counterDefault = 20;
bool setupMode = false;
unsigned long pressStart3 = 0;
bool lastButton3 = HIGH;
unsigned long lastDebounce3 = 0;
const unsigned long setupTimeout = 4000;
unsigned long lastSetupAction = 0;

int counter1 = counterDefault;
int counter2 = counterDefault;
bool running1 = false;
bool running2 = false;

bool lastButton1 = HIGH;
bool lastButton2 = HIGH;
unsigned long lastDebounce1 = 0;
unsigned long lastDebounce2 = 0;
const unsigned long debounceDelay = 50;

unsigned long lastUpdate1 = 0;
unsigned long lastUpdate2 = 0;
const unsigned long interval = 1000;

bool lastButton4 = HIGH;
unsigned long lastDebounce4 = 0;

// --- Функции ---
// Индексация 16×16
int xy2index16(int x, int y) {
  int bx = x / 8;
  int by = y / 8;
  int lx = x % 8;
  int ly = y % 8;
  int block = bx + by * 2;
  return block * 64 + ly * 8 + lx;
}

// Рисуем цифру
void drawDigit16(byte num, int xOffset, CRGB color, CRGB *leds) {
  for (int yg = 0; yg < 16; yg++) {
    int src_row = yg / 2;
    for (int x = 0; x < 8; x++) {
      bool on = (digits[num][src_row] & (1 << (7 - x)));
      int xi = x + xOffset;
      int idx = xy2index16(xi, yg);
      if (idx < NUMPIXELS) leds[idx] = on ? color : CRGB::Black;
    }
  }
}

// Рисуем двузначное число
void drawNumberSingle(int value, CRGB *leds, CRGB color) {
  int tens = value / 10;
  int ones = value % 10;
  fill_solid(leds, NUMPIXELS, CRGB::Black);
  drawDigit16(tens, 0, color, leds);
  drawDigit16(ones, 8, color, leds);
  FastLED.show();
}

// Мигание красным
void flashRed(CRGB *leds, int times) {
  for (int t = 0; t < times; t++) {
    fill_solid(leds, NUMPIXELS, CRGB(150,0,0));
    FastLED.show();
    delay(300);
    fill_solid(leds, NUMPIXELS, CRGB::Black);
    FastLED.show();
    delay(300);
  }
}

// Красный крест
void flashRedCross(CRGB *leds, int times) {
  int crossPixels[] = {
    0,1,9,10,18,19,27,28,36,37,45,46,54,55,63,
    192,193,201,202,210,211,219,220,228,229,237,238,246,247,255,
    70,71,77,78,84,85,91,92,98,99,105,106,112,113,120,
    134,135,141,142,148,149,155,156,162,163,169,170,176,177,184
  };
  int crossCount = sizeof(crossPixels) / sizeof(crossPixels[0]);
  
  for (int t = 0; t < times; t++) {
    fill_solid(leds, NUMPIXELS, CRGB::Black);
    for (int i = 0; i < crossCount; i++) leds[crossPixels[i]] = CRGB(150,0,0);
    FastLED.show();
    delay(300);
    fill_solid(leds, NUMPIXELS, CRGB::Black);
    FastLED.show();
    delay(300);
  }
}

// --- SETUP ---
void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN2, INPUT_PULLUP);
  pinMode(BUTTON_PIN3, INPUT_PULLUP);
  pinMode(BUTTON_PIN4, INPUT_PULLUP);

  FastLED.addLeds<NEOPIXEL, PIN1>(leds1, NUMPIXELS);
  FastLED.addLeds<NEOPIXEL, PIN2>(leds2, NUMPIXELS);
  FastLED.setBrightness(15);

  drawNumberSingle(counter1, leds1, CRGB(50,50,50));
  drawNumberSingle(counter2, leds2, CRGB(50,50,50));
}

// --- LOOP ---
void loop() {
  bool reading1 = digitalRead(BUTTON_PIN);
  if (reading1 != lastButton1) lastDebounce1 = millis();
  if ((millis() - lastDebounce1) > debounceDelay) {
    static bool buttonState1 = HIGH;
    if (reading1 != buttonState1) {
      buttonState1 = reading1;
      if (buttonState1 == LOW && !running2) {
        running2 = !running2;
        running1 = false;
      }
    }
  }
  lastButton1 = reading1;

  bool reading2 = digitalRead(BUTTON_PIN2);
  if (reading2 != lastButton2) lastDebounce2 = millis();
  if ((millis() - lastDebounce2) > debounceDelay) {
    static bool buttonState2 = HIGH;
    if (reading2 != buttonState2) {
      buttonState2 = reading2;
      if (buttonState2 == LOW && !running1) {
        running1 = !running1;
        running2 = false;
      }
    }
  }
  lastButton2 = reading2;

  unsigned long now = millis();

  // --- Таймер ленты 1 ---
  if (running1 && (now - lastUpdate1 >= interval)) {
    lastUpdate1 = now;
    counter1--;
    if (counter1 < 0) counter1 = 0;
    drawNumberSingle(counter1, leds1, normalColor);
    drawNumberSingle(counter2, leds2, running2 ? normalColor : pauseColor);

    if (counter1 == 0) {
      running1 = false;
      fill_solid(leds2, NUMPIXELS, CRGB::Black);
      FastLED.show();
      flashRedCross(leds1, 3);
      counter1 = counter2 = counterDefault;
      drawNumberSingle(counter1, leds1, CRGB(50,50,50));
      drawNumberSingle(counter2, leds2, CRGB(50,50,50));
    }
  }

  // --- Таймер ленты 2 ---
  if (running2 && (now - lastUpdate2 >= interval)) {
    lastUpdate2 = now;
    counter2--;
    if (counter2 < 0) counter2 = 0;
    drawNumberSingle(counter2, leds2, normalColor);
    drawNumberSingle(counter1, leds1, running1 ? normalColor : pauseColor);

    if (counter2 == 0) {
      running2 = false;
      fill_solid(leds1, NUMPIXELS, CRGB::Black);
      FastLED.show();
      flashRedCross(leds2, 3);
      counter1 = counter2 = counterDefault;
      drawNumberSingle(counter1, leds1, CRGB(50,50,50));
      drawNumberSingle(counter2, leds2, CRGB(50,50,50));
    }
  }

  // --- Кнопка 3: настройка ---
  bool reading3 = digitalRead(BUTTON_PIN3);
  if (reading3 != lastButton3) lastDebounce3 = millis();
  if ((millis() - lastDebounce3) > debounceDelay) {
    static bool buttonState3 = HIGH;
    if (reading3 != buttonState3) {
      buttonState3 = reading3;
      if (buttonState3 == LOW) pressStart3 = millis();
      else {
        unsigned long pressDuration = millis() - pressStart3;
        if (!setupMode && pressDuration >= longPressTime) {
          setupMode = true;
          lastSetupAction = millis();
          drawNumberSingle(counterDefault, leds1, setupColor);
          drawNumberSingle(counterDefault, leds2, setupColor);
        } else if (setupMode && pressDuration < longPressTime) {
          counterDefault += 10;
          if (counterDefault > 60) counterDefault = 10;
          drawNumberSingle(counterDefault, leds1, setupColor);
          drawNumberSingle(counterDefault, leds2, setupColor);
          lastSetupAction = millis();
        }
      }
    }
  }
  lastButton3 = reading3;

  if (setupMode && (millis() - lastSetupAction > setupTimeout)) {
    setupMode = false;
    counter1 = counter2 = counterDefault;
    drawNumberSingle(counter1, leds1, normalColor);
    drawNumberSingle(counter2, leds2, normalColor);
  }

  // --- Кнопка 4: сброс ---
  bool reading4 = digitalRead(BUTTON_PIN4);
  if (reading4 != lastButton4) lastDebounce4 = millis();
  if ((millis() - lastDebounce4) > debounceDelay) {
    static bool buttonState4 = HIGH;
    if (reading4 != buttonState4) {
      buttonState4 = reading4;
      if (buttonState4 == LOW) {
        running1 = running2 = false;
        counter1 = counter2 = counterDefault;
        fill_solid(leds1, NUMPIXELS, CRGB::Black);
        fill_solid(leds2, NUMPIXELS, CRGB::Black);
        FastLED.show();
        drawNumberSingle(counter1, leds1, normalColor);
        drawNumberSingle(counter2, leds2, normalColor);
      }
    }
  }
  lastButton4 = reading4;
}
