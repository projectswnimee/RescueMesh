#include <Arduino.h>

/* ---------- Pin Definitions (On-board D names) ---------- */
#define LORA_TX   16   // GPIO16 (D16) → AS32 RXD
#define LORA_RX   17   // GPIO17 (D17) → AS32 TXD
#define LORA_MD0  25   // GPIO25 (D25)
#define LORA_MD1  26   // GPIO26 (D26)
#define LORA_AUX  27   // GPIO27 (D27)

/* ---------- UART for AS32 ---------- */
HardwareSerial LoRaSerial(2);

/* ---------- Fixed Message ---------- */
#define FIXED_MSG_LEN 8        // "MESSAGE1"

/* ============================================================
   SETUP
   ============================================================ */
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LORA_MD0, OUTPUT);
  pinMode(LORA_MD1, OUTPUT);
  pinMode(LORA_AUX, INPUT);

  // General working mode
  digitalWrite(LORA_MD0, LOW);
  digitalWrite(LORA_MD1, LOW);
  delay(10);

  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);

  Serial.println("=== Code 1.3.1 : Fixed Length RX Test Started ===");
  Serial.println("Waiting for MESSAGE1...");
}

/* ============================================================
   LOOP
   ============================================================ */
void loop() {

  /* ---------- FIXED-LENGTH LoRa RX ---------- */
  if (LoRaSerial.available() >= FIXED_MSG_LEN) {

    Serial.println(">>> Packet detected (UART count OK)");

    char buffer[FIXED_MSG_LEN + 1];

    for (int i = 0; i < FIXED_MSG_LEN; i++) {
      buffer[i] = LoRaSerial.read();
    }
    buffer[FIXED_MSG_LEN] = '\0';   // Null terminate

    String packet = String(buffer);

    Serial.println("================================");
    Serial.print("RX FIXED: ");
    Serial.println(packet);
    Serial.println("--------------------------------");
  }

  /* ---------- Manual TX for testing ---------- */
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "send") {
      while (digitalRead(LORA_AUX) == LOW) { delay(2); }

      LoRaSerial.print("MESSAGE1");   // EXACT 8 bytes
      Serial.println("TX: MESSAGE1");
    }
  }
}
