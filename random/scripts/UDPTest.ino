#include"UDPLibrary.h"

UDPLib _udp;

char* ssid="ssid";
char* password="pass";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("connected "+String(ssid));
  Serial.println(WiFi.localIP());
  _udp.begin("192.168.0.32",9000,9001);
}

void loop() {
  if(Serial.available()>0){
    String text=Serial.readStringUntil('\n');
    _udp.send(text);
  }
  String r=_udp.read();
  if(r!=""){
    Serial.println(r);
  }
}