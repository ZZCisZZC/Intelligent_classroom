#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

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
  Serial.begin(115200);
  Serial.setTimeout(20000);    // 最长等 20 秒输入
  delay(200);

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
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      cmdBuf.trim();
      if (cmdBuf == "UPDATE") {
        // 读取完整 JSON
        String jsonPayload = readFullJSON();  
        if (jsonPayload.length()) {
          publishJSON(jsonPayload);
          Serial.println(F("[INFO] JSON 已发布"));
        } else {
          Serial.println(F("[WARN] 未能读取到完整 JSON"));
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
  Serial.println(F("请输入 WiFi SSID："));
  wifiSSID = Serial.readStringUntil('\n');
  wifiSSID.trim();

  Serial.println(F("请输入 WiFi 密码："));
  wifiPass = Serial.readStringUntil('\n');
  wifiPass.trim();

  Serial.print(F("将尝试连接到 WiFi："));
  Serial.println(wifiSSID);
}

/* ===== 连接 WiFi ===== */
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());
  Serial.print(F("正在连接 WiFi"));
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println();
  Serial.print(F("WiFi 已连接，IP："));
  Serial.println(WiFi.localIP());
}

/* ===== 连接并订阅 MQTT ===== */
void connectMQTT() {
  Serial.print(F("连接 MQTT... "));
  while (!mqttClient.connected()) {
    // clientID 用 apikey，用户名/密码留空
    if (mqttClient.connect(BEMFA_APIKEY, nullptr, nullptr)) {
      Serial.println(F("成功"));
      mqttClient.subscribe(MQTT_SUB_TOPIC);
      Serial.print(F("已订阅主题："));
      Serial.println(MQTT_SUB_TOPIC);
    } else {
      Serial.print(F("失败, rc="));
      Serial.print(mqttClient.state());
      Serial.println(F("，2s后重试"));
      delay(2000);
    }
  }
}

/* ===== MQTT 发布 JSON ===== */
void publishJSON(const String& payload) {
  if (!mqttClient.connected()) connectMQTT();
  bool ok = mqttClient.publish(MQTT_PUB_TOPIC, payload.c_str(), false);
  if (!ok) Serial.println(F("[ERROR] 发布失败"));
}

/* ===== MQTT 回调，将收到的消息转发到串口 ===== */
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // 只处理 setControl 主题
  if (String(topic) == MQTT_SUB_TOPIC) {
    Serial.print(F("SET\n"));
    String msg;
    for (unsigned int i = 0; i < length; i++) {
      msg += (char)payload[i];
    }
    Serial.println(msg);
  }
}

/* ===== 辅助：读取完整多行 JSON ===== */
String readFullJSON() {
  String result;
  int braceCount = 0;
  bool started = false;
  unsigned long start = millis();

  while (millis() - start < 3000) {
    while (Serial.available()) {
      char c = Serial.read();
      result += c;
      if (c == '{') {
        braceCount++;
        started = true;
      } else if (c == '}') {
        braceCount--;
      }
      // 括号配对完成，退出
      if (started && braceCount == 0) {
        result.trim();
        return result;
      }
    }
    delay(10);
  }
  return "";  // 超时或未完整读取
}
