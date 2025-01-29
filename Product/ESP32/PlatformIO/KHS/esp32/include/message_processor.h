#pragma once
#include <Arduino.h>
#include <WiFi.h>

class MessageProcessor {
public:
    MessageProcessor();
    bool processMessage(WiFiClient& client);
    void statusResponse(WiFiClient& client);
    void sendResponse(WiFiClient& client, uint8_t response);

private:
    float bytesToFloat(const uint8_t* bytes);
    int16_t bytesToInt16(const uint8_t* bytes);
};