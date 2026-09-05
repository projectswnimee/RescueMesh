#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Starting Bluetooth...");

  // Start Bluetooth Classic
  SerialBT.begin("ESP32_BT_MONITOR");

  Serial.println("Bluetooth ready");
  Serial.println("Waiting for messages...");
}

void loop() {
  // Phone → ESP32
  if (SerialBT.available()) {
    String msg = SerialBT.readStringUntil('\n');
    msg.trim();

    if (msg.length()) {
      Serial.println("FROM PHONE: " + msg);
    }
  }
}