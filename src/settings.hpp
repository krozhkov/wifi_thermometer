#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <Arduino.h>
#include <vector>

namespace settings {
  extern String ssid;
  extern String password;
  extern std::vector<String> optionNames;
  boolean isSettingsValid();
  void readConfig();
  void writeConfig();
  void applySetting(String settingName, String settingValue);
  String getQueryString();
}

#endif /* _SETTINGS_H_ */
