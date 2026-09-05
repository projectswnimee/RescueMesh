/* ============================================================
   Code 1.0 : ESP32 GPIO & UART Sanity Test
   Board    : NodeMCU ESP32 CH340 (ESP-WROOM-32)
   Module   : AS32-TTL-100 LoRa
   Purpose  : Verify GPIO wiring, UART2, AUX behavior

   ============================================================
   ================= ESP32 ↔ AS32-TTL-100 PINOUT =================

   NOTE:
   - GPIO numbers are used in code
   - D-numbers are the SILK-SCREEN names printed on the ESP32 board
   - Use D-numbers while wiring physically

   UART USED : Serial2
---------------------------------------------------------------

   ESP32 PIN (GPIO)   BOARD NAME   →   AS32 PIN
---------------------------------------------------------------
   GPIO17             D17          →   RXD  (AS32)
   GPIO16             D16          →   TXD  (AS32)

   GPIO25             D25          →   MD0
   GPIO26             D26          →   MD1
   GPIO27             D27          →   AUX

   GND                GND          →   GND
   5V / 3V3           5V / 3V3     →   VCC (2.5–5.5V)

   ============================================================ */

/* ================= ESP32 GPIO DEFINITIONS ================= */
// UART2 pins
#define LORA_TX   17   // GPIO17 (D17) → AS32 RXD
#define LORA_RX   16   // GPIO16 (D16) → AS32 TXD

// Mode control pins
#define LORA_MD0  25   // GPIO25 (D25) → AS32 MD0
#define LORA_MD1  26   // GPIO26 (D26) → AS32 MD1

// Status pin
#define LORA_AUX  27   // GPIO27 (D27) → AS32 AUX

/* ================= LoRa SYSTEM STANDARD ================= */
#define LORA_UART_BAUD   9600    // Factory default baud rate
#define LORA_AIR_SPEED   2.4     // kbps (factory default)
#define LORA_FREQ        433     // MHz
#define LORA_CHAN        0x17    // 433 MHz channel
#define LORA_TX_POWER    0       // 20 dBm
#define LORA_WAKE_TIME   0       // 250 ms
/* ============================================================ */

void setup() {
  // USB Serial (PC ↔ ESP32)
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== Code 1.0 : ESP32 GPIO & UART Sanity Test ===");

  // Configure GPIO directions
  pinMode(LORA_MD0, OUTPUT);
  pinMode(LORA_MD1, OUTPUT);
  pinMode(LORA_AUX, INPUT);

  // Force AS32 into GENERAL MODE
  // MD0 = LOW, MD1 = LOW
  digitalWrite(LORA_MD0, LOW);
  digitalWrite(LORA_MD1, LOW);

  Serial.println("AS32 Mode Set : GENERAL MODE (MD0=0, MD1=0)");

  // Start UART2 for LoRa
  Serial2.begin(
    LORA_UART_BAUD,
    SERIAL_8N1,
    LORA_RX,   // GPIO16 (D16)
    LORA_TX    // GPIO17 (D17)
  );

  Serial.println("UART2 initialized @ 9600 baud");

  // Wait until AS32 is idle
  Serial.print("Waiting for AUX to go HIGH...");
  while (digitalRead(LORA_AUX) == LOW) {
    delay(10);
  }
  Serial.println(" READY");

  Serial.println("Type text in Serial Monitor to send via LoRa UART");
}

void loop() {
  /* -------- ESP32 → AS32 -------- */
  if (Serial.available()) {
    char c = Serial.read();
    Serial2.write(c);
    Serial.print("[TX] ");
    Serial.println(c);
  }

  /* -------- AS32 → ESP32 -------- */
  if (Serial2.available()) {
    char c = Serial2.read();
    Serial.print("[RX] ");
    Serial.println(c);
  }

  /* -------- AUX Status Monitor -------- */
  static unsigned long lastAUXCheck = 0;
  if (millis() - lastAUXCheck > 1000) {
    lastAUXCheck = millis();
    Serial.print("AUX Status : ");
    Serial.println(digitalRead(LORA_AUX) ? "HIGH (IDLE)" : "LOW (BUSY)");
  }
}
