/* ============================================================
   Code 1.2F : Messaging + Deduplication + TTL + Random Delay
               + Bluetooth Classic Input
   Board    : NodeMCU ESP32 CH340 (ESP-WROOM-32)
   Module   : AS32-TTL-100 LoRa

   Message Format:
   Node01|01|10|Message_String\n

   Dedup Key:
   NodeNumber + SEQ  → "Node0101"

   TTL:
   - Checked BEFORE deduplication
   - Dropped when TTL <= 0

   Bluetooth:
   - Phone sends text ending with '\n'
   - ESP32 forwards it to LoRa

   Random Rebroadcast Delay:
   - 1 to 5 seconds

   ============================================================ */

#include <Arduino.h>
#include "BluetoothSerial.h"

/* ================= PINOUT (SHARED – DO NOT CHANGE) =================
   ESP32 PIN   →  AS32 PIN
   D17 (GPIO17) → RXD
   D16 (GPIO16) → TXD
   D25 (GPIO25) → MD0
   D26 (GPIO26) → MD1
   ================================================================= */

#define LORA_TX   17   // D17 → AS32 RXD
#define LORA_RX   16   // D16 → AS32 TXD
#define LORA_MD0  25   // D25 → MD0
#define LORA_MD1  26   // D26 → MD1

HardwareSerial LoRaSerial(2);
BluetoothSerial SerialBT;

/* ================= NODE CONFIG ================= */
const String NODE_NUMBER = "Node01";   // CHANGE PER NODE
uint8_t seqCounter = 1;
const int DEFAULT_TTL = 1;

/* ================= BUFFERS ================= */
String rxBuffer = "";

/* ================= DEDUP STORAGE ================= */
String dedupList[10];
uint8_t dedupIndex = 0;

/* ================= DEDUP FUNCTIONS ================= */

bool isDuplicate(String id) {
  for (int i = 0; i < 10; i++) {
    if (dedupList[i] == id) return true;
  }
  return false;
}

void storeDedupID(String id) {
  dedupList[dedupIndex] = id;
  dedupIndex = (dedupIndex + 1) % 10;
}

/* ================= SETUP ================= */

void setup() {
  Serial.begin(115200);
  delay(1000);

  /* LoRa Mode: NORMAL */
  pinMode(LORA_MD0, OUTPUT);
  pinMode(LORA_MD1, OUTPUT);
  digitalWrite(LORA_MD0, LOW);
  digitalWrite(LORA_MD1, LOW);

  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);

  /* Bluetooth Classic */
  SerialBT.begin("ESP32_BT_MONITOR");

  randomSeed(micros());

  Serial.println("=== Code 1.2F : LoRa + Dedup + TTL + Bluetooth ===");
  Serial.println("Bluetooth device name: ESP32_BT_MONITOR");
}

/* ================= LOOP ================= */

void loop() {

  /* ============================================================
     BLUETOOTH → LORA TRANSMISSION
     ============================================================ */
  if (SerialBT.available()) {
    String btMsg = SerialBT.readStringUntil('\n');
    btMsg.trim();

    if (btMsg.length()) {

      String seqStr = (seqCounter < 10) ? "0" + String(seqCounter) : String(seqCounter);
      seqCounter++;

      String fullMsg =
        NODE_NUMBER + "|" +
        seqStr + "|" +
        String(DEFAULT_TTL) + "|" +
        btMsg + "\n";

      LoRaSerial.write(fullMsg.c_str(), fullMsg.length());

      Serial.print("[BT → LoRa] ");
      Serial.print(fullMsg);
    }
  }

  /* ============================================================
     SERIAL MONITOR → LORA (OPTIONAL)
     ============================================================ */
  if (Serial.available()) {
    String userMsg = Serial.readStringUntil('\n');

    String seqStr = (seqCounter < 10) ? "0" + String(seqCounter) : String(seqCounter);
    seqCounter++;

    String fullMsg =
      NODE_NUMBER + "|" +
      seqStr + "|" +
      String(DEFAULT_TTL) + "|" +
      userMsg + "\n";

    LoRaSerial.write(fullMsg.c_str(), fullMsg.length());

    Serial.print("[Serial → LoRa] ");
    Serial.print(fullMsg);
  }

  /* ============================================================
     LORA RECEIVE + DEDUP + TTL + REBROADCAST
     ============================================================ */
  while (LoRaSerial.available()) {
    char c = LoRaSerial.read();

    if (c == '\n') {
      Serial.print("[Received] ");
      Serial.println(rxBuffer);

      int p1 = rxBuffer.indexOf('|');
      int p2 = rxBuffer.indexOf('|', p1 + 1);
      int p3 = rxBuffer.indexOf('|', p2 + 1);

      if (p1 > 0 && p2 > p1 && p3 > p2) {

        String nodeNo = rxBuffer.substring(0, p1);
        String seqNo  = rxBuffer.substring(p1 + 1, p2);
        int ttl       = rxBuffer.substring(p2 + 1, p3).toInt();
        String msg    = rxBuffer.substring(p3 + 1);

        /* -------- TTL CHECK -------- */
        if (ttl <= 0) {
          Serial.println("[Dropped] TTL expired");
          rxBuffer = "";
          return;
        }

        /* -------- DEDUP CHECK -------- */
        String dedupID = nodeNo + seqNo;

        if (isDuplicate(dedupID)) {
          Serial.println("[Dropped] Duplicate detected");
          rxBuffer = "";
          return;
        }

        storeDedupID(dedupID);

        /* -------- RANDOM DELAY -------- */
        int randomDelayMs = random(1000, 5001);
        Serial.print("[Rebroadcast Delay] ");
        Serial.print(randomDelayMs);
        Serial.println(" ms");

        delay(randomDelayMs);

        /* -------- REBROADCAST -------- */
        int newTTL = ttl - 1;

        String rebroadcastMsg =
          nodeNo + "|" +
          seqNo + "|" +
          String(newTTL) + "|" +
          msg + "\n";

        LoRaSerial.write(rebroadcastMsg.c_str(), rebroadcastMsg.length());

        Serial.print("[Rebroadcasted] ");
        Serial.print(rebroadcastMsg);
      }

      rxBuffer = "";
    } else {
      rxBuffer += c;
    }
  }
}
