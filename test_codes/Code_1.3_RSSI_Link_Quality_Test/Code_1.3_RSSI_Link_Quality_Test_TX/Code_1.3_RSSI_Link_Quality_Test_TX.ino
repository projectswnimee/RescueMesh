/* ============================================================
   Code 1.3 : RSSI & Link Quality Test
   Role     : Continuous Transmitter
   Board    : NodeMCU ESP32 CH340
   LoRa     : AS32-TTL-100 (Transparent Mode)
   ============================================================ */

/* ---------- Pin Mapping (FINAL & SHARED) ---------- */
#define LORA_RX   16   // D16 ← AS32 TXD
#define LORA_TX   17   // D17 → AS32 RXD
#define LORA_MD0  25   // D25
#define LORA_MD1  26   // D26
#define LORA_AUX  27   // D27

#include <Arduino.h>
#include <HardwareSerial.h>

HardwareSerial LoRaSerial(2);

uint32_t packetCounter = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  /* --- AS32 General Mode (Transparent) --- */
  pinMode(LORA_MD0, OUTPUT);
  pinMode(LORA_MD1, OUTPUT);
  digitalWrite(LORA_MD0, LOW);
  digitalWrite(LORA_MD1, LOW);

  pinMode(LORA_AUX, INPUT);

  /* --- UART to AS32 --- */
  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);

  Serial.println("=== Code 1.3 : RSSI Test | TRANSMITTER ===");
}

void loop() {
  packetCounter++;

  /* Simple continuous beacon */
  LoRaSerial.print("RSSI_TEST_PKT ");
  LoRaSerial.println(packetCounter);

  Serial.print("TX -> Packet ");
  Serial.println(packetCounter);

  delay(1000);   // 1 packet / second
}
