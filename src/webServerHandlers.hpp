#ifndef _WEB_SERVER_HANDLERS_H_
#define _WEB_SERVER_HANDLERS_H_

#include <Arduino.h>
#include <ESP8266WebServer.h>

namespace web {
  extern ESP8266WebServer server;
  void initWebServer();
}

#endif /* _WEB_SERVER_HANDLERS_H_ */
