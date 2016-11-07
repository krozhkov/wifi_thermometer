#include <Arduino.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "win.h"

// For the Adafruit shield, these are the default.
#define TFT_DC D1
#define TFT_CS D2

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

unsigned long testFillScreen() {
  unsigned long start = micros();
  tft.fillScreen(ILI9341_BLACK);
  yield();
  tft.fillScreen(ILI9341_RED);
  yield();
  tft.fillScreen(ILI9341_GREEN);
  yield();
  tft.fillScreen(ILI9341_BLUE);
  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();
  return micros() - start;
}

void drawColorBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h) {
  int16_t i, j;
  uint16_t word;

  for(j = 0; j < h; j++) {
    for(i = 0; i < w; i++) {
      word = pgm_read_word(bitmap + ((j * w + i) << 1));
      tft.drawPixel(x + i, y + (h - j), (word >> 8) | (word << 8));
    }
    yield();
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("ILI9341 Test!");

  tft.begin();

  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX);

  Serial.println(F("Benchmark                Time (microseconds)"));

  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  drawColorBitmap(0, 0, win_bits, win_width, win_height);

  //yield();
  //tft.drawPixel(0, 0, ILI9341_RED);
  //tft.drawPixel(200, 100, ILI9341_RED);
  //tft.drawPixel(319, 239, ILI9341_GREEN);
}


void loop(void) {
  /*for(uint8_t rotation=0; rotation<4; rotation++) {
    tft.setRotation(rotation);
    testText();
    delay(1000);
  }*/
}
