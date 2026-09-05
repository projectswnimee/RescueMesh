#include <Arduino.h>

/* ---------- Pin Definitions (D names) ---------- */
#define LORA_TX   16   // D16 → AS32 TXD
#define LORA_RX   17   // D17 → AS32 RXD
#define LORA_MD0  25   // D25
#define LORA_MD1  26   // D26
#define LORA_AUX  27   // D27

HardwareSerial LoRaSerial(2);

/* ---------- Timing ---------- */
#define DEDUP_WINDOW_MS      5000
#define REBROADCAST_MIN_MS   1000
#define REBROADCAST_MAX_MS  10000
#define MAX_NODES              10

/* ---------- Dedup Table ---------- */
typedef struct {
  String nodeId;
  uint16_t seq;
  unsigned long lastSeen;
} DedupEntry;

DedupEntry dedup[MAX_NODES];
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
  randomSeed(esp_random());

  Serial.println("=== Code 1.3.1 : Dedup + Random Delay + TTL ===");
}

/* ============================================================
   LOOP
   ============================================================ */
void loop() {
  if (!LoRaSerial.available()) return;

  String packet = LoRaSerial.readStringUntil('\n');
  packet.trim();
  Serial.print("RX: "); Serial.println(packet);

  int p1 = packet.indexOf('|');
  int p2 = packet.indexOf('|', p1 + 1);
  int p3 = packet.indexOf('|', p2 + 1);

  if (p1 < 0 || p2 < 0 || p3 < 0) {
    Serial.println("Invalid format → DROPPED");
    return;
  }

  String nodeId = packet.substring(0, p1);
  uint16_t seq  = packet.substring(p1 + 1, p2).toInt();
  int ttl       = packet.substring(p2 + 1, p3).toInt();
  String data   = packet.substring(p3 + 1);

  /* ---------- Deduplication ---------- */
  if (isDuplicate(nodeId, seq)) {
    Serial.println("Duplicate (within window) → DROPPED");
    return;
  }

  Serial.println("New packet → ACCEPTED");

  /* ---------- TTL Handling ---------- */
  if (ttl <= 0) {
    Serial.println("TTL = 0 → NOT REBROADCASTED");
    return;
  }

  ttl--;  // decrement hop count

  /* ---------- Random Delay ---------- */
  unsigned long delayMs =
    random(REBROADCAST_MIN_MS, REBROADCAST_MAX_MS + 1);

  Serial.print("TTL left: ");
  Serial.println(ttl);
  Serial.print("Rebroadcast in ");
  Serial.print(delayMs / 1000.0);
  Serial.println(" s");

  delay(delayMs);

  /* ---------- Rebuild Packet ---------- */
  String rebroadcast =
    nodeId + "|" + String(seq) + "|" + String(ttl) + "|" + data;

  LoRaSerial.println(rebroadcast);
  Serial.println("Packet REBROADCASTED");
  Serial.println("--------------------------------");
}

/* ============================================================
   Deduplication (Time-based)
   ============================================================ */
bool isDuplicate(String nodeId, uint16_t seq) {
  unsigned long now = millis();

  for (int i = 0; i < dedupCount; i++) {
    if (dedup[i].nodeId == nodeId && dedup[i].seq == seq) {
      if (now - dedup[i].lastSeen < DEDUP_WINDOW_MS) return true;
      dedup[i].lastSeen = now;
      return false;
    }
  }

  if (dedupCount < MAX_NODES) {
    dedup[dedupCount++] = { nodeId, seq, now };
  }
  return false;
}

/* ============================================================
   AS32 General Mode
   ============================================================ */
void setAS32GeneralMode() {
  digitalWrite(LORA_MD0, LOW);  // D25
  digitalWrite(LORA_MD1, LOW);  // D26
  delay(10);
}
