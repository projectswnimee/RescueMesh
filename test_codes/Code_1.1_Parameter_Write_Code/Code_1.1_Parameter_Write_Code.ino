/* ============================================================
   Code 1.1.1 : AS32 Parameter WRITE Code (50 mW)
   Board      : ESP32
   Module     : AS32-TTL-100
   ============================================================ */

#include <Arduino.h>

/* ---------- Pin Definitions (DO NOT CHANGE) ---------- */
#define LORA_TX   17    // ESP32 D17 → AS32 RXD
#define LORA_RX   16    // ESP32 D16 ← AS32 TXD

#define LORA_MD0  25    // ESP32 D25 → AS32 MD0
#define LORA_MD1  26    // ESP32 D26 → AS32 MD1
#define LORA_AUX  27    // ESP32 D27 ← AS32 AUX

/* ---------- UART2 for AS32 ---------- */
HardwareSerial LoRaSerial(2);

/* ---------- Function Prototypes ---------- */
void enterSleepMode();
void waitForAUXHigh();
void writeAS32Parameters();

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== Code 1.1.1 : AS32 Parameter WRITE (50 mW) ===");

  pinMode(LORA_MD0, OUTPUT);
  pinMode(LORA_MD1, OUTPUT);
  pinMode(LORA_AUX, INPUT);

  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);

  enterSleepMode();
  writeAS32Parameters();
}

void loop() {
  // One-time configuration code
}

/* ============================================================
   Put AS32 into Sleep Mode (MD0=1, MD1=1)
   ============================================================ */
void enterSleepMode() {
  Serial.println("[INFO] Entering AS32 SLEEP mode...");

  digitalWrite(LORA_MD0, HIGH);
  digitalWrite(LORA_MD1, HIGH);

  delay(5);
  waitForAUXHigh();

  Serial.println("[OK] AS32 is now in SLEEP mode");
}

/* ============================================================
   Wait until AUX goes HIGH (module idle)
   ============================================================ */
void waitForAUXHigh() {
  while (digitalRead(LORA_AUX) == LOW) {
    delay(1);
  }
  delay(2);
}

/* ============================================================
   Write Parameters Permanently (C0 Command)
   ============================================================ */
void writeAS32Parameters() {
  Serial.println("[ACTION] Writing AS32 parameters (50 mW)...");

  uint8_t cmd[6] = {
    0xC0,   // Save parameters permanently
    0x00,   // ADDH
    0x00,   // ADDL
    0x1A,   // SPEED
    0x17,   // CHAN
    0x58    // OPTION
  };

  LoRaSerial.write(cmd, 6);
  LoRaSerial.flush();

  // Wait until module finishes writing
  waitForAUXHigh();
  delay(10);

  Serial.print("[RESPONSE] ");

  while (LoRaSerial.available()) {
    char c = LoRaSerial.read();
    Serial.print(c);
  }

  Serial.println("\n[INFO] Parameter write complete");
}