#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include "credentials.h"

#define PIN_LEDS 16
#define NUM_PIXELS 60

Adafruit_NeoPixel leds(NUM_PIXELS, PIN_LEDS, NEO_GRB + NEO_KHZ800);
bool current_state = false;
AsyncWebServer server(80);

// region Effects
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
uint32_t color;

void single_color() {
    for (int i = 0; i < leds.numPixels(); ++i) {
        leds.setPixelColor(i, color);
    }
    leds.show();
}
// endregion
// endregion

// region HTML
String constructHtml(bool ledState) {
    String ptr = "<!DOCTYPE html> <html>\n";
    ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    ptr += "<title>LED Control</title>\n";
    ptr += R"(<link rel="stylesheet" type="text/css" href="/style.css">)";
    ptr += "</head>\n";
    ptr += "<body>\n";
    ptr += "<h1>ESP32 Web Server</h1>\n";
    if (ledState) {
        ptr += "<p>LED: ON</p><a class=\"button button-off\" href=\"/off\">OFF</a>\n";
    } else {
        ptr += "<p>LED: OFF</p><a class=\"button button-on\" href=\"/on\">ON</a>\n";
    }
    if (leds.numPixels() == 0) {
        ptr += "<p> NO PIXELS </p>";
    }

    ptr += "</body>\n";
    ptr += "</html>\n";

    return ptr;
}

void serveSite(AsyncWebServerRequest *request) {
    request->send(200, "text/html", constructHtml(current_state));
}

void handleOn(AsyncWebServerRequest *request) {
    current_state = true;
    serveSite(request);
}

void handleOff(AsyncWebServerRequest *request) {
    current_state = false;
    serveSite(request);
}
// endregion

// region Setup and Loop
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

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
    server.on("/on", HTTP_GET, handleOn);
    server.on("/off", HTTP_GET, handleOff);
    server.begin();
}

#pragma clang diagnostic pop

void loop() {
    if (current_state) {
        rainbow();
    } else {
        leds.clear();
        leds.show();
    }
}
// endregion