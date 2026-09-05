/* ============================================================
   Code 1.X.1 : Continuous LoRa Transmitter Test
   Board      : NodeMCU ESP32 CH340
   Module     : AS32-TTL-100
   Purpose    : Send fixed message every 10 seconds
                with '\n' terminator
   ============================================================ */

#include <Arduino.h>

/* ================= PIN DEFINITIONS ================= */
#define LORA_RX_PIN   16   // D17 → AS32 RXD
#define LORA_TX_PIN   17   // D16 → AS32 TXD
#define LORA_MD0_PIN  25   // D25 → MD0
#define LORA_MD1_PIN  26   // D26 → MD1
#define LORA_AUX_PIN  27   // D27 → AUX

HardwareSerial LoRaSerial(2);

/* ================= MESSAGE ================= */
String fixedMessage = "FLOOD_NODE_TEST_MESSAGE";

void setup() {
  Serial.begin(115200);

  /* General Mode */
  pinMode(LORA_MD0_PIN, OUTPUT);
  pinMode(LORA_MD1_PIN, OUTPUT);
  digitalWrite(LORA_MD0_PIN, LOW);
  digitalWrite(LORA_MD1_PIN, LOW);

  pinMode(LORA_AUX_PIN, INPUT);

  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);
}

void loop() {
  String msg = fixedMessage + "\n";

  LoRaSerial.write(msg.c_str(), msg.length());

  Serial.print("[Sent] ");
  Serial.print(msg);

  delay(10000);  // 10 seconds
}