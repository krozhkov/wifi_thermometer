#include <Arduino.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "webServerHandlers.hpp"
#include "settings.hpp"
#include "debug.h"
using namespace std;
using namespace settings;

namespace web {
  ESP8266WebServer server;
  String serverName = "192.168.2.1";

  /** Is this an IP? */
  boolean isIp(String str) {
    for (int i = 0; i < str.length(); i++) {
      int c = str.charAt(i);
      if (c != '.' && (c < '0' || c > '9')) {
        return false;
      }
    }
    return true;
  }

  void handleSave() {
    debug("[handleSave] enter.\n");

    for (vector<String>::size_type i = 0; i != optionNames.size(); i++) {
      if (server.hasArg(optionNames[i].c_str())) {
        debug("[handleSave] %s is %s\n", optionNames[i].c_str(), server.arg(optionNames[i].c_str()).c_str());
        applySetting(optionNames[i], server.arg(optionNames[i].c_str()));
      }
    }

    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");

    debug("[handleSave] open file.\n");
    File f = SPIFFS.open("/success.htm", "r");
    if (!f) {
      debug("success.htm not found.\n");
      return;
    }

    debug("[handleSave] stream file.\n");
    if (server.streamFile(f, "text/html") != f.size()) {
      debug("Sent less data than expected!\n");
    }

    debug("[handleSave] close file.\n");
    f.close();

    writeConfig();
  }

  void redirectToIndex(){
    server.sendHeader("Location", String("http://") + serverName + String("/index.htm") + getQueryString(), true);
    server.send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
  }

  boolean captivePortal() {
    debug("%s requested.\n", server.hostHeader().c_str());
    serverName.toLowerCase();
    server.hostHeader().toLowerCase();
    if (!isIp(server.hostHeader()) && !server.hostHeader().startsWith(serverName)) {
      debug("Request redirected from [%s%s] to captive portal\n", server.hostHeader().c_str(), server.uri().c_str());
      redirectToIndex();
      return true;
    }
    return false;
  }

  void handleNotFound() {
    if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
      return;
    }
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";

    for ( uint8_t i = 0; i < server.args(); i++ ) {
      message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
    }
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.send ( 404, "text/plain", message );
  }

  void handleRoot() {
    if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
      return;
    }
    redirectToIndex();
  }

  void initWebServer() {
    server = ESP8266WebServer(80);
    server.on("/", handleRoot);
    server.on("/ncsi.txt", handleRoot);
    server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
    server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
    server.on("/save", HTTP_POST, handleSave);
    server.serveStatic("/index.htm", SPIFFS, "/index.htm");
    server.serveStatic("/bootstrap.min.css", SPIFFS, "/bootstrap.min.css");
    server.serveStatic("/bootstrap-theme.min.css", SPIFFS, "/bootstrap-theme.min.css");
    server.onNotFound(handleNotFound);
    server.begin();
  }
}
