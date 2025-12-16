
#include <Arduino.h>
#include <WiFi.h>

const char* ssid = "LinDesktop";        // Wi-Fi 名称
const char* password = "linhlzyforest"; // Wi-Fi 密码

void setup() {
  // 初始化串口
  Serial.begin(115200);
  delay(1000);

  // 连接 Wi-Fi
  WiFi.begin(ssid, password);

  // 等待连接
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  // 打印 IP 地址
  Serial.println("");
  Serial.println("Connected to Wi-Fi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // 这里可以添加更多功能
  delay(1000);
}



/* #include <Arduino.h>

#define LED_PIN 48   // 假设是 GPIO48，先用这个，若没反应再试 38 或 8

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("ESP32-S3 Test: Blink + Serial OK!");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  Serial.println("LED ON");
  delay(1000);

  digitalWrite(LED_PIN, LOW);
  Serial.println("LED OFF");
  delay(1000);
} */
