#include <Arduino.h>

/* ---------- Pin Definitions (On-board D names) ---------- */
#define LORA_TX   16   // GPIO16 (D16) → AS32 TXD
#define LORA_RX   17   // GPIO17 (D17) → AS32 RXD
#define LORA_MD0  25   // GPIO25 (D25)
#define LORA_MD1  26   // GPIO26 (D26)
#define LORA_AUX  27   // GPIO27 (D27)

/* ---------- UART ---------- */
HardwareSerial LoRaSerial(2);

/* ---------- Deduplication Settings ---------- */
#define MAX_NODES            10
#define DEDUP_WINDOW_MS     5000   // 5s test window

/* ---------- Random Rebroadcast Delay ---------- */
#define REBROADCAST_MIN_MS  1000
#define REBROADCAST_MAX_MS 10000

/* ---------- Dedup Table ---------- */
typedef struct {
  String nodeId;
  uint16_t lastSeq;
  unsigned long lastSeen;
} DedupEntry;

DedupEntry dedupTable[MAX_NODES];
uint8_t dedupCount = 0;

/* ---------- Prototypes ---------- */
bool isDuplicate(String nodeId, uint16_t seq);
void setAS32GeneralMode();

/* ============================================================
   SETUP
   ============================================================ */
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LORA_MD0, OUTPUT);
  pinMode(LORA_MD1, OUTPUT);
  pinMode(LORA_AUX, INPUT);

  setAS32GeneralMode();

  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);

  randomSeed(esp_random());  // ESP32 hardware RNG

  Serial.println("=== Code 1.3.1 : Dedup + Random Rebroadcast ===");
}

/* ============================================================
   LOOP
   ============================================================ */
void loop() {
  if (LoRaSerial.available()) {
    String packet = LoRaSerial.readStringUntil('\n');
    packet.trim();

    Serial.print("RX: ");
    Serial.println(packet);

    int p1 = packet.indexOf('|');
    int p2 = packet.indexOf('|', p1 + 1);

    if (p1 == -1 || p2 == -1) {
      Serial.println("Invalid packet → DROPPED");
      return;
    }

    String nodeId = packet.substring(0, p1);
    uint16_t seq  = packet.substring(p1 + 1, p2).toInt();

    if (isDuplicate(nodeId, seq)) {
      Serial.println("Duplicate → DROPPED");
    } else {
      Serial.println("New packet → ACCEPTED");

      // Random rebroadcast delay
      unsigned long delayMs =
        random(REBROADCAST_MIN_MS, REBROADCAST_MAX_MS + 1);

      Serial.print("Rebroadcasting in ");
      Serial.print(delayMs / 1000.0);
      Serial.println(" seconds");

      delay(delayMs);

      LoRaSerial.println(packet);  // Rebroadcast
      Serial.println("Packet REBROADCASTED");
    }

    Serial.println("--------------------------------");
  }
}

/* ============================================================
   Deduplication Logic (Time-based)
   ============================================================ */
bool isDuplicate(String nodeId, uint16_t seq) {
  unsigned long now = millis();

  for (int i = 0; i < dedupCount; i++) {
    if (dedupTable[i].nodeId == nodeId &&
        dedupTable[i].lastSeq == seq) {

      if (now - dedupTable[i].lastSeen < DEDUP_WINDOW_MS) {
        return true;
      } else {
        dedupTable[i].lastSeen = now;
        return false;
      }
    }
  }

  if (dedupCount < MAX_NODES) {
    dedupTable[dedupCount++] = { nodeId, seq, now };
  }

  return false;
}

/* ============================================================
   AS32 General Mode (MD0=0, MD1=0)
   ============================================================ */
void setAS32GeneralMode() {
  digitalWrite(LORA_MD0, LOW);  // D25
  digitalWrite(LORA_MD1, LOW);  // D26
  delay(10);
}
