#include <Arduino.h>
#include "LedControl.h"

LedControl lc = LedControl(D3,D1,D2,2);

unsigned long delaytime=1000;

static const byte minus[] PROGMEM = {B00000000, B00000000, B00000000, B00000000, B00000111, B00000000, B00000000, B00000000};
static const byte digits[10][8] = {
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

void setup() {
  for (int i = 0; i < lc.getDeviceCount(); i++) {
    lc.shutdown(i, false);
    lc.setIntensity(i, 4);
    lc.clearDisplay(i);
  }
}

void writeArduinoOnMatrix() {
  for (int num = 0; num < 10; num++) {
    lc.setRow(0,0,digits[num][0]);
    lc.setRow(0,1,digits[num][1]);
    lc.setRow(0,2,digits[num][2]);
    lc.setRow(0,3,digits[num][3]);
    lc.setRow(0,4,digits[num][4]);
    lc.setRow(0,5,digits[num][5]);
    lc.setRow(0,6,digits[num][6]);
    lc.setRow(0,7,digits[num][7]);
    delay(delaytime);
  }
}

void loop() {
  writeArduinoOnMatrix();
}
