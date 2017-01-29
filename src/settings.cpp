
#include <Arduino.h>
#include <FS.h>
#include <vector>
#include <algorithm>
#include "urlencode.hpp"
#include "settings.hpp"
#include "debug.h"
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

#include <osapi.h>
#include <os_type.h>
#include <c_types.h>
#include <user_interface.h>

#ifdef __cplusplus
}
#endif

namespace settings {
  String ssid;
  String password;

  const char *configFileName PROGMEM = "/config.txt";

  vector<String> optionNames = { "ssid", "password" };

  #define SSID 0
  #define PASSWORD 1

  void applySetting(String settingName, String settingValue) {
    if (settingName == optionNames[SSID]) {
      ssid = settingValue;
    } else if (settingName == optionNames[PASSWORD]) {
      password = settingValue;
    }
  }

  void readConfig() {
    auto f = SPIFFS.open(configFileName, "r");
    if (!f) {
      debug("%s not found.\n", configFileName);
      return;
    }

    while (f.available()) {
      auto str = f.readStringUntil('\n');
      str.trim();
      debug("[readConfig] line is '%s'\n", str.c_str());
      if (str.length() > 0) {
        auto splitPos = str.indexOf('=');
        auto settingName = str.substring(0, splitPos);
        auto settingValue = str.substring(splitPos + 1);
        debug("[readConfig] name is '%s', value is '%s'\n", settingName.c_str(), settingValue.c_str());
        applySetting(settingName, settingValue);
      }
    }
    f.close();
    // write sdk config
    struct station_config conf;
    wifi_station_get_config(&conf);
    conf.bssid_set = 0;
    strcpy(reinterpret_cast<char*>(conf.ssid), ssid.c_str());
    debug("[writeConfig] ssid writen '%s'\n", reinterpret_cast<char*>(conf.ssid));
    strcpy(reinterpret_cast<char*>(conf.password), password.c_str());
    debug("[writeConfig] password writen '%s'\n", reinterpret_cast<char*>(conf.password));
    ETS_UART_INTR_DISABLE();
    if (!wifi_station_set_config(&conf)) {
      debug("[writeConfig] wifi configuration not saved.\n");
    }
    ETS_UART_INTR_ENABLE();
  }

  void writeConfig() {
    // write config file
    auto f = SPIFFS.open(configFileName, "w+");
    f.println(optionNames[SSID] + "=" + ssid);
    f.println(optionNames[PASSWORD] + "=" + password);
    f.close();
  }

  boolean isSettingsValid() {
    return ssid && ssid.length() > 0 && password && password.length() > 0;
  }

  String getQueryString() {
    return "?" + optionNames[SSID] + "=" + urlencode(ssid) +
           "&" + optionNames[PASSWORD] + "=" + urlencode(password);
  }
}
