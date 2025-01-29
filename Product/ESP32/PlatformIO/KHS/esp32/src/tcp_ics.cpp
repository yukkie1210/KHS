// test_tcp_ics.cpp
#include <Arduino.h>
#include "wifi_connection.h"
#include <IcsHardSerialClass.h>

// Wi-Fi接続
WiFiConnection wifiConnection;

// サーボ設定
const byte EN_PIN = 3; // EN
const byte RX_PIN = 7; // 35
const byte TX_PIN = 6; // 34
const long BAUDRATE = 1250000;
const int TIMEOUT = 20;

// ICSサーボ制御
IcsHardSerialClass krs(&Serial2, EN_PIN, BAUDRATE, TIMEOUT);

void setup() {
    // シリアルモニタ用
    Serial.begin(115200);

    // サーボ用シリアルの初期化
    Serial2.begin(BAUDRATE, SERIAL_8E1, RX_PIN, TX_PIN, false, TIMEOUT);
    krs.begin();

    // Wi-Fiの初期化
    if (wifiConnection.begin()) {
        Serial.println("Connected to WiFi");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("Failed to connect to WiFi");
    }
}

void loop() {
    wifiConnection.handleConnection();

    if (wifiConnection.isConnected()) {
        WiFiClient client = wifiConnection.getServer()->available();
        if (client) {
            Serial.println("Client connected");
            wifiConnection.setClientConnected(true);

            while (client.connected()) {
                wifiConnection.handleConnection();

                // シリアル入力の処理
                if (Serial.available() > 0) {
                    String angles = Serial.readString();
                    angles.trim();

                    if (angles.length() > 0 && angles.substring(0, 1) != "Ch" && angles.substring(0, 1) != "Re") {
                        for (int i = 0; i < 9; ++i) {
                            // 安全にサブストリングを取得する
                            if (angles.length() >= (i * 4 + 4)) {
                                String angle_i = angles.substring(i * 4, i * 4 + 4);
                                float angle_value = angle_i.toFloat();

                                // 有効な範囲か確認
                                if (!isnan(angle_value)) {
                                    int pos_i = krs.degPos(round(angle_value));
                                    krs.setPos(i + 1, pos_i);
                                    Serial.println(angle_i);
                                } else {
                                    Serial.println("Invalid angle value: " + angle_i);
                                }
                            }
                        }
                    }
                }
            }

            wifiConnection.setClientConnected(false);
            Serial.println("Client disconnected");
        }
    }
}