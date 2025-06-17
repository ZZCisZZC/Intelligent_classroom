#include <Arduino.h>
#include <Wire.h>
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;
const char* SLAVE_NAME = "QRS0327";
const unsigned long RECONNECT_INTERVAL = 5000;

const uint8_t  SHT31_ADDR = 0x44;   // 温湿度
const uint8_t  PIN_LDR     = 34;    // 光敏电阻
const uint8_t  PIN_PIR     = 33;    // 人体红外

// 读取温湿度传感器
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
  analogSetPinAttenuation(PIN_LDR, ADC_11db); 

  Serial.println("ESP32 Bluetooth Sensor");
  if (!SerialBT.begin("ESP32_Master", true)) {
    Serial.println("Bluetooth init fail"); while (1) delay(1000);
  }
  SerialBT.setPin("0327");

  Serial.print("Connect to "); Serial.println(SLAVE_NAME);
  SerialBT.connect(SLAVE_NAME);    // 连接蓝牙
}

void loop()
{
  // 指令解析
  static String rx;
  while (SerialBT.available()) {
    char c = SerialBT.read();
    if (c == '\n' || c == '\r') {
      rx.trim();
      if (rx.equalsIgnoreCase("GETSENSOR")) {

        // 温湿度
        float t  = -1;     
        float h  = -1;
        if (readSHT31(t, h) == false) {
          Serial.println("SHT31 read fail -> -1");
        }

        // 光敏电阻
        int rawLdr = analogRead(PIN_LDR);
        float illum = -1;
        if (rawLdr >= 0 && rawLdr <= 4095)       
          illum = 50/(rawLdr / 4096.0f);

        // 人体红外
        int presence = -1;
        int pirVal   = digitalRead(PIN_PIR);
        if (pirVal == LOW || pirVal == HIGH)         
          presence = pirVal;                    

        // 打包输出json
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

  // 断联重连
  static unsigned long lastTry = 0;
  if (!SerialBT.connected() && millis() - lastTry > RECONNECT_INTERVAL) {
    lastTry = millis();
    Serial.print("Reconnect ");
    SerialBT.connect(SLAVE_NAME) ? Serial.println("OK") : Serial.println("fail");
  }
}
