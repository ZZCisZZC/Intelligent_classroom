#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ===== 串口配置 =====
#define RX2_PIN 16    // UART2 接收脚：ESP32 GPIO16（可根据实际硬件调整）
#define TX2_PIN 17    // UART2 发送脚：ESP32 GPIO17（可根据实际硬件调整）
#define BAUD0   115200
#define BAUD2   115200
HardwareSerial Serial2Port(2);  // 使用 UART2

/* ===== 全局设置 ===== */
#define MQTT_SERVER    "bemfa.com"
#define MQTT_PORT      9501
#define BEMFA_APIKEY   "f7f3759ee3cc47068f3f28196cc83ef2"       // = clientID
#define MQTT_PUB_TOPIC "dataUpdate"      // 发布主题
#define MQTT_SUB_TOPIC "setControl"      // 订阅主题

WiFiClient     wifiClient;
PubSubClient   mqttClient(wifiClient);

String wifiSSID;
String wifiPass;

/* ---------- 函数声明 ---------- */
void askWiFiCredentials();
void connectWiFi();
void connectMQTT();
void publishJSON(const String& payload);
void mqttCallback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(BAUD0);
  Serial.setTimeout(20000);    // 最长等 20 秒输入
  delay(200);

  // UART2 用于 UPDATE/SET 通信
  Serial2Port.begin(BAUD2, SERIAL_8N1, RX2_PIN, TX2_PIN);

  askWiFiCredentials();        // 交互式获取 SSID/密码
  connectWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
}

void loop() {
  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();

  // 持续监听串口命令
  static String cmdBuf;
  while (Serial2Port.available()) {
    char c = Serial2Port.read();
    if (c == '\n') {
      Serial.println("Getting data\n");
      cmdBuf.trim();
      if (cmdBuf == "UPDATE") {
        // 读取完整 JSON
        Serial.println("reading JSON:\n");
        String jsonPayload = readFullJSON();  
        Serial.println("read finished\n");
        if (jsonPayload.length()) {
          publishJSON(jsonPayload);
          Serial.println(F("[INFO] JSON Already Published:"));
          Serial.println(jsonPayload);
        } else {
          Serial.println(F("[WARN] NOT JSON"));
        }
      }
      cmdBuf = "";
    } else {
      cmdBuf += c;
    }
  }
}

/* ===== 交互式获取 WiFi SSID/密码 ===== */
void askWiFiCredentials() {
  Serial.println(F("WiFi SSID:"));
  wifiSSID = Serial.readStringUntil('\n');
  wifiSSID.trim();

  Serial.println(F("WiFi PSAAWORD"));
  wifiPass = Serial.readStringUntil('\n');
  wifiPass.trim();

  Serial.print(F("connect to WiFi:"));
  Serial.println(wifiSSID);
}

/* ===== 连接 WiFi ===== */
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());
  Serial.print(F("connecting WiFi"));
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println();
  Serial.print(F("WiFi connected ,ip"));
  Serial.println(WiFi.localIP());
}

/* ===== 连接并订阅 MQTT ===== */
void connectMQTT() {
  Serial.print(F("connect to MQTT... "));
  while (!mqttClient.connected()) {
    // clientID 用 apikey，用户名/密码留空
    if (mqttClient.connect(BEMFA_APIKEY, nullptr, nullptr)) {
      Serial.println(F("MQTT success"));
      mqttClient.subscribe(MQTT_SUB_TOPIC);
      Serial.print(F("subscribe topic:"));
      Serial.println(MQTT_SUB_TOPIC);
    } else {
      Serial.print(F("MQTT failed rc="));
      Serial.print(mqttClient.state());
      Serial.println(F(",2s retry"));
      delay(2000);
    }
  }
}

/* ===== MQTT 发布 JSON ===== */
void publishJSON(const String& payload) {
  if (!mqttClient.connected()) connectMQTT();
  bool ok = mqttClient.publish(MQTT_PUB_TOPIC, payload.c_str(), false);
  if (!ok) Serial2Port.println(F("SENDOK"));
}

/* ===== MQTT 回调，将收到的消息转发到串口 ===== */
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // 只处理 setControl 主题
  if (String(topic) == MQTT_SUB_TOPIC) {
    Serial2Port.print(F("SET\n"));
    String msg;
    for (unsigned int i = 0; i < length; i++) {
      msg += (char)payload[i];
    }
    Serial2Port.println(msg);
    // 再回显到 UART0 调试
    Serial.print(F("[MQTT To UART2] SET\n"));
    Serial.println(msg);
  }
}

/* ===== 辅助：读取完整多行 JSON ===== */
//String readFullJSON() {
//  String result;
//  int braceCount = 0;
//  bool started = false;
//  unsigned long start = millis();
//
//  while (millis() - start < 3000) {
//    while (Serial.available()) {
//      char c = Serial.read();
//      result += c;
//      if (c == '{') {
//        braceCount++;
//        started = true;
//      } else if (c == '}') {
//        braceCount--;
//      }
//      // 括号配对完成，退出
//      if (started && braceCount == 0) {
//        result.trim();
//        return result;
//      }
//    }
//    delay(50);
//  }
//  return "";  // 超时或未完整读取
//}
String readFullJSON() {
  String result;
  int braceCount = 0;
  bool started = false;
  unsigned long start = millis();

  while (millis() - start < 3000) {
    while (Serial2Port.available()) {
      char c = Serial2Port.read();
      result += c;
      Serial.write(c);   // 回显 JSON 到 UART0
      if (c == '{') {
        braceCount++;
        started = true;
      } else if (c == '}') {
        braceCount--;
      }
      // 大括号配对完成
      if (started && braceCount == 0) {
        // **重点：吃掉紧随其后的换行符 \r 或 \n，避免留在缓冲区**
        while (Serial2Port.available()) {
          char c2 = Serial2Port.peek();
          if (c2 == '\r' || c2 == '\n') {
            Serial2Port.read();
          } else break;
        }
        result.trim();
        return result;
      }
    }
    delay(10);
  }
  return "";  // 超时
}
