#include <Arduino.h>
#include <IcsHardSerialClass.h>
#include "wifi_connection.h"

// サーボ設定
const byte EN_PIN = 3;  // EN
const byte RX_PIN = 7;  // 35
const byte TX_PIN = 6;  // 34
const long BAUDRATE = 1250000;
const int TIMEOUT = 20;
const int SERVO_NUM = 8;

// グローバル変数
IcsHardSerialClass krs(&Serial2, EN_PIN, BAUDRATE, TIMEOUT);
WiFiConnection wifiConnection;

// エラーステータス
struct ErrorStatus {
    bool wifiDisconnected = false;
    bool servoError = false;
    bool angleOutOfRange = false;
    String errorMessage;
};

// サイクルタイマー
struct CycleTimer {
    unsigned long startTime;
    double periodSec;

    double getCurrentTimeRatio() {
        return fmod((millis() - startTime) / (periodSec * 1000.0), 1.0);
    }

    void reset() {
        startTime = millis();
    }

    void start(double period) {
        periodSec = period;
        reset();
    }

    bool isCycleComplete() {
        return getCurrentTimeRatio() >= 1.0;
    }
};

class Main {
protected:
    ErrorStatus errorStatus;

    const unsigned long WIFI_RECONNECT_INTERVAL = 5000;
    const unsigned long CLIENT_TIMEOUT = 5000;
    const unsigned long MOTION_UPDATE_INTERVAL = 50;  // 50msで更新

    unsigned long lastMotionUpdate = 0;
    unsigned long lastClientActivity = 0;

    WiFiClient currentClient;  // 現在のクライアント
    bool hasReceivedFirstCommand = false;  // 初回コマンド受信フラグ
    bool isTimeout = false;  // タイムアウト状態
    bool hasClient = false;  // クライアント接続状態

public:
    virtual ~Main() {
        setServoOff();
    }

    static void initializeSystem() {

        // Wi-Fiシリアル通信の初期化
        Serial.begin(115200);

        // サーボ用シリアル通信の初期化
        Serial2.begin(BAUDRATE, SERIAL_8E1, RX_PIN, TX_PIN, false, TIMEOUT);
        delay(100);
        krs.begin();

        // EN_PINの初期設定
        pinMode(EN_PIN, OUTPUT);
        digitalWrite(EN_PIN, HIGH);
        delay(50);

        // Wi-Fi接続の初期化
        if (wifiConnection.begin()) {
            Serial.println("WiFi Connected");
            Serial.println(WiFi.localIP());
        }

        handleInitMode();
    }

    void loop() {
        // タイムアウト処理
        unsigned long currentTime = millis();
        if (isTimeout && currentTime - lastClientActivity > CLIENT_TIMEOUT) {
            Serial.println("Activity timeout - switching to SERVO_OFF");
            setServoOff();
            isTimeout = true;
            hasClient = false;
            wifiConnection.setClientConnected(false);
        }

        // Wi-Fi接続状態の確認
        if (wifiConnection.isConnected()) {
            manageClientConnection(currentTime);
        } else {
            handleWifiDisconnection();
            if (!isTimeout) {
                Serial.println("WiFi disconnected - switching to SERVO_OFF");
                setServoOff();
                isTimeout = true;
            }
        }
    }

protected:
    void manageClientConnection(unsigned long currentTime) {
        if (!hasClient) {
            WiFiClient newClient = wifiConnection.getServer()->available();
            if (newClient) {
                currentClient = newClient;
                hasClient = true;
                Serial.println("Client connected");
                wifiConnection.setClientConnected(true);
                lastClientActivity = currentTime;
                isTimeout = false;
            }
        }

        if (hasClient && currentClient.connected()) {
            wifiConnection.handleConnection();

            if (currentClient.available()) {
                hasReceivedFirstCommand = true;
                lastClientActivity = currentTime;
                isTimeout = false;

                String message = currentClient.readStringUntil('\n');
                processMessage(message);
            }
        } else {
            // クライアント切断処理
            Serial.println("Client disconnected");
            hasClient = false;
            wifiConnection.setClientConnected(false);
        }
    }

    void setServoOff() {
        for (int i = 0; i < SERVO_NUM; ++i) {
            krs.setFree(i + 1);  // サーボをオフにする
        }
    }

    void handleWifiDisconnection() {
        static unsigned long lastReconnectAttempt = 0;
        unsigned long currentTime = millis();

        if (currentTime - lastReconnectAttempt >= WIFI_RECONNECT_INTERVAL) {
            if (wifiConnection.begin()) {
                Serial.println("WiFi Reconnected");
            }
            lastReconnectAttempt = currentTime;
        }
    }

    static void handleInitMode() {
        int positions[SERVO_NUM];
        int speeds[SERVO_NUM] = {30, 30, 30, 30, 30, 30, 30, 30};
        int pos = krs.degPos(0);

        for (int i = 0; i < SERVO_NUM; ++i) {
            positions[i] = pos;
        }
        sendVec2ServoPos(positions, speeds);
    }

    static void sendVec2ServoPos(int posVec[SERVO_NUM], int speedVec[SERVO_NUM]) {
        const int MAX_RETRY = 5;

        for (int i = 1; i <= SERVO_NUM; ++i) {
            // スピード設定
            if (!retryCommand(MAX_RETRY, [&]() { return krs.setSpd(i, speedVec[i - 1]); })) {
                Serial.printf("Failed to set speed for servo %d\n", i);
            }

            // ポジション設定
            if (!retryCommand(MAX_RETRY, [&]() { return krs.setPos(i, posVec[i - 1]); })) {
                Serial.printf("Failed to set position for servo %d\n", i);
            }
        }
    }

    static bool retryCommand(int maxRetry, std::function<int()> command) {
        for (int retry = 0; retry < maxRetry; ++retry) {
            if (command() != -1) {
                return true;
            }
        }
        return false;
    }

    static void processMessage(String message) {
        Serial.printf("Received message: %s\n", message.c_str());

        int angles[SERVO_NUM] = {0};
        int startIdx = 0;

        while (startIdx < message.length()) {
            int delimiterIdx = message.indexOf(';', startIdx);
            if (delimiterIdx == -1) break;

            String command = message.substring(startIdx, delimiterIdx);
            int colonIdx = command.indexOf(':');
            if (colonIdx != -1) {
                int servoID = command.substring(0, colonIdx).toInt();
                angles[servoID - 1] = round(command.substring(colonIdx + 1).toFloat());
            }
            startIdx = delimiterIdx + 1;
        }

        for (int i = 0; i < SERVO_NUM; ++i) {
            Serial.printf("Servo %d angle: %d\n", i + 1, angles[i]);
        }

        sendVec2ServoPos(angles, nullptr);
    }
};

// グローバルインスタンス
Main obj;

void setup() {
    Main::initializeSystem();
}

void loop() {
    obj.loop();
}