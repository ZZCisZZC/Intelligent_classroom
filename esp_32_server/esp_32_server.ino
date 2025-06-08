#include <Arduino.h>
#include <Wire.h>
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;
const char* SLAVE_NAME = "QRS0327";
const unsigned long RECONNECT_INTERVAL = 5000;

const uint8_t  SHT31_ADDR = 0x44;   // 温湿度
const uint8_t  PIN_LDR     = 34;    // 光敏电阻 → ADC1_CH6
const uint8_t  PIN_PIR     = 33;    // 人体红外

// -------- SHT31 驱动 --------
bool readSHT31(float &temperature, float &humidity)
{
  Wire.beginTransmission(SHT31_ADDR);
  Wire.write(0x2C); Wire.write(0x06);          // 单次测量，高重复度
  if (Wire.endTransmission() != 0) return false;
  delay(15);

  Wire.requestFrom(SHT31_ADDR, (uint8_t)6);
  if (Wire.available() != 6) return false;

  uint16_t t_raw  = (Wire.read() << 8) | Wire.read(); Wire.read();
  uint16_t rh_raw = (Wire.read() << 8) | Wire.read(); Wire.read();

  temperature = -45.0 + 175.0 * (t_raw  / 65535.0f);
  humidity    = 100.0 * (rh_raw / 65535.0f);
  return true;
}

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  pinMode(PIN_PIR, INPUT);
  analogSetPinAttenuation(PIN_LDR, ADC_11db);  // 量程 ~3.3 V

  Serial.println("=== ESP32 Bluetooth ===");
  if (!SerialBT.begin("ESP32_Master", true)) {
    Serial.println("BT init fail"); while (1) delay(1000);
  }
  SerialBT.setPin("0327");

  Serial.print("Connect to "); Serial.println(SLAVE_NAME);
  SerialBT.connect(SLAVE_NAME);    // 让它自己试；成功与否都在 loop 里再说
}

void loop()
{
  /* 1. 指令解析 ------------------------------------------------ */
  static String rx;
  while (SerialBT.available()) {
    char c = SerialBT.read();
    if (c == '\n' || c == '\r') {
      rx.trim();
      if (rx.equalsIgnoreCase("GETSENSOR")) {

        /* ---------- 1) 温湿度 ---------- */
        float t  = -1;       // 默认错误值
        float h  = -1;
        if (readSHT31(t, h) == false) {
          Serial.println("SHT31 read fail -> -1");
        }

        /* ---------- 2) 光敏电阻 ---------- */
        int rawLdr = analogRead(PIN_LDR);
        float illum = -1;
        if (rawLdr >= 0 && rawLdr <= 4095)            // 简单有效性判定
          illum = rawLdr / 4095.0f;

        /* ---------- 3) 人体红外 ---------- */
        int presence = -1;
        int pirVal   = digitalRead(PIN_PIR);
        if (pirVal == LOW || pirVal == HIGH)          // GPIO33 一般不会失败
          presence = pirVal;                          // 1 / 0

        /* ---------- 4) 打包输出 ---------- */
        char json[160];
        snprintf(json, sizeof(json),
          "{\"temp\":%.2f,\"humidity\":%.2f,"
          "\"lux\":%.3f,\"person\":%d}",
          t, h, illum, presence);

        SerialBT.println(json);
        Serial.print("Sent -> "); Serial.println(json);
      }
      rx = "";
    } else rx += c;
  }

  /* 2. 失连重连 ------------------------------------------------ */
  static unsigned long lastTry = 0;
  if (!SerialBT.connected() && millis() - lastTry > RECONNECT_INTERVAL) {
    lastTry = millis();
    Serial.print("Reconnecting… ");
    SerialBT.connect(SLAVE_NAME) ? Serial.println("OK") : Serial.println("fail");
  }
}
