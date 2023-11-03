#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include "credentials.h"

#define PIN_LEDS 16
#define NUM_PIXELS 60

// region State
enum Effect {
    Off, SingleColor, Rainbow
};
Effect effect = Off;

String effectToString(Effect e) {
    switch (e) {
        case Off:
            return "off";
        case SingleColor:
            return "single-color";
        case Rainbow:
            return "rainbow";
        default:
            return "";
    }
}
// endregion

// region Effects
Adafruit_NeoPixel leds(NUM_PIXELS, PIN_LEDS, NEO_GRB + NEO_KHZ800);

// region Off
void off() {
    leds.clear();
    leds.show();
}

// endregion
// region Rainbow
uint16_t hue = 0;
uint16_t speed = 10;
uint32_t rainbow_delay = 5;

void rainbow() {
    uint32_t color = Adafruit_NeoPixel::ColorHSV(hue, UINT8_MAX, UINT8_MAX);
    for (int i = 0; i < leds.numPixels(); ++i) {
        leds.setPixelColor(i, color);
    }
    leds.show();
    delay(rainbow_delay);
    // TODO properly incorporate speed variable in next hue calculation
    hue = (hue + speed) % UINT16_MAX;
}
// endregion

// region Single Color
uint8_t red = 255;
uint8_t green = 255;
uint8_t blue = 255;

void singleColor() {
    for (int i = 0; i < leds.numPixels(); ++i) {
        leds.setPixelColor(i, red, green, blue);
    }
    leds.show();
}
// endregion
// endregion

// region Backend
void onJsonBody(
        AsyncWebServerRequest *request,
        uint8_t *bodyData,
        size_t bodyLen,
        size_t index,
        size_t total) {
    if (total > 0 && request->_tempObject == nullptr) {
        request->_tempObject = malloc(total);
    }
    if (request->_tempObject != nullptr) {
        memcpy((uint8_t *) (request->_tempObject) + index, bodyData, bodyLen);
    }
}

void sendJson(AsyncWebServerRequest *request, const DynamicJsonDocument &json, const String &
errorMsg) {
    String jsonString;
    auto bytesWritten = serializeJson(json, jsonString);
    if (bytesWritten > 0) {
        request->send(200, "application/json", jsonString);
    } else {
        request->send(500, "text/plain", errorMsg);
    }
}

void onGetRainbow(AsyncWebServerRequest *request) {
    DynamicJsonDocument json(64);
    json["speed"] = speed;
    sendJson(request, json, "Error creating get rainbow response");
}

void onPostRainbow(AsyncWebServerRequest *request) {
    effect = Rainbow;
    if (request->_tempObject == nullptr) {
        onGetRainbow(request);
        return;
    }
    DynamicJsonDocument json(64);
    auto error = deserializeJson(json, (uint8_t *) (request->_tempObject));
    if (error) {
        request->send(500, "text/plain", error.c_str());
        return;
    }
    speed = json["speed"];
    onGetRainbow(request);
}

void onPostOff(AsyncWebServerRequest *request) {
    effect = Off;
    request->send(200, "text/plain", "");
}

void onGetColor(AsyncWebServerRequest *request) {
    DynamicJsonDocument json(1024);
    json["color"]["red"] = red;
    json["color"]["green"] = green;
    json["color"]["blue"] = blue;
    sendJson(request, json, "Error creating get color response");
}


void onPostColor(AsyncWebServerRequest *request) {
    effect = SingleColor;
    if (request->_tempObject == nullptr) {
        onGetColor(request);
        return;
    }
    DynamicJsonDocument json(1024);
    auto error = deserializeJson(json, (uint8_t *) (request->_tempObject));
    if (error) {
        request->send(500, "text/plain", error.c_str());
        return;
    }
    red = json["color"]["red"];
    green = json["color"]["green"];
    blue = json["color"]["blue"];
    onGetColor(request);
}

void onGetState(AsyncWebServerRequest *request) {
    DynamicJsonDocument json(1024);
    json["effect"] = effectToString(effect);
    json["color"]["red"] = red;
    json["color"]["green"] = green;
    json["color"]["blue"] = blue;
    json["speed"] = speed;

    sendJson(request, json, "Error creating internal state");
}
// endregion

// region Setup and Loop
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

AsyncWebServer server(80);

void setup() {
    leds.begin();

    SPIFFS.begin(true);

    Serial.begin(115200);
    Serial.print("Connecting to...");
    Serial.print(ssid);
    WiFi.begin(ssid, password);
    while (WiFiClass::status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.print("");
    Serial.print("Connected!");
    Serial.print("Got IP: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });
    server.on("/normalize.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/normalize.css", "text/css");
    });
    server.on("/skeleton.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/skeleton.css", "text/css");
    });
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/style.css", "text/css");
    });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/script.js", "application/javascript");
    });
    server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });
    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/favicon.ico", "image/x-icon");
    });
    server.on("/off", HTTP_POST, onPostOff);
    server.on("/rainbow", HTTP_GET, onGetRainbow);
    server.on("/rainbow", HTTP_POST, onPostRainbow, nullptr, onJsonBody);
    server.on("/color", HTTP_GET, onGetColor);
    server.on("/color", HTTP_POST, onPostColor, nullptr, onJsonBody);
    server.on("/state", HTTP_GET, onGetState);
    server.begin();
}

#pragma clang diagnostic pop

void loop() {
    switch (effect) {
        case Off:
            off();
            break;
        case Rainbow:
            rainbow();
            break;
        case SingleColor:
            singleColor();
            break;
        default:
            break;
    }
}
// endregion