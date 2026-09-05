/* ============================================================
   Code 1.1 : AS32 Parameter Read Code (FIXED)
   Board    : ESP32
   Module  : AS32-TTL-100
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
void readAS32Parameters();

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== Code 1.1 : AS32 Parameter Read ===");

  pinMode(LORA_MD0, OUTPUT);
  pinMode(LORA_MD1, OUTPUT);
  pinMode(LORA_AUX, INPUT);

  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);

  enterSleepMode();
  readAS32Parameters();
}

void loop() {
  // One-time diagnostic code
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
  delay(2);  // Stabilization delay
}

/* ============================================================
   Read AS32 Parameters (C1 C1 C1)
   ============================================================ */
void readAS32Parameters() {
  Serial.println("[ACTION] Reading AS32 parameters...");

  uint8_t cmd[3] = {0xC1, 0xC1, 0xC1};

  LoRaSerial.write(cmd, 3);
  LoRaSerial.flush();

  // IMPORTANT: wait until AS32 finishes responding
  waitForAUXHigh();
  delay(5);

  Serial.print("[RESPONSE] Raw Bytes: ");

  while (LoRaSerial.available()) {
    uint8_t b = LoRaSerial.read();
    Serial.print("0x");
    if (b < 0x10) Serial.print("0");
    Serial.print(b, HEX);
    Serial.print(" ");
  }

  Serial.println("\n[INFO] Parameter read complete");
}