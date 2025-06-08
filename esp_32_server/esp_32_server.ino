#include <Arduino.h>
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;
bool btConnected = false;
unsigned long lastReconnectAttempt = 0;
const char* SLAVE_NAME = "QRS0327";
const unsigned long RECONNECT_INTERVAL = 5000;  // 每5秒重连一次
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;

  
void setup() {
  // put your setup code here, to run once:
  // 连接标志和上次重连尝试时间
  Serial.begin(115200);
  Serial.println("=== ESP32 Bluetooth ===");
  if (!SerialBT.begin("ESP32_Master", true)) {
    Serial.println("Bluetooth initialization failed!");
    while (1) delay(1000);
  } else {
    Serial.println("Bluetooth initialized as Master.");
  }

  SerialBT.setPin("0327");

  Serial.print("Attempting to connect to slave: ");
  Serial.print(SLAVE_NAME);
  Serial.println(" ...");
  btConnected = SerialBT.connect(SLAVE_NAME);

  if (btConnected) {
    Serial.println("Connected!");
  } else {
    Serial.println("Initial connection failed.");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long currentMillis = millis();
  if (!SerialBT.connected()) {
    Serial.println("Bluetooth disconnected. Attempting to reconnect...");
    SerialBT.disconnect();
    SerialBT.begin("ESP32_Master", true);
    if (SerialBT.connect(SLAVE_NAME)) {
      Serial.println("Reconnected!");
    } else {
      Serial.println("Reconnection attempt failed.");
      delay(2000);  // 若未连接上，等待2秒再进行下一次检测
      return;
    }
  }

    // 蓝牙连接中，转发串口数据
  if (Serial.available()) {
    while (Serial.available()) {
      char c = Serial.read();
      SerialBT.write(c);
    }
  }

  if (SerialBT.available()) {
    while (SerialBT.available()) {
      char c = SerialBT.read();
      Serial.write(c);
    }
  }

}
