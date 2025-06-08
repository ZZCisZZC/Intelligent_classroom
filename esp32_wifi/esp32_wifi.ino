#include <WiFi.h>
#include <PubSubClient.h>

// —— 请在这里填入你的网络和巴法云信息 —— 
const char* WIFI_SSID     = "123456";
const char* WIFI_PASS     = "15252723816";
const char* MQTT_SERVER   = "bemfa.com";
const uint16_t MQTT_PORT  = 9501;         // 非 TLS 端口
const char* BEMFA_APIKEY  = "f7f3759ee3cc47068f3f28196cc83ef2"; // ClientID
const char* MQTT_TOPIC    = "dataUdate"; // 要发布到的 topic

WiFiClient    netClient;
PubSubClient  mqttClient(netClient);

// 重新连接到 MQTT 服务器
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("[MQTT] 连接中… ");
    // 用户名和密码留空
    if (mqttClient.connect(BEMFA_APIKEY, nullptr, nullptr)) {
      Serial.println("成功");
    } else {
      Serial.print("失败，rc=");
      Serial.print(mqttClient.state());
      Serial.println("，5秒后重试");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // 1. 先连 Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("[WiFi] 连接到 ");
  Serial.println(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n[WiFi] 已连接，IP = " + WiFi.localIP().toString());

  // 2. 配置 MQTT
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  reconnectMQTT();
}

bool  awaitingJson = false;
String jsonBuffer;

void loop() {
  // 保持 MQTT 连接
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  // 串口命令处理
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.equalsIgnoreCase("UPDATE")) {
      // 收到 UPDATE，开始准备接收 JSON
      awaitingJson = true;
      jsonBuffer = "";
      Serial.println("[Serial] 收到 UPDATE，等待下一行 JSON …");
    }
    else if (awaitingJson) {
      // 读取到 JSON（假设在一行内完整发送）
      jsonBuffer = line;
      Serial.println("[Serial] 收到 JSON 数据：");
      Serial.println(jsonBuffer);

      // 发布到 MQTT
      bool ok = mqttClient.publish(MQTT_TOPIC, jsonBuffer.c_str());
      if (ok) {
        Serial.println("[MQTT] 发布成功");
      } else {
        Serial.println("[MQTT] 发布失败");
      }

      awaitingJson = false;
    }
  }
}
