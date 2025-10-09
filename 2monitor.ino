#include <Adafruit_NeoPixel.h>

#define PIN1        6        // первая лента
#define PIN2        7        // вторая лента
#define NUMPIXELS   256
#define BUTTON_PIN  2        // кнопка 1
#define BUTTON_PIN2 3        // кнопка 2
//setup
#define BUTTON_PIN3 4      // кнопка 3 - setup
#define BUTTON_PIN4 5      // кнопка 4 — сброс

Adafruit_NeoPixel strip1(NUMPIXELS, PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(NUMPIXELS, PIN2, NEO_GRB + NEO_KHZ800);

// --- Шрифт 8x8 для цифр 0–9 ---
const byte digits[10][8] = {
  {B00111100,B01100110,B01101110,B01110110,B01100110,B01100110,B00111100,B00000000}, //0
  {B00011000,B00111000,B00011000,B00011000,B00011000,B00011000,B00111100,B00000000}, //1
  {B00111100,B01100110,B00000110,B00001100,B00011000,B01100000,B01111110,B00000000}, //2
  {B00111100,B01100110,B00000110,B00011100,B00000110,B01100110,B00111100,B00000000}, //3
  {B00001100,B00011100,B00101100,B01001100,B01111110,B00001100,B00001100,B00000000}, //4
  {B01111110,B01100000,B01111100,B00000110,B00000110,B01100110,B00111100,B00000000}, //5
  {B00111100,B01100110,B01100000,B01111100,B01100110,B01100110,B00111100,B00000000}, //6
  {B01111110,B01100110,B00001100,B00011000,B00011000,B00011000,B00011000,B00000000}, //7
  {B00111100,B01100110,B01100110,B00111100,B01100110,B01100110,B00111100,B00000000}, //8
  {B00111100,B01100110,B01100110,B00111110,B00000110,B01100110,B00111100,B00000000}  //9
};
//setup
uint32_t normalColor;   // зелёный
uint32_t setupColor;    // синий
uint32_t pauseColor;   // красный

const unsigned long longPressTime = 4000;  // 4 сек для входа в настройку

int counterDefault = 20;   // значение по умолчанию
bool setupMode = false;    // режим настройки
unsigned long pressStart3 = 0;
bool lastButton3 = HIGH;
unsigned long lastDebounce3 = 0;
const unsigned long setupTimeout = 4000;   // таймаут выхода
unsigned long lastSetupAction = 0;

// --- Переменные таймеров и кнопок ---
int counter1 = counterDefault;
int counter2 = counterDefault;
bool running1 = false;   // лента 1
bool running2 = false;   // лента 2

bool lastButton1 = HIGH;
bool lastButton2 = HIGH;
unsigned long lastDebounce1 = 0;
unsigned long lastDebounce2 = 0;
const unsigned long debounceDelay = 50;

unsigned long lastUpdate1 = 0;
unsigned long lastUpdate2 = 0;
const unsigned long interval = 1000; // 1 секунда

bool lastButton4 = HIGH;
unsigned long lastDebounce4 = 0;

// ---------- Индексация 16x16 ----------
int xy2index16(int x, int y) {
  int bx = x / 8;
  int by = y / 8;
  int lx = x % 8;
  int ly = y % 8;
  int block = bx + by * 2;
  return block * 64 + ly * 8 + lx;
}

// ---------- Рисуем большую цифру ----------
void drawDigit16(byte num, int xOffset, uint32_t color, Adafruit_NeoPixel &s) {
  for (int yg = 0; yg < 16; yg++) {
    int src_row = yg / 2;
    for (int x = 0; x < 8; x++) {
      bool on = (digits[num][src_row] & (1 << (7 - x)));
      int xi = x + xOffset;
      int idx = xy2index16(xi, yg);
      s.setPixelColor(idx, on ? color : 0);
    }
  }
}

// ---------- Рисуем число на выбранной ленте ----------
//void drawNumberSingle(int value, Adafruit_NeoPixel &s) {
void drawNumberSingle(int value, Adafruit_NeoPixel &s, uint32_t color) {

 // uint32_t color = s.Color(0,150,0);
  int tens = value / 10;
  int ones = value % 10;
  s.clear();
  drawDigit16(tens, 0, color, s);
  drawDigit16(ones, 8, color, s);
  s.show();
}

// ---------- Красное мигание ----------
void flashRed(Adafruit_NeoPixel &s, int times) {
  for (int t = 0; t < times; t++) {
    for (int i = 0; i < NUMPIXELS; i++) s.setPixelColor(i, s.Color(150,0,0));
    s.show();
    delay(300);
    s.clear();
    s.show();
    delay(300);
  }
}

// ---------- Красное мигание крестика ----------
void flashRedCross(Adafruit_NeoPixel &s, int times) {
  // координаты пикселей, которые формируют крест
  // ЗАМЕНИ на реальные индексы твоего шрифта/матрицы!
  int crossPixels[] = {0, 1, 9, 10, 18, 19, 27, 28, 36, 37, 45, 46, 54, 55, 63, 
                        192, 193,201, 202, 210,211,219,220, 228,229,237,238,246,247,255,// диагональ ↘
                       70, 71, 77,78, 84,85, 91,92, 98,99, 105,106, 112,113, 120, 
                       134,135, 141,142, 148,149, 155,156, 162,163, 169,170, 176,177,184}; // диагональ ↙
  int crossCount = sizeof(crossPixels) / sizeof(crossPixels[0]);
s.clear();
    s.show();
  for (int t = 0; t < times; t++) {
    // зажигаем крест красным
    for (int k = 0; k < crossCount; k++) {
      s.setPixelColor(crossPixels[k], s.Color(150, 0, 0));
    }
    s.show();
    delay(300);

    s.clear();
    s.show();
    delay(300);
  }
}

// ---------- SETUP ----------
void setup() {
  normalColor = strip1.Color(0,150,0);
  setupColor  = strip1.Color(0,0,150);
  pauseColor  = strip1.Color(50,0,0);   // красный
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN2, INPUT_PULLUP);
  pinMode(BUTTON_PIN3, INPUT_PULLUP);
  pinMode(BUTTON_PIN4, INPUT_PULLUP);
  strip1.begin();
  strip2.begin();
  strip1.setBrightness(15);
  strip2.setBrightness(15);
  strip1.show();
  strip2.show();
  drawNumberSingle(counter1, strip1, strip1.Color(50,50,50));
  drawNumberSingle(counter2, strip2, strip1.Color(50,50,50));
}

// ---------- LOOP ----------
void loop() {
  // --- кнопка 1 (D2) управляет лентой 2 ---
  bool reading1 = digitalRead(BUTTON_PIN);
  if (reading1 != lastButton1) lastDebounce1 = millis();
  if ((millis() - lastDebounce1) > debounceDelay) {
    static bool buttonState1 = HIGH;
    if (reading1 != buttonState1) {
      buttonState1 = reading1;
      if (buttonState1 == LOW && !running2) {
        running2 = !running2;  // переключаем ленту 2
        running1 = false;      // останавливаем ленту 1
      }
    }
  }
  lastButton1 = reading1;

  // --- кнопка 2 (D3) управляет лентой 1 ---
  bool reading2 = digitalRead(BUTTON_PIN2);
  if (reading2 != lastButton2) lastDebounce2 = millis();
  if ((millis() - lastDebounce2) > debounceDelay) {
    static bool buttonState2 = HIGH;
    if (reading2 != buttonState2) {
      buttonState2 = reading2;
//      if (buttonState2 == LOW) {
      if (buttonState2 == LOW && !running1) {
        running1 = !running1;  // переключаем ленту 1
        running2 = false;      // останавливаем ленту 2
      }
    }
  }
  lastButton2 = reading2;

  unsigned long now = millis();

  // --- таймер ленты 1 ---
  if (running1 && (now - lastUpdate1 >= interval)) {
    lastUpdate1 = now;
    counter1--;
    if (counter1 < 0) counter1 = 0;
    drawNumberSingle(counter1, strip1, running1 ? normalColor : pauseColor);
    drawNumberSingle(counter2, strip2, running2 ? normalColor : pauseColor);

    if (counter1 == 0) {
      running1 = false;
//      flashRed(strip1, 3);
      strip2.clear();
      strip2.show();
      flashRedCross(strip1, 3);
      counter1 = counterDefault;
      counter2 = counterDefault;
      drawNumberSingle(counter1, strip1, strip1.Color(50,50,50));
      drawNumberSingle(counter2, strip2, strip1.Color(50,50,50));
    }
  }

  // --- таймер ленты 2 ---
  if (running2 && (now - lastUpdate2 >= interval)) {
    lastUpdate2 = now;
    counter2--;
    if (counter2 < 0) counter2 = 0;
    drawNumberSingle(counter2, strip2, running2 ? normalColor : pauseColor);
    drawNumberSingle(counter1, strip1, running1 ? normalColor : pauseColor);

    if (counter2 == 0) {
      running2 = false;
//      flashRed(strip2, 3);
      strip1.clear();
      strip1.show();
      flashRedCross(strip2, 3);
      counter1 = counterDefault;
      counter2 = counterDefault;
      drawNumberSingle(counter1, strip1, strip1.Color(50,50,50));
      drawNumberSingle(counter2, strip2, strip1.Color(50,50,50));
    }
  }

  // --- кнопка 3: настройка значения counterDefault ---
bool reading3 = digitalRead(BUTTON_PIN3);
if (reading3 != lastButton3) lastDebounce3 = millis();
if ((millis() - lastDebounce3) > debounceDelay) {
  static bool buttonState3 = HIGH;
  if (reading3 != buttonState3) {
    buttonState3 = reading3;

    // ---- нажатие ----
    if (buttonState3 == LOW) {
      pressStart3 = millis();
    }
    // ---- отпускание ----
    else {
      unsigned long pressDuration = millis() - pressStart3;

      if (!setupMode && pressDuration >= longPressTime) {
        // вход в режим настройки
        setupMode = true;
        lastSetupAction = millis();
        // показать текущее значение на обеих лентах
        drawNumberSingle(counterDefault, strip1, setupColor);
        drawNumberSingle(counterDefault, strip2, setupColor);
      }
      else if (setupMode && pressDuration < longPressTime) {
        // короткое нажатие в режиме настройки -> следующий шаг
        counterDefault += 10;
        if (counterDefault > 60) counterDefault = 10;
        drawNumberSingle(counterDefault, strip1, setupColor);
        drawNumberSingle(counterDefault, strip2, setupColor);
        lastSetupAction = millis();
      }
    }
  }
}
lastButton3 = reading3;

// --- авто-выход из режима настройки ---
if (setupMode && (millis() - lastSetupAction > setupTimeout)) {
  setupMode = false;
  // применяем новое значение
  counter1 = counterDefault;
  counter2 = counterDefault;
  drawNumberSingle(counter1, strip1, normalColor);
  drawNumberSingle(counter2, strip2, normalColor);
}

// --- кнопка 4: сброс ---
bool reading4 = digitalRead(BUTTON_PIN4);
if (reading4 != lastButton4) lastDebounce4 = millis();
if ((millis() - lastDebounce4) > debounceDelay) {
  static bool buttonState4 = HIGH;
  if (reading4 != buttonState4) {
    buttonState4 = reading4;
    if (buttonState4 == LOW) {
      // сброс всех таймеров и лент
      running1 = false;
      running2 = false;
      counter1 = counterDefault;
      counter2 = counterDefault;
      strip1.clear();
      strip1.show();
      strip2.clear();
      strip2.show();
      drawNumberSingle(counter1, strip1, normalColor);
      drawNumberSingle(counter2, strip2, normalColor);
    }
  }
}
lastButton4 = reading4;

}
