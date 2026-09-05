/* ============================================================
   Code 1.2 : Point-to-Point Messaging (Rebroadcast Test)
   ============================================================ */

#include <Arduino.h>

#define LORA_TX   17
#define LORA_RX   16
#define LORA_MD0  25
#define LORA_MD1  26

HardwareSerial LoRaSerial(2);

String rxBuffer = "";   // Message buffer

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LORA_MD0, OUTPUT);
  pinMode(LORA_MD1, OUTPUT);

  // NORMAL MODE (MD0=0, MD1=0)
  digitalWrite(LORA_MD0, LOW);
  digitalWrite(LORA_MD1, LOW);

  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);

  Serial.println("=== Code 1.2 : Rebroadcast Test ===");
  Serial.println("Type text and press ENTER");
}

void loop() {
  // SEND (Manual from Serial)
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg += '\n';   // Ensure terminator
    LoRaSerial.write(msg.c_str(), msg.length());

    Serial.print("[Sent] ");
    Serial.print(msg);
  }

  // RECEIVE + REBROADCAST
  while (LoRaSerial.available()) {
    char c = LoRaSerial.read();

    if (c == '\n') {
      Serial.print("[Received] ");
      Serial.println(rxBuffer);

      // ---- REBROADCAST AFTER 5 SECONDS ----
      delay(5000);

      String rebroadcastMsg = rxBuffer + '\n';
      LoRaSerial.write(rebroadcastMsg.c_str(), rebroadcastMsg.length());

      Serial.print("[Rebroadcasted] ");
      Serial.print(rebroadcastMsg);

      rxBuffer = "";   // Clear buffer
    } else {
      rxBuffer += c;
    }
  }
}