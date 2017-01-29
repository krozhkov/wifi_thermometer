#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "LedControl.h"

LedControl lc = LedControl(D3, D1, D2, 2);

const char* ssid     = "arbuz";
const char* password = "arbuz321";
const char* tempHost = "weather.nsu.ru";
const int httpPort = 80;
const char* tempPath = "/weather_brief.xml";

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
  if (num < 0 || num > 9) {
    return;
  }

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

String parseTemp(String xml) {
  int start = xml.indexOf("<current>");
  int end = xml.indexOf("</current>");
  String temp = xml.substring(start + 9, end); // 9 - size of <current> string
  int dotPos = temp.indexOf(".");
  return temp.substring(0, dotPos);
}

void drawTemp(String temp) {
  clearBuffer();
  int length = temp.length();
  for (int i = 0; i < length; i++) {
    char chr = temp.charAt(i);
    switch (chr) {
      case '-':
        writeMinus();
        break;
      case '+':
        break;
      default:
        int num = chr - '0';
        writeDigit(num);
        break;
    }
  }
  shiftBuffer(1);
  flushBuffer();
}

void setup() {
  Serial.begin(9600);
  Serial.println();

  for (int i = 0; i < lc.getDeviceCount(); i++) {
    lc.shutdown(i, false);
    lc.setIntensity(i, 2);
    lc.clearDisplay(i);
  }

  WiFi.mode(WIFI_STA);
  while (true) {
    Serial.println("Attempting to connect to wifi.\n");
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
      break;
    }
    delay(10000);
  }
}

void loop() {
  HTTPClient http;
  http.begin(tempHost, httpPort, tempPath);

  int httpCode = http.GET();
  if(httpCode) {
      if(httpCode == 200) {
          String payload = http.getString();
          String temp = parseTemp(payload);
          Serial.println(temp);
          drawTemp(temp);
      }
  } else {
      Serial.print("[HTTP] GET... failed, no connection or no HTTP server\n");
  }

  delay(60000);
}
