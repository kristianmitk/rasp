#ifndef rasp_wifi_h
#define rasp_wifi_h

#include <Arduino.h>
#include <ESP8266WiFi.h>

/**
 * TODO: DOCS
 * [connectToWiFi description]
 */
void connectToWiFi() {
    const char ssid[] = "PiFun1337";
    const char pw[]   = "RaSpFun1337!!";

    WiFi.begin(ssid, pw);

    Serial.println("Mac-Address: " + WiFi.macAddress());
    Serial.print("Waiting for WiFi connection");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to: " + String(ssid));
    Serial.println("Static IP: " + WiFi.localIP().toString());
}

#endif // ifndef rasp_wifi_h
