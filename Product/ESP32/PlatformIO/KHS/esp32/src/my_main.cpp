#include <Arduino.h>
#include <IcsHardSerialClass.h>
#include "wifi_connection.h"


// サーボ設定
const byte EN_PIN = 32;  // ENピン
const byte RX_PIN = 33;  // RXピン
const byte TX_PIN = 25;  // TXピン
const long BAUDRATE = 1250000;  // ボーレート
const int TIMEOUT = 20;  // タイムアウト

const int SERVO_NUM = 7;
const int MAX_RETRY = 5;

/*int messageCount = 0;
const int MINIMUM_MESSAGE_COUNT = 10;
bool started = false;*/

int defaultSpeed[SERVO_NUM] = {50};
float angles[SERVO_NUM] = {0};  // サーボ角度配列の初期化
String newMessage;
String prevMessage;


IcsHardSerialClass krs(&Serial2, EN_PIN, BAUDRATE, TIMEOUT);
WiFiConnection wifiConnection;

WiFiClient currentClient;


void setup() {
    // シリアル通信初期化
    Serial.begin(115200); // WiFiデバッグ用
    Serial2.begin(BAUDRATE, SERIAL_8E1, RX_PIN, TX_PIN, false, TIMEOUT);
    delay(100);
    krs.begin();

    // EN_PIN設定
    pinMode(EN_PIN, OUTPUT);
    digitalWrite(EN_PIN, HIGH); // サーボ電源ON
    delay(50);

    // WiFi接続
    if (wifiConnection.begin()) {
        Serial.println("WiFi Connected");
        Serial.println(WiFi.localIP());
    }
}


void loop() {
    static bool hasClient = false;  // クライアント接続状態管理

    // WiFi接続を監視
    wifiConnection.handleConnection();

    if (wifiConnection.isConnected()) {
        
        // 新しいクライアントを検知
        if (!hasClient) {
            WiFiClient newClient = wifiConnection.getServer()->available();
            if (newClient) {
                currentClient = newClient;
                hasClient = true;
                Serial.println("Client connected");
                wifiConnection.setClientConnected(true);
            }
        }

        if (hasClient && currentClient.connected()) {

            if (currentClient.available()) {
                newMessage = currentClient.readStringUntil('\n');

                // メッセージ更新時のみ処理
                if (newMessage != prevMessage) {
                    prevMessage = newMessage;
                    /*messageCount++;
                    if (messageCount > MINIMUM_MESSAGE_COUNT) {
                        started = true;
                    }*/

                    int startIdx = 0;
                    while (startIdx < newMessage.length()) {
                        int delimiterIdx = newMessage.indexOf(';', startIdx);
                        if (delimiterIdx == -1) break;

                        String command = newMessage.substring(startIdx, delimiterIdx);
                        int colonIdx = command.indexOf(':');
                        if (colonIdx != -1) {
                            int servoID = command.substring(0, colonIdx).toInt();
                            if (servoID >= 1 && servoID <= SERVO_NUM) {
                                angles[servoID - 1] = command.substring(colonIdx + 1).toFloat();
                            } else {
                                Serial.printf("無効なサーボID: %d\n", servoID);
                            }
                        }
                        startIdx = delimiterIdx + 1;
                    }

                    for (int i = 0; i < SERVO_NUM; i++) {
                        Serial.printf("%d: %f\n", i + 1, angles[i]);
                    }
                }
            }


        } else {
            // クライアントが切断された場合
            hasClient = false;
            wifiConnection.setClientConnected(false);
            Serial.println("Client disconnected");
        }

    } else {
        // WiFiが切断された場合
        wifiConnection.handleDisconnection();
        hasClient = false;
    }


    // if (started) {
    //speedSet
    /*for (int i = 0; i < SERVO_NUM; i++) {
        while (!speedSet[i]) {
            delay(1);
            if (krs.setSpd(i + 1, defaultSpeed[i]) != -1) {
                speedSet[i] = true;
            }
        }
    }*/

    for (int i = 0; i < SERVO_NUM; i++) {
        
        int retryCount = 0;
        bool speedSet = false;
        int speed_i = 50;
        while (retryCount < MAX_RETRY && !speedSet) {
            if (krs.setSpd(i + 1, speed_i) != -1) {
                speedSet = true;
            } else {
                retryCount++;
                if (retryCount == MAX_RETRY) {
                    Serial.printf("Failed to set speed for servo %d\n", i + 1);
                }
            }
        }

        retryCount = 0;
        bool posSet = false;
        int pos_i = krs.degPos(angles[i]);
        while (retryCount < MAX_RETRY && !posSet) {
            if (krs.setPos(i + 1, pos_i) != -1) {
                posSet = true;
            } else {
                retryCount++;
                if (retryCount == MAX_RETRY) {
                    Serial.printf("Failed to set position for servo %d\n", i + 1);
                }
            }
        }

        delay(1);
    }

}