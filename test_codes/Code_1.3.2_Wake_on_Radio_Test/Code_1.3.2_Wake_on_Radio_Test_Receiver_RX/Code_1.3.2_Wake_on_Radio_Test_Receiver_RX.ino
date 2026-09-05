/* ============================================================
   Code 1.3.2A : Wake-on-Radio Receiver (Power-Saving Node)
   Board  : NodeMCU ESP32 CH340
   Module : AS32-TTL-100
   ============================================================ */

#define LORA_TX  17   // D16 → AS32 TXD
#define LORA_RX  16   // D17 → AS32 RXD
#define LORA_MD0 25   // D25 → AS32 MD0
#define LORA_MD1 26   // D26 → AS32 MD1
#define LORA_AUX 27   // D27 → AS32 AUX

HardwareSerial LoRa(2);

void waitForAUX() {
  while (digitalRead(LORA_AUX) == LOW);
  delay(2);
}

void setup() {
  Serial.begin(115200);

  pinMode(LORA_MD0, OUTPUT);
  pinMode(LORA_MD1, OUTPUT);
  pinMode(LORA_AUX, INPUT);

  // Power-Saving Mode (MD0=0, MD1=1)
  digitalWrite(LORA_MD0, LOW);
  digitalWrite(LORA_MD1, HIGH);

  delay(50);
  waitForAUX();

  LoRa.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);

  Serial.println("=== Wake-on-Radio Receiver ===");
  Serial.println("Mode: POWER SAVING (Sleep + Monitor)");
  Serial.println("Waiting for wake-up signal...");
}

void loop() {
  if (LoRa.available()) {
    String msg = LoRa.readString();

    Serial.println("⚡ WOKEN UP BY RADIO!");
    Serial.print("Received Message: ");
    Serial.println(msg);
    Serial.println("Returning to low-power monitoring...\n");
  }
}
