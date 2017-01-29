#include <Arduino.h>
#include "LedControl.h"

LedControl lc = LedControl(D3, D1, D2, 2);

unsigned long delaytime = 1000;

static const unsigned char minus[] = {B00000000, B00000000, B00000000, B00000000, B00000111, B00000000, B00000000, B00000000};
static const unsigned char digits[10][8] = {
  {B00000000, B00001110, B00010001, B00010011, B00010101, B00011001, B00010001, B00001110},
  {B00000000, B00000100, B00001100, B00010100, B00000100, B00000100, B00000100, B00011111},
  {B00000000, B00001110, B00010001, B00010001, B00000110, B00001000, B00010000, B00011111},
  {B00000000, B00001110, B00010001, B00000001, B00000110, B00000001, B00010001, B00001110},
  {B00000000, B00010001, B00010001, B00010001, B00001111, B00000001, B00000001, B00000001},
  {B00000000, B00011111, B00010000, B00010000, B00011110, B00000001, B00000001, B00011110},
  {B00000000, B00001110, B00010001, B00010000, B00011110, B00010001, B00010001, B00001110},
  {B00000000, B00011111, B00010001, B00000001, B00000010, B00000010, B00000100, B00000100},
  {B00000000, B00001110, B00010001, B00010001, B00001110, B00010001, B00010001, B00001110},
  {B00000000, B00001110, B00010001, B00010001, B00001111, B00000001, B00010001, B00001110}
};

unsigned short buffer[8];

void clearBuffer() {
  for (int i = 0; i < 8; i++) {
    buffer[i] = 0;
  }
}

void shiftBuffer(int offset) {
  for (int i = 0; i < 8; i++) {
    buffer[i] <<= offset;
  }
}

void flushBuffer() {
  for (int i = 0; i < 8; i++) {
    unsigned char right = buffer[i];
    unsigned char left = buffer[i] >> 8;
    lc.setRow(0, i, right);
    lc.setRow(1, i, left);
  }
}

void dumpBuffer() {
  for (int i = 0; i < 8; i++) {
    Serial.println(String(buffer[i], BIN));
  }
}

void writeDigit(int num) {
  shiftBuffer(6);
  for (int i = 0; i < 8; i++) {
    buffer[i] = buffer[i] | digits[num][i];
  }
}

void writeMinus() {
  shiftBuffer(4);
  for (int i = 0; i < 8; i++) {
    buffer[i] = buffer[i] | minus[i];
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println();
  for (int i = 0; i < lc.getDeviceCount(); i++) {
    lc.shutdown(i, false);
    lc.setIntensity(i, 2);
    lc.clearDisplay(i);
  }
  clearBuffer();
  writeMinus();
  writeDigit(2);
  writeDigit(5);
  shiftBuffer(1);
  flushBuffer();
}

void loop() {
}
