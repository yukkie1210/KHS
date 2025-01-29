#include <WiFi.h>

// WiFi設定
const char* ssid = "Pixel_6259";       // WiFi SSID
const char* password = "yukkiebanana1210"; // WiFi パスワード

// サーバー設定
const int port = 46963;              // ポート番号
WiFiServer server(port);             // WiFiサーバーオブジェクト作成

void setup() {
  Serial.begin(115200);
  
  // WiFiに接続
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // サーバー開始
  server.begin();
  Serial.printf("TCP server started on port %d\n", port);
}

void loop() {
  // クライアントの接続を待機
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected.");
    while (client.connected()) {
      if (client.available()) {
        // クライアントからのデータを読み取る
        String receivedMessage = client.readStringUntil('\n');
        Serial.println("Received: " + receivedMessage);

        // クライアントに返信
        String responseMessage = "Hello from ESP32!";
        client.println(responseMessage);
        Serial.println("Sent: " + responseMessage);
      }
      delay(10); // 短い遅延を挿入
    }

    // クライアントが切断された場合
    Serial.println("Client disconnected.");
    client.stop();
  }
}