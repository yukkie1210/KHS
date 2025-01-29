#include <WiFi.h>
#include <Arduino.h>
#include "esp_wpa2.h"
#include <IcsHardSerialClass.h>

#include "credentials.h"  // 認証情報をインクルード

const char *home_ssid = HOME_SSID;
const char *home_password = HOME_PASSWORD;
const char *utokyo_ssid = UTOKYO_SSID;
const char *utokyo_username = UTOKYO_USERNAME;
const char *utokyo_password = UTOKYO_PASSWORD;



// サーボ設定
const byte EN_PIN = 32;  // ENピン
const byte RX_PIN = 33;  // RXピン
const byte TX_PIN = 25;  // TXピン
const long BAUDRATE = 1250000;  // ボーレート
const int TIMEOUT = 20;  // タイムアウト

const int SERVO_NUM = 8;

int defaultSpeed[SERVO_NUM] = {30, 30, 30, 30, 30, 30, 30};

IcsHardSerialClass krs(&Serial2, EN_PIN, BAUDRATE, TIMEOUT);



// === 関数プロトタイプ ===
static void handleInitMode();
static void sendVec2ServoPos(int posVec[SERVO_NUM], int speedVec[SERVO_NUM]);
static void processMessage(String message);



// TCP server port
WiFiServer server(46963);
#define LED_BUILTIN 2

// 接続状態を追跡
bool clientConnected = false;
bool isUTokyoWiFi = false;

// WiFi接続を試みる関数
bool connectToWiFi(const char* ssid, bool isUTokyo = false) {
    Serial.print("\nAttempting to connect to: ");
    Serial.println(ssid);
    
    if (isUTokyo) {
        // UTokyo WiFiの場合
        WiFi.disconnect(true);
        WiFi.mode(WIFI_STA);
        
        esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)utokyo_username, strlen(utokyo_username));
        esp_wifi_sta_wpa2_ent_set_username((uint8_t *)utokyo_username, strlen(utokyo_username));
        esp_wifi_sta_wpa2_ent_set_password((uint8_t *)utokyo_password, strlen(utokyo_password));
        esp_wifi_sta_wpa2_ent_enable();
        
        WiFi.begin(ssid);
    } else {
        // 家のWiFiの場合
        WiFi.begin(ssid, home_password);
    }
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected successfully!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    
    Serial.println("\nConnection failed!");
    return false;
}

void setup() {
    //wifiのシリアル通信
    Serial.begin(115200); //先に設定する必要が有る

    Serial.begin(BAUDRATE, SERIAL_8E1, RX_PIN, TX_PIN, false, TIMEOUT);
    delay(100);
    krs.begin();

    // EN_PINの設定を追加 この部分を消すと通信はうまく行くがサーボがonにならない
    //この部分があるとサーボがonになるが通信がうまく行かない
    pinMode(EN_PIN, OUTPUT);
    digitalWrite(EN_PIN, HIGH); 
    delay(50); 

    handleInitMode();
    
    // まず家のWiFiに接続を試みる
    if (connectToWiFi(home_ssid)) {
        isUTokyoWiFi = false;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        // TCPサーバーを開始
        server.begin();
        Serial.println("TCP Server started");
    }
}

void loop() {
    // WiFi接続が切れた場合
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi connection lost. Attempting to reconnect...");
        clientConnected = false;
        
        // 現在のネットワークに再接続を試みる
        bool reconnected = connectToWiFi(isUTokyoWiFi ? utokyo_ssid : home_ssid, isUTokyoWiFi);
        
        // 再接続に失敗した場合、別のネットワークを試す
        if (!reconnected) {
            isUTokyoWiFi = !isUTokyoWiFi;
            connectToWiFi(isUTokyoWiFi ? utokyo_ssid : home_ssid, isUTokyoWiFi);
        }
        
        // 接続できた場合、TCPサーバーを再起動
        if (WiFi.status() == WL_CONNECTED) {
            server.begin();
        }
    }
    
    // WiFi接続中の処理
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client = server.available();
        
        if (client) {  // クライアント接続時
            Serial.println("New client connected!");
            clientConnected = true;
            
            if (client.connected()) {
                if (client.available()) {
                    String request = client.readStringUntil('\n');
                    processMessage(request);
                    
                    if (request.indexOf("quit") != -1) {
                        client.stop();
                        clientConnected = false;
                        Serial.println("Client disconnected by command");
                    }
                
                // クライアント接続中は早い点滅
                digitalWrite(LED_BUILTIN, HIGH);
                delay(100);
                digitalWrite(LED_BUILTIN, LOW);
                delay(100);
                }
            }
        }
    }
} /*else {
        // WiFi未接続時は遅い点滅
        digitalWrite(LED_BUILTIN, HIGH);
        delay(3000);
        digitalWrite(LED_BUILTIN, LOW);
        delay(3000);
    }
}*/

// 初期モード処理
static void handleInitMode() {
    int positions[SERVO_NUM];
    int pos = krs.degPos(0);

    for (int i = 0; i < SERVO_NUM; ++i) {
        positions[i] = pos;
    }
    sendVec2ServoPos(positions, nullptr);
}

static void sendVec2ServoPos(int posVec[SERVO_NUM], int speedVec[SERVO_NUM]){
    //static int defaultSpeed[SERVO_NUM] = {127, 127, 127, 127, 127, 127, 127};
    if (speedVec == nullptr) {
        speedVec = defaultSpeed;
    }
    
    // 各サーボについて最大5回までリトライ
    const int MAX_RETRY = 5;
    
    for (int i = 1; i < SERVO_NUM; ++i) {
    //for (int i = 0; i < SERVO_NUM; ++i) {
        // スピード設定
        int retryCount = 0;
        bool speedSet = false;
        while (retryCount < MAX_RETRY && !speedSet) {
            if (krs.setSpd(i, speedVec[i]) != -1) {
                speedSet = true;
            } else {
                retryCount++;
                if (retryCount == MAX_RETRY) {
                    Serial.printf("Failed to set speed for servo %d\n", i);
                }
            }
        }
        
        // ポジション設定
        retryCount = 0;
        bool posSet = false;
        while (retryCount < MAX_RETRY && !posSet) {
            if (krs.setPos(i, posVec[i]) != -1) {
                posSet = true;
            } else {
                retryCount++;
                if (retryCount == MAX_RETRY) {
                    Serial.printf("Failed to set position for servo %d\n", i);
                }
            }
        }
        
        Serial.printf("Servo %d: pos=%d, speed=%d\n", i, posVec[i], speedVec[i]);
        
        // 各サーボ設定後に短い待機時間を入れる
        delay(1);
    }
}

// メッセージの処理
static void processMessage(String message) {
    Serial.printf("受信したメッセージ: %s\n", message.c_str());

    int angles[SERVO_NUM] = {0};  // サーボ角度配列の初期化
    int startIdx = 0;

    while (startIdx < message.length()) {
        int delimiterIdx = message.indexOf(';', startIdx);
        if (delimiterIdx == -1) break;

        String command = message.substring(startIdx, delimiterIdx);
        int colonIdx = command.indexOf(':');
        if (colonIdx != -1) {
            int servoID = command.substring(0, colonIdx).toInt();
            if (servoID >= 1 && servoID <= SERVO_NUM) {
                angles[servoID - 1] = round(command.substring(colonIdx + 1).toFloat());
            } else {
                Serial.printf("無効なサーボID: %d\n", servoID);
            }
        }
        startIdx = delimiterIdx + 1;
    }

    for (int i = 0; i < SERVO_NUM; ++i) {
        Serial.printf("サーボ %d の角度: %d\n", i + 1, angles[i]);
    }

    sendVec2ServoPos(angles, nullptr);
}