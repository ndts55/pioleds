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
uint32_t color = Adafruit_NeoPixel::Color(255, 255, 255);

void singleColor() {
    for (int i = 0; i < leds.numPixels(); ++i) {
        leds.setPixelColor(i, color);
    }
    leds.show();
}
// endregion
// endregion

// region HTML
String indexHtml("");

void serveSite(AsyncWebServerRequest *request) {
    request->send(200, "text/html", indexHtml);
}

void handleGetRainbow(AsyncWebServerRequest *request) {
    // TODO return current rainbow speed
}

void handlePostRainbow(AsyncWebServerRequest *request) {
    // TODO parse json in request for rainbow speed information
    effect = Rainbow;
    request->redirect("/");
}

void handleOff(AsyncWebServerRequest *request) {
    effect = Off;
    request->redirect("/");
}

void handleGetColor(AsyncWebServerRequest *request) {
    // TODO return current color
}

void handlePostColor(AsyncWebServerRequest *request) {
    // TODO parse json in request for color information
    effect = SingleColor;
    color = Adafruit_NeoPixel::Color(255, 255, 255);
    request->redirect("/");
}

void handleGetState(AsyncWebServerRequest *request) {
    DynamicJsonDocument json(1024);
    json["effect"] = effect;
    // TODO parse color to hex
    json["color"] = color;
    json["speed"] = speed;

    String jsonString = "";
    auto bytesWritten = serializeJson(json, jsonString);
    if (bytesWritten > 0) {
        request->send(200, "text/json", jsonString);
    } else {
        request->send(500, "text/plain", "Error creating internal state");
    }
}
// endregion

// region Setup and Loop
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

AsyncWebServer server(80);

void setup() {
    leds.begin();

    SPIFFS.begin(true);
    auto file = SPIFFS.open("/index.html");
    while (file.available()) {
        indexHtml += char(file.read());
    }
    file.close();

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

    server.on("/", HTTP_GET, serveSite);
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
    server.on("/off", HTTP_POST, handleOff);
    server.on("/rainbow", HTTP_GET, handleGetRainbow);
    server.on("/rainbow", HTTP_POST, handlePostRainbow);
    server.on("/color", HTTP_GET, handleGetColor);
    server.on("/color", HTTP_POST, handlePostColor);
    server.on("/state", HTTP_GET, handleGetState);
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