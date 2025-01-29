#include <Arduino.h>
#include <IcsHardSerialClass.h>

// サーボ設定
const byte EN_PIN = 32;  // ENピン
const byte RX_PIN = 33;  // RXピン
const byte TX_PIN = 25;  // TXピン
const long BAUDRATE = 1250000;  // ボーレート
const int TIMEOUT = 20;  // タイムアウト

const int SERVO_NUM = 8;

int defaultSpeed[SERVO_NUM] = {30, 30, 30, 30, 30, 30, 30, 30};
int timeCount = 0;
int angles[SERVO_NUM] = {45, 45, 45, 45, 45, 45, 45, 45};  // サーボ角度配列の初期化
String newMessage;
String prevMessage;

IcsHardSerialClass krs(&Serial2, EN_PIN, BAUDRATE, TIMEOUT);


void setup(){
    Serial2.begin(BAUDRATE, SERIAL_8E1, RX_PIN, TX_PIN, false, TIMEOUT);
    krs.begin();
}

void loop(){
    
    /*for (int i = 0; i < SERVO_NUM; i++){
        angles[i] = -angles[i];
    }*/

    const int MAX_RETRY = 50;

    // 受信した角度情報をサーボに反映
    for (int i = 0; i < SERVO_NUM; i++) {

        int retryCount = 0;
        bool speedSet = false;
        while (retryCount < MAX_RETRY && !speedSet){
            if (krs.setSpd(i + 1, defaultSpeed[i]) != -1){
                speedSet = true;
            } else {
                retryCount++;
                if (retryCount == MAX_RETRY){
                    Serial.printf("Failed to set speed for servo %d\n", i + 1);
                }
            }
        }
        delay(1);

        retryCount = 0;
        bool posSet = false;
        int pos_i = krs.degPos(angles[i]);
        while (retryCount < MAX_RETRY && !posSet){
            if (krs.setPos(i + 1, pos_i) != -1){
                posSet = true;
            } else {
                retryCount++;
                if (retryCount == MAX_RETRY){
                    Serial.printf("Failed to set position for servo %d\n", i + 1);
                }
            }
        }
        delay(1);
    }

    // delay(5000);
}