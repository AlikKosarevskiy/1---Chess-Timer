#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(
  8, 8, PIN,
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB + NEO_KHZ800
);

void setup() {
  matrix.begin();
  matrix.setBrightness(40);
}

void loop() {
  matrix.fillScreen(matrix.Color(255, 0, 0)); // красный
  matrix.show();
  delay(500);
  matrix.fillScreen(matrix.Color(0, 255, 0)); // зелёный
  matrix.show();
  delay(500);
  matrix.fillScreen(matrix.Color(0, 0, 255)); // синий
  matrix.show();
  delay(500);
}
