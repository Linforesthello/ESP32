#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

/* ========= WiFi 配置 ========= */
const char* WIFI_SSID = "LinDesktop";
const char* WIFI_PASS = "linhlzyforest";

/* ========= MQTT 配置 ========= */
const char* MQTT_HOST = "broker.emqx.io";
const int   MQTT_PORT = 1883;
const char* SUB_TOPIC = "test/esp32s3/in";
const char* PUB_TOPIC = "test/esp32s3/out";

/* ========= 对象 ========= */
WiFiClient espClient;
PubSubClient mqttClient(espClient);

/* ========= 串口缓存 ========= */
String uart1_rx;
String uart2_rx;

/* ========= MQTT 回调 ========= */
void mqttCallback(char* topic, byte* payload, unsigned int length)
{
    String msg;
    for (unsigned int i = 0; i < length; i++) {
        msg += (char)payload[i];
    }

    Serial.print("[MQTT RX] ");
    Serial.println(msg);

    // 下发到 UART1（全双工）
    Serial1.println(msg);
}

/* ========= WiFi 连接 ========= */
void wifiConnect()
{
    Serial.print("Connecting WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

/* ========= MQTT 连接 ========= */
void mqttConnect()
{
    while (!mqttClient.connected()) {
        Serial.print("Connecting MQTT... ");
        if (mqttClient.connect("esp32s3_test_minimal")) {
            Serial.println("OK");
            mqttClient.subscribe(SUB_TOPIC);
            Serial.print("Subscribed: ");
            Serial.println(SUB_TOPIC);
        } else {
            Serial.print("FAILED, rc=");
            Serial.println(mqttClient.state());
            delay(2000);
        }
    }
}

/* ========= UART 处理 ========= */
void handleUart1()
{
    while (Serial1.available()) {
        char c = Serial1.read();
        if (c == '\n') {
            uart1_rx.trim();
            if (uart1_rx.length()) {
                Serial.print("[UART1 → USB CDC] ");
                Serial.println(uart1_rx);
                mqttClient.publish(PUB_TOPIC, uart1_rx.c_str());
            }
            uart1_rx = "";
        } else {
            uart1_rx += c;
        }
    }
}

void handleUart2()
{
    while (Serial2.available()) {
        char c = Serial2.read();
        if (c == '\n') {
            uart2_rx.trim();
            if (uart2_rx.length()) {
                Serial.print("[UART2 → USB CDC] ");
                Serial.println(uart2_rx);
                mqttClient.publish(PUB_TOPIC, uart2_rx.c_str());
            }
            uart2_rx = "";
        } else {
            uart2_rx += c;
        }
    }
}

/* ========= Arduino 生命周期 ========= */
void setup()
{
    // 1️⃣ USB CDC 初始化
    Serial.begin(115200);
    delay(2000); // 等待 USB CDC 枚举完成
    Serial.println("=== USB CDC OK ===");

    // 2️⃣ 串口初始化
    // Serial1.begin(115200); // UART1 全双工

    // 假设 UART1: RX = 21, TX = 20
    Serial1.begin(115200, SERIAL_8N1, 18, 17); // UART1 全双工

    Serial.println("Starting 2..."); // <-- 新增的调试行

    // 假设 UART2: RX = 19, TX = 18
    // Serial2.begin(115200, SERIAL_8N1, 19, 18); // UART2 单向

    // Serial2.begin(115200); // UART2 单向

    Serial.println("Starting WiFi connection..."); // <-- 新增的调试行

    // 3️⃣ WiFi
    wifiConnect();

    // 4️⃣ MQTT
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    mqttConnect();
}

void loop()
{
    // MQTT 保持连接
    if (!mqttClient.connected()) {
        mqttConnect();
    }
    mqttClient.loop();

    // UART 数据处理
    handleUart1();
    handleUart2();

    // USB CDC 心跳
    static uint32_t last = 0;
    if (millis() - last > 1000) {
        last = millis();
        Serial.println("alive");
    }
}

// #include <Arduino.h>
// #include <WiFi.h>
// #include <PubSubClient.h>
// void setup() {
//     Serial.begin(115200);
//     delay(2000);
//     Serial.println("Hello USB CDC");
// }
// void loop() {
//     Serial.println("alive");
//     delay(1000);
// }

// #include <Arduino.h>
// #include <WiFi.h>
// #include <PubSubClient.h>

// // ====== WiFi 配置 ======
// const char* WIFI_SSID = "LinDesktop";
// const char* WIFI_PASS = "linhlzyforest";

// // ====== MQTT 配置 ======
// const char* MQTT_HOST = "broker.emqx.io";
// const int   MQTT_PORT = 1883;
// const char* SUB_TOPIC = "test/esp32s3/in";
// const char* PUB_UART1_TOPIC = "uart1/state";
// const char* PUB_UART2_TOPIC = "uart2/raw";

// // ====== 对象 ======
// WiFiClient espClient;
// PubSubClient mqttClient(espClient);

// // ====== 串口缓存 ======
// String uart1_rx;
// String uart2_rx;

// // ====== MQTT 回调 ======
// void mqttCallback(char* topic, byte* payload, unsigned int length) {
//     String msg;
//     for (unsigned int i=0; i<length; i++) msg += (char)payload[i];

//     Serial.print("[MQTT RX] "); Serial.println(msg);

//     // 下发给 UART1
//     Serial1.println(msg);
// }

// // ====== WiFi 连接 ======
// void wifiConnect() {
//     Serial.print("Connecting WiFi");
//     WiFi.begin(WIFI_SSID, WIFI_PASS);
//     while (WiFi.status() != WL_CONNECTED) {
//         delay(500); Serial.print(".");
//     }
//     Serial.println();
//     Serial.println("WiFi connected");
//     Serial.print("IP: "); Serial.println(WiFi.localIP());
// }

// // ====== MQTT 连接 ======
// void mqttConnect() {
//     while (!mqttClient.connected()) {
//         Serial.print("Connecting MQTT... ");
//         if (mqttClient.connect("esp32s3_usbcdc_demo")) {
//             Serial.println("OK");
//             mqttClient.subscribe(SUB_TOPIC);
//             Serial.print("Subscribed: "); Serial.println(SUB_TOPIC);
//         } else {
//             Serial.print("FAILED, rc="); Serial.println(mqttClient.state());
//             delay(2000);
//         }
//     }
// }

// // ====== 串口接收处理 ======
// void handleUART1() {
//     while (Serial1.available()) {
//         char c = Serial1.read();
//         if (c == '\n') {
//             uart1_rx.trim();
//             if (uart1_rx.length()) {
//                 mqttClient.publish(PUB_UART1_TOPIC, uart1_rx.c_str());
//                 Serial.print("[UART1 → MQTT] "); Serial.println(uart1_rx);
//             }
//             uart1_rx = "";
//         } else uart1_rx += c;
//     }
// }

// void handleUART2() {
//     while (Serial2.available()) {
//         char c = Serial2.read();
//         if (c == '\n') {
//             uart2_rx.trim();
//             if (uart2_rx.length()) {
//                 mqttClient.publish(PUB_UART2_TOPIC, uart2_rx.c_str());
//                 Serial.print("[UART2 → MQTT] "); Serial.println(uart2_rx);
//             }
//             uart2_rx = "";
//         } else uart2_rx += c;
//     }
// }

// // ====== Setup ======
// void setup() {
//     Serial.begin(115200);   // USB CDC 调试
//     Serial1.begin(115200);  // TTL 串口1
//     Serial2.begin(115200);  // TTL 串口2

//     delay(100);
//     Serial.println("=== ESP32S3 USB CDC Demo ===");

//     wifiConnect();

//     mqttClient.setServer(MQTT_HOST, MQTT_PORT);
//     mqttClient.setCallback(mqttCallback);

//     mqttConnect();
// }

// // ====== Loop ======
// void loop() {
//     if (!mqttClient.connected()) mqttConnect();
//     mqttClient.loop();

//     handleUART1();  // 全双工串口
//     handleUART2();  // 单向串口

//     // USB CDC 心跳
//     static uint32_t last = 0;
//     if (millis() - last > 1000) {
//         last = millis();
//         Serial.println("alive");
//     }
// }


// #include <Arduino.h>
// #include <WiFi.h>
// #include <PubSubClient.h>

// /* ========= WiFi 配置 ========= */
// const char* WIFI_SSID = "LinDesktop";
// const char* WIFI_PASS = "linhlzyforest";

// /* ========= MQTT 配置 ========= */
// const char* MQTT_HOST = "broker.emqx.io";
// const int   MQTT_PORT = 1883;
// const char* SUB_TOPIC = "test/esp32s3/in";     // MQTT 下行 → UART1
// const char* PUB_UART1 = "test/esp32s3/out";    // UART1 上行 → MQTT
// const char* PUB_UART2 = "test/esp32s3/raw";    // UART2 上行 → MQTT

// /* ========= 串口配置 ========= */
// #define UART1_BAUD 115200
// #define UART2_BAUD 115200

// /* ========= 对象 ========= */
// WiFiClient espClient;
// PubSubClient mqttClient(espClient);

// /* ========= 串口缓存 ========= */
// String uart1Buffer;
// String uart2Buffer;

// /* ========= MQTT 回调 ========= */
// void mqttCallback(char* topic, byte* payload, unsigned int length) {
//     String msg;
//     for (unsigned int i = 0; i < length; i++) {
//         msg += (char)payload[i];
//     }

//     Serial.print("[MQTT RX] ");
//     Serial.println(msg);

//     // 下发到 UART1
//     Serial1.println(msg);
// }

// /* ========= WiFi 连接 ========= */
// void wifiConnect() {
//     Serial.print("Connecting WiFi");
//     WiFi.begin(WIFI_SSID, WIFI_PASS);
//     while (WiFi.status() != WL_CONNECTED) {
//         delay(500);
//         Serial.print(".");
//     }
//     Serial.println();
//     Serial.println("WiFi connected");
//     Serial.print("IP: ");
//     Serial.println(WiFi.localIP());
// }

// /* ========= MQTT 连接 ========= */
// void mqttConnect() {
//     while (!mqttClient.connected()) {
//         Serial.print("Connecting MQTT... ");
//         if (mqttClient.connect("esp32s3_dual_uart")) {
//             Serial.println("OK");
//             mqttClient.subscribe(SUB_TOPIC);
//             Serial.print("Subscribed: ");
//             Serial.println(SUB_TOPIC);
//         } else {
//             Serial.print("FAILED, rc=");
//             Serial.println(mqttClient.state());
//             delay(2000);
//         }
//     }
// }

// /* ========= 串口1处理 ========= */
// void handleUart1_RX() {
//     while (Serial1.available()) {
//         char c = Serial1.read();
//         if (c == '\n') {
//             uart1Buffer.trim();
//             if (uart1Buffer.length()) {
//                 mqttClient.publish(PUB_UART1, uart1Buffer.c_str());
//                 Serial.print("[UART1 → MQTT] ");
//                 Serial.println(uart1Buffer);
//             }
//             uart1Buffer = "";
//         } else {
//             uart1Buffer += c;
//         }
//     }
// }

// /* ========= 串口2处理 ========= */
// void handleUart2_RX() {
//     while (Serial2.available()) {
//         char c = Serial2.read();
//         if (c == '\n') {
//             uart2Buffer.trim();
//             if (uart2Buffer.length()) {
//                 mqttClient.publish(PUB_UART2, uart2Buffer.c_str());
//                 Serial.print("[UART2 → MQTT] ");
//                 Serial.println(uart2Buffer);
//             }
//             uart2Buffer = "";
//         } else {
//             uart2Buffer += c;
//         }
//     }
// }

// void setup() {
//     Serial.begin(115200);       // USB CDC
//     Serial1.begin(UART1_BAUD);  // 全双工
//     Serial2.begin(UART2_BAUD);  // 单向接收

//     delay(1000);
//     Serial.println("\nUSB CDC OK");

//     wifiConnect();

//     mqttClient.setServer(MQTT_HOST, MQTT_PORT);
//     mqttClient.setCallback(mqttCallback);
//     mqttConnect();
// }

// void loop() {
//     if (!mqttClient.connected()) mqttConnect();
//     mqttClient.loop();

//     handleUart1_RX();  // UART1 上行 → MQTT
//     handleUart2_RX();  // UART2 上行 → MQTT

//     // 心跳
//     static uint32_t last = 0;
//     if (millis() - last > 1000) {
//         last = millis();
//         Serial.println("alive");
//     }
// }


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 成功显示！

// #include <Arduino.h>
// #include <WiFi.h>
// #include <PubSubClient.h>

// /* ========= WiFi 配置 ========= */
// const char* WIFI_SSID = "LinDesktop";
// const char* WIFI_PASS = "linhlzyforest";

// /* ========= MQTT 配置 ========= */
// const char* MQTT_HOST = "broker.emqx.io";
// const int   MQTT_PORT = 1883;
// const char* SUB_TOPIC = "test/esp32s3/in";

// /* ========= 对象 ========= */
// WiFiClient espClient;
// PubSubClient mqttClient(espClient);

// /* ========= MQTT 回调 ========= */
// void mqttCallback(char* topic, byte* payload, unsigned int length)
// {
//     String msg;
//     for (unsigned int i = 0; i < length; i++) {
//         msg += (char)payload[i];
//     }

//     Serial.print("[MQTT RX] ");
//     Serial.println(msg);
// }

// /* ========= WiFi 连接 ========= */
// void wifiConnect()
// {
//     Serial.print("Connecting WiFi");
//     WiFi.begin(WIFI_SSID, WIFI_PASS);

//     while (WiFi.status() != WL_CONNECTED) {
//         delay(500);
//         Serial.print(".");
//     }

//     Serial.println();
//     Serial.println("WiFi connected");
//     Serial.print("IP: ");
//     Serial.println(WiFi.localIP());
// }

// /* ========= MQTT 连接 ========= */
// void mqttConnect()
// {
//     while (!mqttClient.connected()) {
//         Serial.print("Connecting MQTT... ");

//         if (mqttClient.connect("esp32s3_usbcdc_demo")) {
//             Serial.println("OK");
//             mqttClient.subscribe(SUB_TOPIC);
//             Serial.print("Subscribed: ");
//             Serial.println(SUB_TOPIC);
//         } else {
//             Serial.print("FAILED, rc=");
//             Serial.println(mqttClient.state());
//             delay(2000);
//         }
//     }
// }

// void setup()
// {
//     /* USB CDC 初始化 */
//     Serial.begin(115200);
//     delay(1000);

//     Serial.println();
//     Serial.println("USB CDC OK");

//     /* WiFi */
//     wifiConnect();

//     /* MQTT */
//     mqttClient.setServer(MQTT_HOST, MQTT_PORT);
//     mqttClient.setCallback(mqttCallback);

//     mqttConnect();
// }

// void loop()
// {
//     if (!mqttClient.connected()) {
//         mqttConnect();
//     }

//     mqttClient.loop();

//     /* 心跳，方便你确认程序没死 */
//     static uint32_t last = 0;
//     if (millis() - last > 1000) {
//         last = millis();
//         Serial.println("alive");
//     }
// }
// 成功显示！
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// -----------------------------------------------------------------
// 测试usb-otg,cdc功能，成立

// #include <Arduino.h>

// void setup() {
//   Serial.begin(115200);
//   while (!Serial) delay(10);
//   Serial.println("USB CDC OK");
// }

// void loop() {
//   Serial.println("alive");
//   delay(1000);
// }

// 测试usb-otg,cdc功能，成立
// -----------------------------------------------------------------

// #include <Arduino.h>
// #include <WiFi.h>
// #include <PubSubClient.h>

// // ================= WiFi 配置 =================
// const char* WIFI_SSID = "LinDesktop";
// const char* WIFI_PWD  = "linhlzyforest";

// // ================= MQTT 配置 =================
// const char* MQTT_BROKER = "broker.emqx.io";
// const int   MQTT_PORT   = 1883;

// const char* MQTT_PUB_TOPIC = "motor/1/speed";
// const char* MQTT_SUB_TOPIC = "motor/1/control";

// const char* MQTT_UARTA_RX_TOPIC = "motor/1/state";
// const char* MQTT_UARTB_RX_TOPIC = "sensor/1/raw";

// const char* MQTT_CLIENT_ID = "esp32s3-client-001";

// // ================= UART 引脚定义（关键） =================
// #define UARTA_RX 16
// #define UARTA_TX 17

// #define UARTB_RX 18
// #define UARTB_TX 19   // 实际不用，只是占位

// // ================= 对象 =================
// WiFiClient espClient;
// PubSubClient mqttClient(espClient);

// // ================= 串口缓存 =================
// String uartA_rx;
// String uartB_rx;

// // ================= MQTT 接收回调 =================
// void onMqttMessage(char* topic, byte* payload, unsigned int length)
// {
//     String msg;
//     for (unsigned int i = 0; i < length; i++) {
//         msg += (char)payload[i];
//     }

//     Serial.println("========== MQTT 收到消息 ==========");
//     Serial.print("主题: ");
//     Serial.println(topic);
//     Serial.print("内容: ");
//     Serial.println(msg);

//     // MQTT → UART-A
//     Serial1.println(msg);
//     Serial.println("[MQTT → UART-A] 已转发");
// }

// // ================= WiFi 连接 =================
// void connectWiFi()
// {
//     Serial.print("连接 WiFi ");
//     WiFi.mode(WIFI_STA);
//     WiFi.begin(WIFI_SSID, WIFI_PWD);

//     while (WiFi.status() != WL_CONNECTED) {
//         delay(500);
//         Serial.print(".");
//     }

//     Serial.println("\nWiFi 已连接");
//     Serial.print("IP: ");
//     Serial.println(WiFi.localIP());
// }

// // ================= MQTT 重连 =================
// void reconnectMQTT()
// {
//     while (!mqttClient.connected()) {
//         Serial.print("连接 MQTT ... ");
//         if (mqttClient.connect(MQTT_CLIENT_ID)) {
//             Serial.println("成功");
//             mqttClient.subscribe(MQTT_SUB_TOPIC);
//             Serial.print("已订阅: ");
//             Serial.println(MQTT_SUB_TOPIC);
//         } else {
//             Serial.print("失败 rc=");
//             Serial.println(mqttClient.state());
//             delay(2000);
//         }
//     }
// }

// // ================= UART-A RX =================
// void handleUartA_RX()
// {
//     while (Serial1.available()) {
//         char c = Serial1.read();
//         if (c == '\n') {
//             uartA_rx.trim();
//             if (uartA_rx.length()) {
//                 mqttClient.publish(MQTT_UARTA_RX_TOPIC, uartA_rx.c_str());
//                 Serial.print("[UART-A → MQTT] ");
//                 Serial.println(uartA_rx);
//             }
//             uartA_rx = "";
//         } else {
//             uartA_rx += c;
//         }
//     }
// }

// // ================= UART-B RX =================
// void handleUartB_RX()
// {
//     while (Serial2.available()) {
//         char c = Serial2.read();
//         if (c == '\n') {
//             uartB_rx.trim();
//             if (uartB_rx.length()) {
//                 mqttClient.publish(MQTT_UARTB_RX_TOPIC, uartB_rx.c_str());
//                 Serial.print("[UART-B → MQTT] ");
//                 Serial.println(uartB_rx);
//             }
//             uartB_rx = "";
//         } else {
//             uartB_rx += c;
//         }
//     }
// }

// // ================= Arduino 生命周期 =================
// void setup()
// {
//     Serial.begin(115200);
//     delay(200);
//     Serial.println("\n=== ESP32 setup start ===");

//     Serial1.begin(115200, SERIAL_8N1, UARTA_RX, UARTA_TX);
//     Serial2.begin(115200, SERIAL_8N1, UARTB_RX, UARTB_TX);

//     Serial.println("UART 初始化完成");

//     connectWiFi();

//     mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
//     mqttClient.setCallback(onMqttMessage);
// }

// void loop()
// {
//     if (!mqttClient.connected()) {
//         reconnectMQTT();
//     }
//     mqttClient.loop();

//     handleUartA_RX();
//     handleUartB_RX();

//     // loop 心跳
//     static uint32_t t = 0;
//     if (millis() - t > 2000) {
//         t = millis();
//         Serial.println("[loop alive]");
//     }
// }



// #include <Arduino.h>
// #include <WiFi.h>
// #include <PubSubClient.h>

// // ================= WiFi 配置 =================
// const char* WIFI_SSID = "LinDesktop";
// const char* WIFI_PWD  = "linhlzyforest";

// // ================= MQTT 配置 =================
// const char* MQTT_BROKER = "broker.emqx.io";
// const int   MQTT_PORT   = 1883;

// const char* MQTT_PUB_TOPIC = "motor/1/speed";
// const char* MQTT_SUB_TOPIC = "motor/1/control";

// // 新增：串口转发用 Topic
// const char* MQTT_UARTA_RX_TOPIC = "motor/1/state";   // 串口A → MQTT
// const char* MQTT_UARTB_RX_TOPIC = "sensor/1/raw";    // 串口B → MQTT

// const char* MQTT_CLIENT_ID = "esp32s3-client-001";

// // ================= 对象 =================
// WiFiClient espClient;
// PubSubClient mqttClient(espClient);

// // ================= 串口缓存 =================
// String uartA_rx;
// String uartB_rx;

// // ================= MQTT 接收回调 =================
// void onMqttMessage(char* topic, byte* payload, unsigned int length)
// {
//     String msg;
//     for (unsigned int i = 0; i < length; i++) {
//         msg += (char)payload[i];
//     }

//     Serial.println("========== MQTT 收到消息 ==========");
//     Serial.print("主题: ");
//     Serial.println(topic);
//     Serial.print("内容: ");
//     Serial.println(msg);

//     // ===== 关键修改点 ① =====
//     // MQTT → 串口A（全双工，下发给 STM32）
//     Serial1.println(msg);

//     // 你原有的示例逻辑（可保留/可删）
//     if (msg == "start") {
//         Serial.println(">>> 电机启动");
//     } 
//     else if (msg == "stop") {
//         Serial.println(">>> 电机停止");
//     } 
//     else if (msg.startsWith("speed=")) {
//         int speed = msg.substring(6).toInt();
//         Serial.print(">>> 设置速度 = ");
//         Serial.println(speed);
//     }
// }

// // ================= WiFi 连接 =================
// void connectWiFi()
// {
//     Serial.print("连接 WiFi");
//     WiFi.mode(WIFI_STA);
//     WiFi.begin(WIFI_SSID, WIFI_PWD);

//     while (WiFi.status() != WL_CONNECTED) {
//         delay(500);
//         Serial.print(".");
//     }

//     Serial.println("\nWiFi 已连接");
//     Serial.print("IP: ");
//     Serial.println(WiFi.localIP());
// }

// // ================= MQTT 重连 =================
// void reconnectMQTT()
// {
//     while (!mqttClient.connected()) {
//         Serial.print("连接 MQTT ... ");
//         if (mqttClient.connect(MQTT_CLIENT_ID)) {
//             Serial.println("成功");

//             mqttClient.subscribe(MQTT_SUB_TOPIC);
//             Serial.print("已订阅: ");
//             Serial.println(MQTT_SUB_TOPIC);
//         } else {
//             Serial.print("失败, rc=");
//             Serial.print(mqttClient.state());
//             Serial.println("，2 秒后重试");
//             delay(2000);
//         }
//     }
// }

// // ================= 串口A：全双工（STM32） =================
// void handleUartA_RX()
// {
//     while (Serial1.available()) {
//         char c = Serial1.read();
//         if (c == '\n') {
//             uartA_rx.trim();
//             if (uartA_rx.length()) {
//                 mqttClient.publish(MQTT_UARTA_RX_TOPIC, uartA_rx.c_str());
//                 Serial.print("[UART-A → MQTT] ");
//                 Serial.println(uartA_rx);
//             }
//             uartA_rx = "";
//         } else {
//             uartA_rx += c;
//         }
//     }
// }

// // ================= 串口B：仅接收 =================
// void handleUartB_RX()
// {
//     while (Serial2.available()) {
//         char c = Serial2.read();
//         if (c == '\n') {
//             uartB_rx.trim();
//             if (uartB_rx.length()) {
//                 mqttClient.publish(MQTT_UARTB_RX_TOPIC, uartB_rx.c_str());
//                 Serial.print("[UART-B → MQTT] ");
//                 Serial.println(uartB_rx);
//             }
//             uartB_rx = "";
//         } else {
//             uartB_rx += c;
//         }
//     }
// }

// // ================= Arduino 生命周期 =================
// void setup()
// {

  

//     Serial.begin(115200);       // USB 调试
//     Serial1.begin(115200);      // 串口A：全双工
//     Serial2.begin(115200);      // 串口B：单向接收

//     delay(100);

//     connectWiFi();

//     mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
//     mqttClient.setCallback(onMqttMessage);
// }

// void loop()
// {
//     if (!mqttClient.connected()) {
//         reconnectMQTT();
//     }
//     mqttClient.loop();

//     // ===== 关键修改点 ② =====
//     handleUartA_RX();   // 全双工串口
//     handleUartB_RX();   // 单向串口

//     // ===== 你原有的周期发布 =====
//     static unsigned long lastPub = 0;
//     if (millis() - lastPub > 1000) {
//         lastPub = millis();
//         String msg = "motor,1,12," + String(millis());
//         mqttClient.publish(MQTT_PUB_TOPIC, msg.c_str());
//         Serial.print("发布: ");
//         Serial.println(msg);
//     }
// }


// --------------------------------------------

// #include <Arduino.h>
// #include <WiFi.h>              // ✅ ESP32 用这个
// #include <PubSubClient.h>

// // ================= WiFi 配置 =================
// const char* WIFI_SSID = "LinDesktop";
// const char* WIFI_PWD  = "linhlzyforest";

// // ================= MQTT 配置 =================
// const char* MQTT_BROKER = "broker.emqx.io";
// const int   MQTT_PORT   = 1883;

// const char* MQTT_PUB_TOPIC = "motor/1/speed";
// const char* MQTT_SUB_TOPIC = "motor/1/control";
// const char* MQTT_CLIENT_ID = "esp32s3-client-001";

// // ================= 对象 =================
// WiFiClient espClient;
// PubSubClient mqttClient(espClient);

// // ================= MQTT 接收回调 =================
// void onMqttMessage(char* topic, byte* payload, unsigned int length)
// {
//     String msg;
//     for (unsigned int i = 0; i < length; i++) {
//         msg += (char)payload[i];
//     }

//     Serial.println("========== MQTT 收到消息 ==========");
//     Serial.print("主题: ");
//     Serial.println(topic);
//     Serial.print("内容: ");
//     Serial.println(msg);

//     // 示例：字符串控制逻辑
//     if (msg == "start") {
//         Serial.println(">>> 电机启动");
//     } 
//     else if (msg == "stop") {
//         Serial.println(">>> 电机停止");
//     } 
//     else if (msg.startsWith("speed=")) {
//         int speed = msg.substring(6).toInt();
//         Serial.print(">>> 设置速度 = ");
//         Serial.println(speed);
//     }
// }

// // ================= WiFi 连接 =================
// void connectWiFi()
// {
//     Serial.print("连接 WiFi");
//     WiFi.mode(WIFI_STA);
//     WiFi.begin(WIFI_SSID, WIFI_PWD);

//     while (WiFi.status() != WL_CONNECTED) {
//         delay(500);
//         Serial.print(".");
//     }

//     Serial.println("\nWiFi 已连接");
//     Serial.print("IP: ");
//     Serial.println(WiFi.localIP());
// }

// // ================= MQTT 重连 =================
// void reconnectMQTT()
// {
//     while (!mqttClient.connected()) {
//         Serial.print("连接 MQTT ... ");
//         if (mqttClient.connect(MQTT_CLIENT_ID)) {
//             Serial.println("成功");

//             mqttClient.subscribe(MQTT_SUB_TOPIC);
//             Serial.print("已订阅: ");
//             Serial.println(MQTT_SUB_TOPIC);
//         } else {
//             Serial.print("失败, rc=");
//             Serial.print(mqttClient.state());
//             Serial.println("，2 秒后重试");
//             delay(2000);
//         }
//     }
// }

// // ================= Arduino 生命周期 =================
// void setup()
// {
//     Serial.begin(115200);
//     delay(10);

//     connectWiFi();

//     mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
//     mqttClient.setCallback(onMqttMessage);
// }

// void loop()
// {
//     if (!mqttClient.connected()) {
//         reconnectMQTT();
//     }
//     mqttClient.loop();

//     // 示例：定期发布
//     static unsigned long lastPub = 0;
//     if (millis() - lastPub > 1000) {
//         lastPub = millis();
//         String msg = "motor,1,12," + String(millis());
//         mqttClient.publish(MQTT_PUB_TOPIC, msg.c_str());
//         Serial.print("发布: ");
//         Serial.println(msg);
//     }
// }

// 

// --------------------------------------------------


// #include <Arduino.h>
// #include <WiFi.h>

// const char* ssid = "LinDesktop";        // Wi-Fi 名称
// const char* password = "linhlzyforest"; // Wi-Fi 密码

// void setup() {
//   // 初始化串口
//   Serial.begin(115200);
//   delay(1000);

//   // 连接 Wi-Fi
//   WiFi.begin(ssid, password);

//   // 等待连接
//   Serial.print("Connecting to Wi-Fi");
//   while (WiFi.status() != WL_CONNECTED) {
//     Serial.print(".");
//     delay(1000);
//   }

//   // 打印 IP 地址
//   Serial.println("");
//   Serial.println("Connected to Wi-Fi");
//   Serial.print("IP Address: ");
//   Serial.println(WiFi.localIP());
// }

// void loop() {
//   // 这里可以添加更多功能
//   delay(1000);
// }

