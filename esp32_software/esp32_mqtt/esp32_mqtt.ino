#include <Arduino.h>
#include <WiFi.h>
#include "PubSubClient.h"

// 串口配置
#define RX2_PIN 16    
#define TX2_PIN 17   
#define BAUD0   115200
#define BAUD2   115200
HardwareSerial Serial2Port(2);  // UART2

// 巴法云设置
#define MQTT_SERVER    "bemfa.com"
#define MQTT_PORT      9501
#define BEMFA_APIKEY   "f7f3759ee3cc47068f3f28196cc83ef2"       
#define MQTT_PUB_TOPIC "dataUpdate"      // 发布主题
#define MQTT_SUB_TOPIC "setControl"      // 订阅主题

WiFiClient     wifiClient;
PubSubClient   mqttClient(wifiClient);

String wifiSSID="123456";
String wifiPass="15252723816";

void askWiFiCredentials();
void connectWiFi();
void connectMQTT();
void publishJSON(const String& payload);
void mqttCallback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(BAUD0);
  //Serial.setTimeout(20000);  
  delay(200);

  // UART2用于与开发板通信
  Serial2Port.begin(BAUD2, SERIAL_8N1, RX2_PIN, TX2_PIN);

  //askWiFiCredentials();        // 从串口获取WiFi账号和密码
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
      // Serial.println("Getting data\n");
      cmdBuf.trim();
      if (cmdBuf == "UPDATE") {
        // Serial.println("reading JSON:\n");
        String jsonPayload = readFullJSON();  
        // Serial.println("read finished\n");
        if (jsonPayload.length()) {
          publishJSON(jsonPayload);
          // Serial.println(F("[INFO] JSON Already Published:"));
          // Serial.println(jsonPayload);
        } else {
          // Serial.println(F("[WARN] NOT JSON"));
        }
      }
      cmdBuf = "";
    } else {
      cmdBuf += c;
    }
  }
}

// 获取WiFi账号密码
void askWiFiCredentials() {
  // Serial.println(F("WiFi SSID:"));
  wifiSSID = Serial.readStringUntil('\n');
  wifiSSID.trim();

  // Serial.println(F("WiFi PSAAWORD"));
  wifiPass = Serial.readStringUntil('\n');
  wifiPass.trim();

  // Serial.print(F("connect to WiFi:"));
  // Serial.println(wifiSSID);
}

// 连接WiFi
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());
  // Serial.print(F("connecting WiFi"));
  while (WiFi.status() != WL_CONNECTED) {
    // Serial.print('.');
    delay(500);
  }
  // Serial.println();
  // Serial.print(F("WiFi connected ,ip"));
  // Serial.println(WiFi.localIP());
}

// 订阅并连接mqtt
void connectMQTT() {
  // Serial.print(F("connect to MQTT... "));
  while (!mqttClient.connected()) {
    if (mqttClient.connect(BEMFA_APIKEY, nullptr, nullptr)) {
      // Serial.println(F("MQTT success"));
      mqttClient.subscribe(MQTT_SUB_TOPIC);
      // Serial.print(F("subscribe topic:"));
      // Serial.println(MQTT_SUB_TOPIC);
    } else {
      // Serial.print(F("MQTT failed rc="));
      // Serial.print(mqttClient.state());
      // Serial.println(F(",2s retry"));
      delay(2000);
    }
  }
}

// mqtt发布json
void publishJSON(const String& payload) {
  if (!mqttClient.connected()) connectMQTT();
  bool ok = mqttClient.publish(MQTT_PUB_TOPIC, payload.c_str(), false);
  if (!ok) Serial2Port.println(F("SENDOK"));
}

// mqtt将收到的消息转发到串口，调试用
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  if (String(topic) == MQTT_SUB_TOPIC) {
    String msg;
    for (unsigned int i = 0; i < length; i++) {
      msg += (char)payload[i];
    }
    Serial2Port.print(F("SET"));
    Serial2Port.println(msg);
    // Serial.print(F("[MQTT To UART2] SET\n"));
    // Serial.println(msg);
  }
}

// 读取完整的json
String readFullJSON() {
  String result;
  int braceCount = 0;
  bool started = false;
  unsigned long start = millis();

  while (millis() - start < 3000) {
    while (Serial2Port.available()) {
      char c = Serial2Port.read();
      result += c;  
      if (c == '{') {
        braceCount++;
        started = true;
      } else if (c == '}') {
        braceCount--;
      }
      // 大括号配对完成
      if (started && braceCount == 0) {
        // 删除紧随其后的换行符\r\n，避免留在缓冲区
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
