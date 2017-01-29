#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include "LedControl.h"
#include "dhdebug.h"

LedControl lc = LedControl(D3, D1, D2, 2);

const char* ssid     = "arbuz";
const char* password = "arbuz321";
const char* tempHost = "weather.nsu.ru";
const int httpPort = 80;
const char* tempPath = "/weather_brief.xml";

IPAddress apIP(192, 168, 2, 1);
IPAddress netMask(255, 255, 255, 0);
ESP8266WebServer server;
const char *apName = "termometr-device";

#ifdef __cplusplus
extern "C" {
#endif

#include <strings.h>
#include <osapi.h>
#include <os_type.h>
#include <c_types.h>
#include <user_interface.h>

extern int rtc_mem_check(int f);

#ifdef __cplusplus
}
#endif

// this code part of https://github.com/devicehive/esp8266-firmware/blob/master/firmware-src/sources/main.c

typedef struct {
  unsigned int magic;
  unsigned int resetCounter;
} RESET_COUNTER;
#define RESET_COUNTER_MAGIC 0x12345678
#define RESET_COUNTER_RTC_ADDRESS 128
#define RESET_NUM 3

LOCAL os_timer_t mResetTimer;
LOCAL unsigned int mSpecialMode = 0;

LOCAL void ICACHE_FLASH_ATTR reset_counter(void *arg) {
  dhdebug("[reset_counter]\n");
  RESET_COUNTER counter;
  counter.magic = RESET_COUNTER_MAGIC;
  counter.resetCounter = 0;
  system_rtc_mem_write(RESET_COUNTER_RTC_ADDRESS, &counter, sizeof(counter));
}

void run_user_rf_pre_init(void) {
  RESET_COUNTER counter;
  system_rtc_mem_read(RESET_COUNTER_RTC_ADDRESS, &counter, sizeof(counter));
  if(counter.magic == RESET_COUNTER_MAGIC && counter.resetCounter <= RESET_NUM) {
    counter.resetCounter++;
    dhdebug("[run_user_rf_pre_init] resetCounter == %d.\n.", counter.resetCounter);
    if(counter.resetCounter == RESET_NUM) {
      dhdebug("[run_user_rf_pre_init] set special mode.\n");
      reset_counter(0);
      mSpecialMode = 1;
    } else {
      dhdebug("[run_user_rf_pre_init] setup timer.\n");
      system_rtc_mem_write(RESET_COUNTER_RTC_ADDRESS, &counter, sizeof(counter));
      os_timer_disarm(&mResetTimer);
      os_timer_setfn(&mResetTimer, (os_timer_func_t *)reset_counter, NULL);
      os_timer_arm(&mResetTimer, 3000, 0);
    }
  } else {
    dhdebug("[run_user_rf_pre_init] reset counter.\n");
    reset_counter(0);
  }

  system_restore();
  rtc_mem_check(0);
}

// end of https://github.com/devicehive/esp8266-firmware/blob/master/firmware-src/sources/main.c

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

void drawSpecialMode() {
  clearBuffer();
  writeMinus();
  shiftBuffer(1);
  writeMinus();
  shiftBuffer(4);
  flushBuffer();
}

void setup() {
  Serial.begin(9600);
  Serial.println();
  run_user_rf_pre_init();

  for (int i = 0; i < lc.getDeviceCount(); i++) {
    lc.shutdown(i, false);
    lc.setIntensity(i, 2);
    lc.clearDisplay(i);
  }

  WiFi.mode(WIFI_STA);

  if(mSpecialMode) {
    dhdebug("special mode.\n");
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, netMask);
    WiFi.softAP(apName);
    //initWebServer();
    drawSpecialMode();
  } else {
    dhdebug("normal mode.\n");
    while (true) {
      Serial.println("Attempting to connect to wifi.\n");
      WiFi.begin(ssid, password);
      if (WiFi.waitForConnectResult() == WL_CONNECTED) {
        break;
      }
      delay(10000);
    }
    dhdebug("Connected.\n");
  }
}

void loop() {
  if (mSpecialMode) {
    //server.handleClient();
  } else {
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
}
