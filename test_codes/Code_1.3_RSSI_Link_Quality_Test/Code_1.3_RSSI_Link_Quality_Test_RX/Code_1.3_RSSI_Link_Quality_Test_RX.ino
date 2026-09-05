/* ============================================================
   Code 1.3 : RSSI & Link Quality Test
   Role     : Receiver with RSSI display
   Board    : NodeMCU ESP32 CH340
   LoRa     : AS32-TTL-100
   ============================================================

   FINAL SHARED PINOUT (FROZEN)
   --------------------------------
   D16 (GPIO16) ← AS32 TXD
   D17 (GPIO17) → AS32 RXD
   D25 (GPIO25) → MD0
   D26 (GPIO26) → MD1
   D27 (GPIO27) → AUX
   3V3          → VCC
   GND          → GND
*/

#include <Arduino.h>
#include <HardwareSerial.h>

/* ---------- LoRa Pins ---------- */
#define LORA_RX   16   // D16 ← AS32 TXD
#define LORA_TX   17   // D17 → AS32 RXD
#define LORA_MD0  25   // D25
#define LORA_MD1  26   // D26
#define LORA_AUX  27   // D27

HardwareSerial LoRaSerial(2);

/* Buffer for received packet */
char rxBuffer[128];
uint8_t rxIndex = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  /* --- AS32 General Mode --- */
  pinMode(LORA_MD0, OUTPUT);
  pinMode(LORA_MD1, OUTPUT);
  digitalWrite(LORA_MD0, LOW);
  digitalWrite(LORA_MD1, LOW);

  pinMode(LORA_AUX, INPUT);

  /* --- UART --- */
  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);

  Serial.println("=== Code 1.3 : RSSI Receiver ===");
  Serial.println("Waiting for packets...");
}

void loop() {
  while (LoRaSerial.available()) {
    char c = LoRaSerial.read();

    /* Packet delimiter assumed as '\n' */
    if (c == '\n') {
      if (rxIndex >= 1) {
        /* Last byte = RSSI */
        uint8_t rssi = rxBuffer[rxIndex - 1];
        rxBuffer[rxIndex - 1] = '\0';  // remove RSSI from payload

        Serial.print("RX: ");
        Serial.print(rxBuffer);
        Serial.print(" | RSSI: ");
        Serial.println(rssi);
      }
      rxIndex = 0;  // reset buffer
    }
    else {
      if (rxIndex < sizeof(rxBuffer) - 1) {
        rxBuffer[rxIndex++] = c;
      }
    }
  }
}
