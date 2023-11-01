#include <Arduino.h>
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
uint16_t hue_step = 10;
uint32_t rainbow_delay = 5;

void rainbow() {
    uint32_t color = Adafruit_NeoPixel::ColorHSV(hue, UINT8_MAX, UINT8_MAX);
    for (int i = 0; i < leds.numPixels(); ++i) {
        leds.setPixelColor(i, color);
    }
    leds.show();
    delay(rainbow_delay);
    hue = (hue + hue_step) % UINT16_MAX;
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
String constructHtml() {
    auto file = SPIFFS.open("/index.html");
    String html("");
    while (file.available()) {
        html += char(file.read());
    }
    file.close();
    auto offStatus = "off";
    auto rainbowStatus = "off";
    auto singleStatus = "off";
    switch (effect) {
        case Off:
            offStatus = "on";
            break;
        case Rainbow:
            rainbowStatus = "on";
            break;
        case SingleColor:
            singleStatus = "on";
            break;
    }
    html.replace("{{rainbow-class}}", rainbowStatus);
    html.replace("{{white-class}}", singleStatus);
    html.replace("{{off-class}}", offStatus);
    return html;
}

void serveSite(AsyncWebServerRequest *request) {
    request->send(200, "text/html", constructHtml());
}

void handleRainbow(AsyncWebServerRequest *request) {
    effect = Rainbow;
    request->redirect("/");
}

void handleWhite(AsyncWebServerRequest *request) {
    effect = SingleColor;
    color = Adafruit_NeoPixel::Color(255, 255, 255);
    request->redirect("/");
}

void handleOff(AsyncWebServerRequest *request) {
    effect = Off;
    request->redirect("/");
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

    server.on("/", HTTP_GET, serveSite);
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/style.css", "text/css");
    });
    server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });
    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/favicon.ico", "image/x-icon");
    });
    server.on("/off", HTTP_GET, handleOff);
    server.on("/rainbow", HTTP_GET, handleRainbow);
    server.on("/white", HTTP_GET, handleWhite);
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