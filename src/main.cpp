#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include "credentials.h"

#define PIN_LEDS 13
#define NUM_PIXELS 60

// region State
const uint8_t BRIGHTNESS_MIN = 60;
uint8_t brightness = 200;
enum Effect {
    Off, SingleColor, Rainbow, Christmas
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
        case Christmas:
            return "christmas";
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
uint16_t rainbowHue = 0;
uint16_t rainbowSpeed = 10;
const uint32_t rainbowDelay = 5;

void rainbow() {
    uint32_t color = Adafruit_NeoPixel::ColorHSV(rainbowHue, UINT8_MAX, UINT8_MAX);
    for (int i = 0; i < leds.numPixels(); ++i) {
        leds.setPixelColor(i, color);
    }
    leds.show();
    delay(rainbowDelay);
    rainbowHue = (rainbowHue + rainbowSpeed) % UINT16_MAX;
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

// region Christmas
const uint16_t christmasDelay = 500;
bool christmasSwitch = false;

void christmas() {
    for (int i = 0; i < leds.numPixels(); ++i) {
        if (i % 2 == christmasSwitch) {
            leds.setPixelColor(i, 255, 0, 0);
        } else {
            leds.setPixelColor(i, 0, 255, 0);
        }
    }
    leds.show();
    delay(christmasDelay);
    christmasSwitch = !christmasSwitch;
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

void onGetState(AsyncWebServerRequest *request) {
    DynamicJsonDocument json(1024);
    json["effect"] = effectToString(effect);
    json["color"]["red"] = red;
    json["color"]["green"] = green;
    json["color"]["blue"] = blue;
    json["rainbowSpeed"] = rainbowSpeed;
    json["brightness"] = brightness;

    sendJson(request, json, "Error creating internal state");
}

void onPostRainbow(AsyncWebServerRequest *request) {
    effect = Rainbow;
    if (request->_tempObject == nullptr) {
        onGetState(request);
        return;
    }
    DynamicJsonDocument json(64);
    auto error = deserializeJson(json, (uint8_t *) (request->_tempObject));
    if (error) {
        request->send(500, "text/plain", error.c_str());
        return;
    }
    rainbowSpeed = json["rainbowSpeed"];
    onGetState(request);
}

void onPostOff(AsyncWebServerRequest *request) {
    effect = Off;
    onGetState(request);
}

void onPostColor(AsyncWebServerRequest *request) {
    effect = SingleColor;
    if (request->_tempObject == nullptr) {
        onGetState(request);
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
    onGetState(request);
}

void onPostBrightness(AsyncWebServerRequest *request) {
    if (request->_tempObject == nullptr) {
        onGetState(request);
        return;
    }
    DynamicJsonDocument json(1024);
    auto error = deserializeJson(json, (uint8_t *) (request->_tempObject));
    if (error) {
        request->send(500, "text/plain", error.c_str());
        return;
    }
    brightness = json["brightness"];
    if (brightness < BRIGHTNESS_MIN) { brightness = BRIGHTNESS_MIN; }
    onGetState(request);
}

void onPostChristmas(AsyncWebServerRequest *request) {
    effect = Christmas;
    onGetState(request);
}
// endregion

// region Setup and Loop
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

const IPAddress localIp(192, 168, 0, 91);
const IPAddress gateway(192, 168, 0, 1);
const IPAddress subnet(255, 255, 255, 0);
AsyncWebServer server(80);

void setup() {
    leds.begin();

    SPIFFS.begin(true);

    WiFi.config(localIp, gateway, subnet);
    WiFi.begin(ssid, password);
    while (WiFiClass::status() != WL_CONNECTED) { delay(500); }

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
    server.on("/rainbow", HTTP_POST, onPostRainbow, nullptr, onJsonBody);
    server.on("/color", HTTP_POST, onPostColor, nullptr, onJsonBody);
    server.on("/state", HTTP_GET, onGetState);
    server.on("/brightness", HTTP_POST, onPostBrightness, nullptr, onJsonBody);
    server.on("/christmas", HTTP_POST, onPostChristmas);
    server.begin();
}

#pragma clang diagnostic pop

void loop() {
    if (leds.getBrightness() != brightness) { leds.setBrightness(brightness); }
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
        case Christmas:
            christmas();
            break;
        default:
            break;
    }
}
// endregion