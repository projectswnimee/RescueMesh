/* ============================================================
   Code 1.3.2B : Wake-on-Radio Transmitter
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

  // Wake-Up Mode (MD0=1, MD1=0)
  digitalWrite(LORA_MD0, HIGH);
  digitalWrite(LORA_MD1, LOW);

  delay(50);
  waitForAUX();

  LoRa.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);

  Serial.println("=== Wake-on-Radio Transmitter ===");
  Serial.println("Sending wake-up message every 5 seconds...");
}

void loop() {
  waitForAUX();
  LoRa.println("SOS_WAKE_TEST");

  Serial.println("📡 Wake-up packet sent");
  delay(5000);
}
