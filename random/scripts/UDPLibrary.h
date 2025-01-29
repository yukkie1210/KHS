#pragma once
#include <WiFi.h>
#include <WiFiUdp.h>

class UDPLib {
  private:
    WiFiUDP wifiUdp;
    String host;
    int send_port;
    int receive_port;
  public:
    void begin(String ip, int send_p, int rec_p);
    void send(String);
    String read();
};