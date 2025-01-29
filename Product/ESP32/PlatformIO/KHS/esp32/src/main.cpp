#include <Arduino.h>
#include <IcsHardSerialClass.h>
#include "wifi_connection.h"
#include "message_processor.h"
#include "motion_patterns.h"

// サーボ設定
const byte EN_PIN = 5;
const byte RX_PIN = 16;
const byte TX_PIN = 17;
const long BAUDRATE = 1250000;
const int TIMEOUT = 20;

const int SERVO_NUM = 7;

// グローバル変数として定義
IcsHardSerialClass krs(&Serial2, EN_PIN, BAUDRATE, TIMEOUT);
WiFiConnection wifiConnection;
MessageProcessor messageProcessor;

WiFiClient currentClient;

// エラーステータス
struct ErrorStatus {
    bool wifiDisconnected = false;
    bool servoError = false;
    bool angleOutOfRange = false;
    String errorMessage;
};

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

class CrushMain {
protected:
    //static IcsHardSerialClass krs;
    // static WiFiConnection wifiConnection;
    //static WiFiClient currentClient;
    // static MessageProcessor messageProcessor;
    
    CrushMode currentMode;
    CrushMode previousMode;

    SwimParameters currentParams;  // 追加：現在のパラメータを保持
    WingUpMode currentWingMode;

    ErrorStatus errorStatus;
    
    const double MAX_WING_ANGLE = 25.0;
    const double MIN_WING_ANGLE = -25.0;
    const unsigned long WIFI_RECONNECT_INTERVAL = 5000;
    const unsigned long MODE_TRANSITION_DELAY = 1000;
    
    unsigned long lastMotionUpdate = 0;
    const unsigned long MOTION_UPDATE_INTERVAL = 50;  // 20ms間隔で更新

    virtual void updateMotion() = 0;
    virtual void handleEmergencySurface() = 0;  // 追加

public:
    CrushMain() : currentMode(CrushMode::INIT_POSE), 
                  previousMode(CrushMode::SERVO_OFF) {}
    
    virtual ~CrushMain() {
        setServoOff();
    }

    static void initializeSystem() {
        // // LED初期化
        // pinMode(LED_BUILTIN, OUTPUT);
        // digitalWrite(LED_BUILTIN, LOW);

        //wifiのシリアル通信
        Serial.begin(115200); //先に設定する必要が有る


        //モータのシリアル通信
        Serial2.begin(BAUDRATE, SERIAL_8E1, RX_PIN, TX_PIN, false, TIMEOUT);
        delay(100);
        krs.begin();

        // EN_PINの設定を追加 この部分を消すと通信はうまく行くがサーボがonにならない
        //この部分があるとサーボがonになるが通信がうまく行かない
        pinMode(EN_PIN, OUTPUT);
        digitalWrite(EN_PIN, HIGH); 
        delay(50); 



        if (wifiConnection.begin()) {
            Serial.println("WiFi Connected");
            Serial.println(WiFi.localIP());
        }
    }

    virtual void loop() {
        static bool hasReceivedFirstCommand = false;  // 初回コマンド受信フラグ
        static unsigned long lastClientActivity = 0;
        const unsigned long CLIENT_TIMEOUT = 5000;  // 5秒のタイムアウト
        static bool isTimeout = false;
        static bool wasConnected = false; 

        static WiFiClient currentClient;  // クライアントをstatic変数として保持
        static bool hasClient = false;    // クライアント接続状態を保持

        // 初回コマンドを受信するまではサーボオフ
        if (!hasReceivedFirstCommand) {
            if (currentMode != CrushMode::SERVO_OFF) {
                currentMode = CrushMode::SERVO_OFF;
                setServoOff();
            }
        }

        // wifi接続
        wifiConnection.handleConnection();

        // 現在の時刻を取得
        unsigned long currentTime = millis();

        // タイムアウトチェックをループの最初で実行
        if (!isTimeout && hasReceivedFirstCommand && (currentTime - lastClientActivity > CLIENT_TIMEOUT)) {
            Serial.println("Activity timeout - switching to SERVO_OFF");
            currentMode = CrushMode::SERVO_OFF;
            setServoOff();
            isTimeout = true;
            hasClient = false;  // クライアント接続状態もリセット
            wifiConnection.setClientConnected(false);
        }

        if (wifiConnection.isConnected()) {
            //WiFiClient client = wifiConnection.getServer()->available();毎回clientを接続

            // クライアントがまだ接続されていない場合のみ、新しいクライアントを受け付ける
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
                
                // test_tcp_ics.cppと同様の処理構造に修正
                //while (client.connected()) {
            if (hasClient && currentClient.connected()) { //ifにしてみました　whileだと抜け出せないので
                wifiConnection.handleConnection();   
                currentTime = millis();  // ループ内で時刻を更新 

                if (messageProcessor.processMessage(currentClient)) {
                    hasReceivedFirstCommand = true;  // 初回コマンド受信フラグを立てる
                    lastClientActivity = currentTime;  // メッセージを受信したら時間を更新
                    isTimeout = false; 

                    // メッセージを受信したときだけモーション更新
                    auto mode = messageProcessor.getCurrentMode();
                    auto params = messageProcessor.getCurrentParams();
                    auto wingMode = messageProcessor.getCurrentWingMode();
                    currentMode = mode;
                    currentParams = params;  // パラメータを保存
                    currentWingMode = wingMode;  // パラメータを保存

                    // デバッグ出力を追加
                    Serial.printf("Mode: %d, Period: %.2f, Wing: %.1f, Max: %.1f, Y: %.2f\n",
                        static_cast<int>(mode),
                        params.periodSec,
                        params.wingDeg,
                        params.maxAngleDeg,
                        params.yRate);

                    //updateMotion();  // 各クラスで実装される処理
                }
                
                // タイムアウトチェックをループ内でも実行
                if (!isTimeout && (currentTime - lastClientActivity > CLIENT_TIMEOUT)) {
                    Serial.println("Activity timeout - switching to EMERGENCY_SURFACE");
                    currentMode = CrushMode::EMERGENCY_SURFACE;
                    handleEmergencySurface();
                    isTimeout = true;
                }
                
            } else {
                // クライアントが切断された場合の処理
                hasClient = false;
                wifiConnection.setClientConnected(false);
                Serial.println("Client disconnected");
            }

        } else {
            handleWifiDisconnection();
            if (!isTimeout) {
                Serial.println("WiFi disconnected - switching to SERVO_OFF");
                currentMode = CrushMode::SERVO_OFF;
                setServoOff();
                isTimeout = true;
            }
            hasClient = false;
        }

                // 継続的なモーション更新（クライアント接続状態に関係なく実行）
        if (!isTimeout &&hasReceivedFirstCommand) {
            if (currentTime - lastMotionUpdate >= MOTION_UPDATE_INTERVAL) {
                updateMotion();
                lastMotionUpdate = currentTime;
            }
        }
    }

protected:
    void setServoOff() {
        for (int i = 0; i < SERVO_NUM; ++i) {
            krs.setFree(i);//変換したデータをID:0に送る
        }
    }

    bool isAngleValid(double angle) {
        if (angle < MIN_WING_ANGLE || angle > MAX_WING_ANGLE) {
            errorStatus.angleOutOfRange = true;
            errorStatus.errorMessage = "Angle out of range: " + String(angle);
            return false;
        }
        return true;
    }

    void handleWifiDisconnection() {
        static unsigned long lastReconnectAttempt = 0;
        const unsigned long RECONNECT_INTERVAL = 5000;  // 5秒ごとに再接続を試みる
        
        unsigned long currentTime = millis();
        if (currentTime - lastReconnectAttempt >= RECONNECT_INTERVAL) {
            if (wifiConnection.begin()) {
                Serial.println("WiFi Reconnected");
            }
            lastReconnectAttempt = currentTime;
        }
    }
    
};

// 静的メンバの定義
//IcsHardSerialClass CrushMain::krs(&Serial2, EN_PIN, BAUDRATE, TIMEOUT);

//WiFiConnection CrushMain::wifiConnection;
//MessageProcessor CrushMain::messageProcessor;

class CrushBody : public CrushMain {
private:
    SplineInterpolator splineInterpolator;
    std::vector<WingMotionPoint> currentPattern;
    CycleTimer cycleTimer;
    //const int BODY_SERVO_ID = 0;
    unsigned long lastUpdateTime = 0;
    
    
    CrushMode currentMode;

    //parameters
    // クラス内でパラメータを保持
    const int BASE_SPEED = 60;
    const int SPEED_VARIATION = 40;
    
    // サーボIDの定義を明確に
    const int RIGHT_UP_DOWN_ID = 1;
    const int RIGHT_FRONT_BACK_ID = 2;
    const int LEFT_UP_DOWN_ID = 4;
    const int LEFT_FRONT_BACK_ID = 5;    

    const double DEFAULT_SURFACE_ANGLE = 30.0;
    const double WING_ROTATION_ANGLE = 90.0;

    unsigned long lastMotionUpdate = 0;
    const unsigned long MOTION_UPDATE_INTERVAL = 20;  // 20ms間隔で更新


protected:
    void updateMotion() override {
        auto mode = messageProcessor.getCurrentMode();
        auto params = messageProcessor.getCurrentParams();
        // WiFi通信が来ても、モードが同じなら継続
        if (mode != currentMode) {
            // モードが変更された時のみ初期化処理を行う
            Serial.printf("Mode changed from %d to %d\n", static_cast<int>(currentMode), static_cast<int>(mode));
            currentMode = mode;
            cycleTimer.reset();  // タイマーをリセット
            // その他の初期化処理
        }
        
        switch (currentMode) {
            case CrushMode::SERVO_OFF:
                setServoOff();
                break;
            case CrushMode::INIT_POSE:
                handleInitMode();
                break;
            case CrushMode::STAY:
                handleStayMode(currentParams);
                break;
            case CrushMode::SWIM:
                handleSwimMode(currentParams);
                break;
            case CrushMode::RAISE:
                handleRaiseMode(currentWingMode);
                break;
            case CrushMode::EMERGENCY_SURFACE:
                handleEmergencySurface();
                break;
        }
    }

private:
    // void sendVec2ServoPos(int posVec[SERVO_NUM], int speedVec[SERVO_NUM]){
    //     static int defaultSpeed[SERVO_NUM] = {127, 127, 127, 127, 127, 127, 127};

    //     if (speedVec == nullptr) {
    //         speedVec = defaultSpeed;
    //     }
    //     for (int i = 0; i < SERVO_NUM; ++i) {
    //         //if (isAngleValid(posVec[i])) {
    //             while (krs.setSpd(i,speedVec[i]) == -1) {
    //                 delay(1);
    //             }
    //             while (krs.setPos(i, posVec[i]) == -1) {
    //                 delay(1);
    //             }
    //         //}
    //         Serial.printf("Servo %d: pos=%d, speed=%d\n", i, posVec[i], speedVec[i]);
    //     }
        
    // }

    void sendVec2ServoPos(int posVec[SERVO_NUM], int speedVec[SERVO_NUM]){
        //static int defaultSpeed[SERVO_NUM] = {127, 127, 127, 127, 127, 127, 127};
        static int defaultSpeed[SERVO_NUM] = {50, 50, 50, 50, 50, 50, 50};
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

        void handleInitMode() {
        // if (currentMode == CrushMode::INIT_POSE) {
            int positions[SERVO_NUM];
            int speeds[SERVO_NUM] = {30, 30, 30, 30, 30, 30, 30};
            int pos = krs.degPos(0);
            for (int i = 1; i < SERVO_NUM; ++i) {
                positions[i] = pos;
            }
            sendVec2ServoPos(positions, speeds);
        }

    void handleStayMode(const SwimParameters& params) {
        // 固定値の設定
        const double PERIOD_MS = 3000.0;  // 3秒周期
        const double MAX_ANGLE = 20.0;    // 振幅±20度
        
        // 現在の時間から角度を計算
        unsigned long currentTime = millis();
        //double timeRatio = fmod(currentTime, PERIOD_MS) / PERIOD_MS;  // 0.0 ~ 1.0の値
        //double currentAngle = MAX_ANGLE * sin(TWO_PI * timeRatio);    // -30 ~ +30度
            // paramsを使用して処理
    double timeRatio = fmod(currentTime, params.periodSec * 1000.0) / (params.periodSec * 1000.0);
    double currentAngle = params.maxAngleDeg * sin(TWO_PI * timeRatio);

        // サーボの位置と速度を設定
        int positions[SERVO_NUM] = {0};  // 0番は使わない
        int speeds[SERVO_NUM] = {127, 127, 127, 127, 127, 127, 127};  // 一定速度
        
            // wingdegをラジアンに変換
            double WING_DEG = 0.0;

            double wingRad = WING_DEG * PI / 180.0;
            
            // サーボの角度を計算
            double angle1 = currentAngle * cos(wingRad);  // サーボ1,4用
            double angle2 = currentAngle * sin(wingRad);  // サーボ2,5用
        
            // 各サーボに角度を設定
        positions[1] = krs.degPos(angle1);  // 右の上下
        positions[2] = krs.degPos(angle2);  // 右の前後
        positions[4] = krs.degPos(-angle1);  // 左の上下
        positions[5] = krs.degPos(-angle2);  // 左の前後
        
        // 3番と6番は0度に維持
        positions[3] = krs.degPos(0);
        positions[6] = krs.degPos(0);
        
        // サーボに送信
        sendVec2ServoPos(positions, speeds);
        
        // デバッグ出力（500msごと）
        static unsigned long lastDebugTime = 0;
        if (currentTime - lastDebugTime > 500) {
            Serial.printf("Stay Mode: Current angle = %.2f\n", currentAngle);
            lastDebugTime = currentTime;
        }
    }

// void handleStayMode(const SwimParameters& params) {
//     // モード遷移の処理（これは保持）
//     if (previousMode != CrushMode::STAY) {
//         cycleTimer.reset();
//         previousMode = CrushMode::STAY;
//     }

//     // 周期の更新確認
//     if (cycleTimer.periodSec != params.periodSec) {
//         cycleTimer.start(params.periodSec);
//     }

//     // 時間比率の計算（0.0 ~ 1.0）
//     double timeRatio = cycleTimer.getCurrentTimeRatio();
    
//     // wingdegをラジアンに変換
//     double wingRad = params.wingDeg * PI / 180.0;
    
//     // サーボの角度を計算
//     double angle1 = params.maxAngleDeg * sin(wingRad) * sin(TWO_PI * timeRatio);  // サーボ1,4用
//     double angle2 = params.maxAngleDeg * cos(wingRad) * sin(TWO_PI * timeRatio);  // サーボ2,5用
    
//     // サーボの位置と速度を設定
//     int positions[SERVO_NUM] = {0};  // 0番は使わない
//     int speeds[SERVO_NUM] = {127, 127, 127, 127, 127, 127, 127};
    
//     // 各サーボに角度を設定
//     positions[1] = krs.degPos(angle1);  // 右の上下
//     positions[2] = krs.degPos(angle2);  // 右の前後
//     positions[4] = krs.degPos(-angle1);  // 左の上下
//     positions[5] = krs.degPos(-angle2);  // 左の前後
    
//     // 3番と6番は0度に維持
//     positions[3] = krs.degPos(0);
//     positions[6] = krs.degPos(0);
    
//     // サーボに送信
//     sendVec2ServoPos(positions, speeds);

//     // デバッグ出力（500msごと）
//     static unsigned long lastDebugTime = 0;
//     if (millis() - lastDebugTime > 500) {
//         Serial.printf("Stay Mode: angle1=%.2f, angle2=%.2f\n", angle1, angle2);
//         lastDebugTime = millis();
//     }
// }
    

void handleSwimMode(const SwimParameters& params) {
    // 固定値の設定
    // const double PERIOD_MS = 2000.0;      // 2秒周期
    // const double MAX_ANGLE = 20.0;        // 振幅±30度
    // const double WING_DEG = 20.0;         // 翼角度
    // const double RIGHT_RATE = 1.0; //1.2       // 右の振幅率（1.0より大きいと右に曲がる）
    // const double LEFT_RATE = 1.0;  //0.8       // 左の振幅率
    // const double WING_ROTATION = 30.0;    // 翼の回転角度

    double PERIOD_MS = params.periodSec * 1000.0;      // 2秒周期
    double MAX_ANGLE = params.maxAngleDeg;        // 振幅±30度
    double WING_DEG = params.wingDeg;         // 翼角度
    double RIGHT_RATE = (1.0 + params.yRate) / 2.0; //1.2       // 右の振幅率（1.0より大きいと右に曲がる）
    double LEFT_RATE =  (1.0 - params.yRate) / 2.0;  //0.8       // 左の振幅率
    double WING_ROTATION = 30.0;    // 翼の回転角度

    
    // 現在の時間から基本角度を計算
    unsigned long currentTime = millis();
    double timeRatio = fmod(currentTime, PERIOD_MS) / PERIOD_MS;
    double baseAngle = MAX_ANGLE * sin(TWO_PI * timeRatio);
    
    // 翼の方向計算
    double wingRad = WING_DEG * PI / 180.0;
    
    // 左右の振幅調整
    double rightAngle1 = baseAngle * RIGHT_RATE * cos(wingRad);
    double rightAngle2 = baseAngle * RIGHT_RATE * sin(wingRad);
    double leftAngle1 = baseAngle * LEFT_RATE * cos(wingRad);
    double leftAngle2 = baseAngle * LEFT_RATE * sin(wingRad);
    
    // 3番と6番サーボの制御（前進動作用）
    double rotationAngle = 0.0;
    if (!params.isBackward) {  // 前進の場合
        // baseAngle2が正から負に変化する領域（前から後ろに動かすとき）で90度
        if (cos(TWO_PI * timeRatio) < 0) {
            rotationAngle = WING_ROTATION;
        }
    } else {  // 後退の場合
        // baseAngle2が負から正に変化する領域（後ろから前に動かすとき）で90度
        if (cos(TWO_PI * timeRatio) > 0) {
            rotationAngle = WING_ROTATION;
        }
    }
    
    int positions[SERVO_NUM] = {0};
    int speeds[SERVO_NUM] = {127, 127, 127, 30, 127, 127, 30};
    
    // 各サーボに角度を設定
    positions[1] = krs.degPos(rightAngle1);   // 右の上下
    positions[2] = krs.degPos(rightAngle2);   // 右の前後
    positions[3] = krs.degPos(rotationAngle); // 右の回転
    positions[4] = krs.degPos(-leftAngle1);   // 左の上下
    positions[5] = krs.degPos(-leftAngle2);   // 左の前後
    positions[6] = krs.degPos(rotationAngle); // 左の回転
    
    sendVec2ServoPos(positions, speeds);
    
    // デバッグ出力
    static unsigned long lastDebugTime = 0;
    if (currentTime - lastDebugTime > 500) {
        Serial.printf("Swim Mode: Base=%.2f, Right=%.2f, Left=%.2f, Rotation=%.2f\n",
            baseAngle, rightAngle1, leftAngle1, rotationAngle);
        lastDebugTime = currentTime;
    }
}

//     void handleSwimMode(const SwimParameters& params) {
//         // パラメータの妥当性チェック


//         // モード遷移の処理
//         if (previousMode != CrushMode::SWIM) {
//             cycleTimer.reset();
//             previousMode = CrushMode::SWIM;
//         }

//         // 周期の更新確認
//         if (cycleTimer.periodSec != params.periodSec) {
//             cycleTimer.start(params.periodSec);
//         }

//         double timeRatio = cycleTimer.getCurrentTimeRatio();
//         double wingRad = params.wingDeg * PI / 180.0;
        
//         // 左右の振幅調整（yRateに基づく）
//         double rightAmplitude = params.maxAngleDeg * (1.0 + params.yRate) / 2.0;
//         double leftAmplitude = params.maxAngleDeg * (1.0 - params.yRate) / 2.0;
        
//         // 基本の角度計算
//         double baseAngle1 = sin(wingRad) * sin(TWO_PI * timeRatio);  // 上下運動
//         double baseAngle2 = cos(wingRad) * sin(TWO_PI * timeRatio);  // 前後運動
        
//         // 3番と6番サーボの角度計算（前進/後退用）
//         double angle3 = 0.0;
//         if (!params.isBackward) {  // 前進の場合
//             // baseAngle2が正から負に変化する領域（前から後ろに動かすとき）で90度
//             if (cos(TWO_PI * timeRatio) < 0) {
//                 angle3 = WING_ROTATION_ANGLE;
//             }
//         } else {  // 後退の場合
//             // baseAngle2が負から正に変化する領域（後ろから前に動かすとき）で90度
//             if (cos(TWO_PI * timeRatio) > 0) {
//                 angle3 = WING_ROTATION_ANGLE;
//             }
//         }

//         int positions[SERVO_NUM] = {0};
//         int speeds[SERVO_NUM] = {0};
        
//         // 速度計算
//         double speedFactor = abs(cos(TWO_PI * timeRatio));
//         int currentSpeed = BASE_SPEED + (int)(SPEED_VARIATION * speedFactor);
        
//         try {
//             // 右腕の設定
//             positions[RIGHT_UP_DOWN_ID] = krs.degPos(rightAmplitude * baseAngle1);
//             positions[RIGHT_FRONT_BACK_ID] = krs.degPos(rightAmplitude * baseAngle2);
//             positions[3] = krs.degPos(angle3);  // 右の傾き制御
            
//             // 左腕の設定
//             positions[LEFT_UP_DOWN_ID] = krs.degPos(leftAmplitude * baseAngle1);
//             positions[LEFT_FRONT_BACK_ID] = krs.degPos(leftAmplitude * baseAngle2);
//             positions[6] = krs.degPos(angle3);  // 左の傾き制御
            
//             // 速度の設定
//             for (int i = 1; i <= 6; i++) {
//                 speeds[i] = currentSpeed;
//             }
            
//             // 位置と速度を設定
//             sendVec2ServoPos(positions, speeds);
            
//         } catch (...) {
//             errorStatus.servoError = true;
//             errorStatus.errorMessage = "Servo control error in SwimMode";
//         }
// }



void handleRaiseMode(const WingUpMode& wingMode) {
    unsigned long currentTime = millis();
    int positions[SERVO_NUM] = {0};  // すべて0で初期化
    int speeds[SERVO_NUM] = {30, 30, 30, 30, 30, 30, 30};  // servo2,5のみ速度80
    
    // servo1,3,4,6は0度
    positions[1] = krs.degPos(0);
    positions[3] = krs.degPos(0);
    positions[4] = krs.degPos(0);
    positions[6] = krs.degPos(0);
    
    // // servo2,5は30度
    // positions[2] = krs.degPos(20);
    // positions[5] = krs.degPos(-20);
        // WingUpModeに応じてヒレの位置を設定
    
    
    switch(wingMode) {
        case WingUpMode::RIGHT:
            positions[2] = krs.degPos(20);  // 右のヒレのみ上げる
            positions[5] = krs.degPos(0);
            break;
            
        case WingUpMode::LEFT:
            positions[5] = krs.degPos(-20);  // 左のヒレのみ上げる
            positions[2] = krs.degPos(0);
            break;
            
        case WingUpMode::BOTH:
            positions[2] = krs.degPos(20);   // 両方のヒレを上げる
            positions[5] = krs.degPos(-20);
            break;
    }
    
    
    sendVec2ServoPos(positions, speeds);
    
    // デバッグ出力
    static unsigned long lastDebugTime = 0;
    if (millis() - lastDebugTime > 500) {
        Serial.printf("Raise Mode: servo2,5 at 30 degrees, speed 80\n");
        lastDebugTime = currentTime;
    }
}

//     void handleRaiseMode(const SwimParameters& params) {
//     // パラメータの妥当性チェック
//     // if (params.maxAngleDeg < 0) {
//     //     errorStatus.errorMessage = "Invalid parameters in RaiseMode";
//     //     return;
//     // }

//     // モード遷移の処理
//     if (previousMode != CrushMode::RAISE) {
//         previousMode = CrushMode::RAISE;
//     }

//     int positions[7] = {0};  // すべて0で初期化
//     int speeds[7] = {BASE_SPEED};  // 一定速度で動作
    
//     WingUpMode mode = messageProcessor.getCurrentWingMode();
    
//     try {
//         switch (mode) {
//             case WingUpMode::RIGHT:
//                 // 右側のヒレを上げる
//                 positions[RIGHT_UP_DOWN_ID] = krs.degPos(0);           // ID:1 は 0度
//                 positions[RIGHT_FRONT_BACK_ID] = krs.degPos(params.maxAngleDeg);  // ID:2 は最大角度
//                 positions[3] = krs.degPos(0);           // ID:3 は 0度
//                 break;
                
//             case WingUpMode::LEFT:
//                 // 左側のヒレを上げる
//                 positions[LEFT_UP_DOWN_ID] = krs.degPos(0);           // ID:4 は 0度
//                 positions[LEFT_FRONT_BACK_ID] = krs.degPos(params.maxAngleDeg);   // ID:5 は最大角度
//                 positions[6] = krs.degPos(0);           // ID:6 は 0度
//                 break;
                
//             case WingUpMode::BOTH:
//                 // 両方のヒレを上げる
//                 positions[RIGHT_UP_DOWN_ID] = krs.degPos(0);           // ID:1 は 0度
//                 positions[RIGHT_FRONT_BACK_ID] = krs.degPos(params.maxAngleDeg);  // ID:2 は最大角度
//                 positions[3] = krs.degPos(0);           // ID:3 は 0度
                
//                 positions[LEFT_UP_DOWN_ID] = krs.degPos(0);           // ID:4 は 0度
//                 positions[LEFT_FRONT_BACK_ID] = krs.degPos(params.maxAngleDeg);   // ID:5 は最大角度
//                 positions[6] = krs.degPos(0);           // ID:6 は 0度
//                 break;
//         }
        
//         // 位置と速度を設定
//         sendVec2ServoPos(positions, speeds);
        
//     } catch (...) {
//         errorStatus.servoError = true;
//         errorStatus.errorMessage = "Servo control error in RaiseMode";
//     }
// }
void handleEmergencySurface() {
    static unsigned long emergencyStartTime = 0;
    static bool emergencyInitialized = false;
    const unsigned long EMERGENCY_STAY_DURATION = 5000;  // 3秒間
    static CrushMode previousEmergencyMode = CrushMode::SERVO_OFF;
    
    // 緊急浮上モードに入った時の初期化
    if (previousEmergencyMode != CrushMode::EMERGENCY_SURFACE) {
        emergencyStartTime = millis();
        emergencyInitialized = true;
        previousEmergencyMode = CrushMode::EMERGENCY_SURFACE;
        Serial.println("Emergency Surface Mode Started");
    }
    
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - emergencyStartTime;
    
    if (elapsedTime <= EMERGENCY_STAY_DURATION) {
        // フェーズ1: 3秒間STAYモードで浮上動作
        SwimParameters stayParams;
        stayParams.periodSec = 1.0;     // 1秒周期
        stayParams.maxAngleDeg = 20.0;  // 大きめの角度で浮上
        stayParams.wingDeg = 0.0;       // 真上に動かす
        handleStayMode(stayParams);
        Serial.println("Emergency Phase 1: STAY mode");
    } else if (elapsedTime <= EMERGENCY_STAY_DURATION + 1000) {  // +1秒
        // フェーズ2: INIT_POSEに遷移
        handleInitMode();
        Serial.println("Emergency Phase 2: INIT_POSE");
    } else {
        // フェーズ3: SERVO_OFF
        setServoOff();
        Serial.println("Emergency Phase 3: SERVO_OFF");
        
        // 緊急モードの完了後、モードを変更
        currentMode = CrushMode::SERVO_OFF;
        emergencyInitialized = false;
    }
}

};

// class CrushMouth : public CrushMain {
// private:
//     const int MOUTH_SERVO_ID = 0;
//     const double MOUTH_OPEN_ANGLE = 30.0;
//     const double MOUTH_CLOSE_ANGLE = 0.0;
//     unsigned long lastMouthUpdate = 0;
//     const unsigned long MOUTH_UPDATE_INTERVAL = 100;

// protected:
//     void updateMotion() override {
//         unsigned long currentTime = millis();
//         if (currentTime - lastMouthUpdate >= MOUTH_UPDATE_INTERVAL) {
//             bool shouldOpen = messageProcessor.getMouthOpen();
//             double targetAngle = shouldOpen ? MOUTH_OPEN_ANGLE : MOUTH_CLOSE_ANGLE;
//             krs.setPos(MOUTH_SERVO_ID, krs.degPos(targetAngle));
//             lastMouthUpdate = currentTime;
//         }
//     }
// };

// グローバルインスタンス
CrushBody body;
//CrushMouth mouth;
//WiFiClient CrushMain::currentClient;

void setup() {
    CrushMain::initializeSystem();
}

void loop() {
    body.loop();
    //mouth.loop(); //とりあえずコメントアウト
}