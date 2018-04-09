#ifndef rasp_wifi_h
#define rasp_wifi_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "config.h"

/**
 * Establishes a wifi connection to the in the function specified SSID.
 * This function is meant to be called once in arduino's setup() function.
 */
void connectToWiFi() {
    WiFi.begin(SSID, SSID_PW);

    Serial.println("Mac-Address: " + WiFi.macAddress());
    Serial.print("Waiting for WiFi connection");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    RASPDBG("\nConnected to: %s\n", SSID)
    Serial.println("Static IP: " + WiFi.localIP().toString());
}

#endif // ifndef rasp_wifi_h
