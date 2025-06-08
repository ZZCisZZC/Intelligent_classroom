/*  =====  头文件  =====  */
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

/*  =====  Wi-Fi 配置  =====  */
#define WIFI_SSID      "123456"      // ← 改成你的热点
#define WIFI_PASS      "15252723816"

/*  =====  巴法云 MQTT 配置  =====  */
#define BEMFA_APIKEY   "f7f3759ee3cc47068f3f28196cc83ef2"  // ← 你的 apikey (= clientID)
#define MQTT_SERVER    "bemfa.com"
#define MQTT_PORT      9501          // 非 TLS
#define MQTT_TOPIC     "dataUdate"   // ← 你要上传到的主题

/*  =====  全局对象  =====  */
WiFiClient     wifiClient;
PubSubClient   mqttClient(wifiClient);

/* ---------- 函数声明 ---------- */
void connectWiFi();
void connectMQTT();
void publishJSON(const String& payload);

/* ---------- 初始化 ---------- */
void setup()
{
  Serial.begin(115200);
  delay(200);

  connectWiFi();     // 先联网
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
}

/* ---------- 主循环 ---------- */
void loop()
{
  /* 1) 维持 MQTT 连接 */
  if (!mqttClient.connected())
    connectMQTT();
  mqttClient.loop();

  /* 2) 监听串口命令 */
  static String serialBuf;
  while (Serial.available())
  {
    char c = Serial.read();
    if (c == '\n')               // 一行结束
    {
      serialBuf.trim();          // 去掉\r\n
      if (serialBuf == "UPDATE") // 下一行就是 JSON
      {
        /* 阻塞式读取下一整行 JSON（直到换行）*/
        String jsonLine = Serial.readStringUntil('\n');
        jsonLine.trim();
        if (jsonLine.startsWith("{") && jsonLine.endsWith("}"))
        {
          publishJSON(jsonLine);
          Serial.println(F("[INFO] JSON 发布完毕"));
        }
        else
        {
          Serial.println(F("[WARN] 未检测到合法 JSON"));
        }
      }
      serialBuf = "";            // 清空缓存
    }
    else
    {
      serialBuf += c;            // 累加字符
    }
  }
}

/* ---------- Wi-Fi 连接函数 ---------- */
void connectWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print(F("Connecting WiFi"));
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);
  }
  Serial.print(F("\nWiFi OK, IP = "));
  Serial.println(WiFi.localIP());
}

/* ---------- MQTT 连接函数 ---------- */
void connectMQTT()
{
  Serial.print(F("Connecting MQTT ... "));
  while (!mqttClient.connected())
  {
    /* clientID 直接用 apikey，用户名/密码留空 */
    if (mqttClient.connect(BEMFA_APIKEY, nullptr, nullptr))
    {
      Serial.println(F("OK"));
    }
    else
    {
      Serial.print(F("failed, rc="));
      Serial.print(mqttClient.state());
      Serial.println(F(" -> retry in 2s"));
      delay(2000);
    }
  }
}

/* ---------- MQTT 发布函数 ---------- */
void publishJSON(const String& payload)
{
  if (!mqttClient.connected())
    connectMQTT();

  bool ok = mqttClient.publish(MQTT_TOPIC, payload.c_str(), /*retain=*/false);
  if (!ok)
    Serial.println(F("[ERROR] MQTT publish failed"));
}
