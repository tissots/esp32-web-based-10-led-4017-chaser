#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "patterns.h"

extern AsyncWebServer server;

inline void initWebServer() {
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Route for root web page - served directly via SPIFFS flash structure
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/pattern", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("p")) {
      pattern = request->getParam("p")->value().toInt();
      countdownActive = false;
      resetCounter();
    }
    request->send(200);
  });

  server.on("/speed", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("s")) {
      speed = request->getParam("s")->value().toInt();
    }
    request->send(200);
  });

  server.on("/brightness", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("b")) {
      brightness = request->getParam("b")->value().toInt();
      ledcWrite(pwmChannel, brightness);
    }
    request->send(200);
  });

  server.on("/countdown", HTTP_GET, [](AsyncWebServerRequest* request) {
    countdownActive = true;
    countdownValue = 10;
    countdownTimer = millis();
    resetCounter();
    request->send(200);
  });

  server.begin();
}

#endif