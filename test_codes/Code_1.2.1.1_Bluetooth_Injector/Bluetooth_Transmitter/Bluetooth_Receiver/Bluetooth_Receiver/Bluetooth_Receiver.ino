/* ============================================================
   LoRa Monitor Node (No Bluetooth)
   ============================================================ */

#include <Arduino.h>

#define LORA_TX   17
#define LORA_RX   16
#define LORA_MD0  25
#define LORA_MD1  26

HardwareSerial LoRaSerial(2);
String incomingBuffer = "";

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LORA_MD0, OUTPUT);
  pinMode(LORA_MD1, OUTPUT);
  digitalWrite(LORA_MD0, LOW);
  digitalWrite(LORA_MD1, LOW);

  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);

  Serial.println("=== LoRa Monitor Mode ===");
}

void loop() {
  while (LoRaSerial.available()) {
    char c = LoRaSerial.read();
    if (c == '\n') {
      Serial.print("[Received] ");
      Serial.println(incomingBuffer);
      incomingBuffer = "";
    } else {
      incomingBuffer += c;
    }
  }
}