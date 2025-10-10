#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
//#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/TomThumb.h>
#include <Fonts/FreeMono9pt7b.h>

#define PIN 6

// Матрица 8x16 (8 ширина, 16 высота)
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(
  8, 16, PIN,
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB + NEO_KHZ800);

int currentDigit = 0;
unsigned long lastChange = 0;
const unsigned long interval = 1000; // интервал между цифрами (мс)

void setup() {
  matrix.begin();
  matrix.setBrightness(40);
  matrix.setFont(&FreeMono9pt7b);
  matrix.setTextColor(matrix.Color(0, 0, 255));
  matrix.setTextSize(1); // уменьшение масштаба
}

void loop() {
  unsigned long now = millis();

  if (now - lastChange >= interval) {
    lastChange = now;

    // Очистка экрана
    matrix.fillScreen(0);

    // Установка позиции для центрирования цифры
    matrix.setCursor(-2, 14); // подгон под высоту матрицы 16 пикселей

    // Вывод цифры
    matrix.print(currentDigit);
    matrix.show();

    // Следующая цифра
    currentDigit++;
    if (currentDigit > 9) currentDigit = 0;
  }
}
