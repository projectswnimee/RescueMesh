#include <Arduino.h>

/* ============================================================
   Code 1.3.1 : Deduplication Test Code (Time-based)
   Dedup Window : 5 seconds (TEST)
   ============================================================ */

/* ---------- Pin Definitions (On-board D names) ---------- */
#define LORA_TX   16   // GPIO16 (D16) → AS32 TXD
#define LORA_RX   17   // GPIO17 (D17) → AS32 RXD
#define LORA_MD0  25   // GPIO25 (D25)
#define LORA_MD1  26   // GPIO26 (D26)
#define LORA_AUX  27   // GPIO27 (D27)

/* ---------- UART ---------- */
HardwareSerial LoRaSerial(2);

/* ---------- Dedup Settings ---------- */
#define MAX_NODES        10
#define DEDUP_WINDOW_MS 5000   // 5 seconds (TEST)

/* ---------- Dedup Table ---------- */
typedef struct {
  String   nodeId;
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

  Serial.println("=== Code 1.3.1 : Time-based Deduplication Test ===");
}

/* ============================================================
   LOOP
   ============================================================ */
void loop() {
  if (LoRaSerial.available()) {
    String packet = LoRaSerial.readStringUntil('\n');
    packet.trim();

    Serial.print("RX RAW: ");
    Serial.println(packet);

    int p1 = packet.indexOf('|');
    int p2 = packet.indexOf('|', p1 + 1);

    if (p1 == -1 || p2 == -1) {
      Serial.println("Invalid packet → DROPPED");
      return;
    }

    String nodeId = packet.substring(0, p1);
    uint16_t seq  = packet.substring(p1 + 1, p2).toInt();
    String data   = packet.substring(p2 + 1);

    if (isDuplicate(nodeId, seq)) {
      Serial.println("DUPLICATE (within 5s window) → DROPPED");
    } else {
      Serial.println("ACCEPTED");
      Serial.print("Node: "); Serial.println(nodeId);
      Serial.print("Seq : "); Serial.println(seq);
      Serial.print("Data: "); Serial.println(data);
    }

    Serial.println("--------------------------------");
  }
}

/* ============================================================
   Time-based Deduplication Logic
   ============================================================ */
bool isDuplicate(String nodeId, uint16_t seq) {
  unsigned long now = millis();

  for (int i = 0; i < dedupCount; i++) {
    if (dedupTable[i].nodeId == nodeId &&
        dedupTable[i].lastSeq == seq) {

      if (now - dedupTable[i].lastSeen < DEDUP_WINDOW_MS) {
        return true;   // Duplicate within time window
      } else {
        dedupTable[i].lastSeen = now; // Window expired
        return false;
      }
    }
  }

  // New entry
  if (dedupCount < MAX_NODES) {
    dedupTable[dedupCount].nodeId   = nodeId;
    dedupTable[dedupCount].lastSeq  = seq;
    dedupTable[dedupCount].lastSeen = now;
    dedupCount++;
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
