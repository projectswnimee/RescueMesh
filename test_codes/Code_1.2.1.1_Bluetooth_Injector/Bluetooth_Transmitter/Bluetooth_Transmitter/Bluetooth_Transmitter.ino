/* ============================================================
   Bluetooth → LoRa Broadcast (Source Node)
   Based on Code 1.2.1 (RAW broadcast)
   ============================================================ */

#include <Arduino.h>
#include "BluetoothSerial.h"

/* ---------- Bluetooth ---------- */
BluetoothSerial SerialBT;

/* ---------- LoRa Pins ---------- */
#define LORA_TX   17
#define LORA_RX   16
#define LORA_MD0  25
#define LORA_MD1  26

HardwareSerial LoRaSerial(2);

void setup() {
  Serial.begin(115200);
  delay(1000);

  /* Bluetooth */
  SerialBT.begin("ESP32_LORA_SOURCE");
  Serial.println("Bluetooth ready");

  /* LoRa Mode */
  pinMode(LORA_MD0, OUTPUT);
  pinMode(LORA_MD1, OUTPUT);
  digitalWrite(LORA_MD0, LOW);
  digitalWrite(LORA_MD1, LOW);

  /* LoRa UART */
  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);

  Serial.println("=== Bluetooth → LoRa Broadcast ===");
}

void loop() {
  /* --- Bluetooth input → LoRa broadcast --- */
  if (SerialBT.available()) {
    String msg = SerialBT.readStringUntil('\n');
    msg.trim();

    if (msg.length()) {
      LoRaSerial.write(msg.c_str(), msg.length());
      LoRaSerial.write('\n');

      Serial.print("[BT Broadcast] ");
      Serial.println(msg);
    }
  }
}