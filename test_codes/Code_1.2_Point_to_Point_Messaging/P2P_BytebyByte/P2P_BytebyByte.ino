/* ============================================================
   Code 1.2.1 : Broadcast & Monitor Mode (Full Message Print)
   Purpose  : Send broadcast messages and display full incoming messages
   ============================================================ */

#include <Arduino.h>

#define LORA_TX   17
#define LORA_RX   16
#define LORA_MD0  25
#define LORA_MD1  26

HardwareSerial LoRaSerial(2);

String incomingBuffer = ""; // buffer for assembling full message

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LORA_MD0, OUTPUT);
  pinMode(LORA_MD1, OUTPUT);

  // NORMAL MODE
  digitalWrite(LORA_MD0, LOW);
  digitalWrite(LORA_MD1, LOW);

  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);

  Serial.println("=== Broadcast & Monitor Mode (Full Msg) ===");
  Serial.println("Type messages to broadcast, full incoming messages will appear below:");
}

void loop() {
  // --- Broadcast input from Serial Monitor ---
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n'); // capture full line
    LoRaSerial.write(msg.c_str(), msg.length()); // send raw bytes
    LoRaSerial.write('\n'); // append newline for delimiter
    Serial.print("[Broadcast] ");
    Serial.println(msg);
  }

  // --- Monitor incoming messages from LoRa ---
  while (LoRaSerial.available()) {
    char c = LoRaSerial.read();
    if (c == '\n') { // newline signals end of full message
      Serial.print("[Received] ");
      Serial.println(incomingBuffer);
      incomingBuffer = ""; // clear buffer for next message
    } else {
      incomingBuffer += c; // accumulate bytes into buffer
    }
  }
}