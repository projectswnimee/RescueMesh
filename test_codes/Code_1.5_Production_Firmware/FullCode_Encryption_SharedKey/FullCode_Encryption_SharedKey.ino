/* ============================================================
   FloodGuard AIoT — Field Node v4.3 (SIMPLE BUTTON PRESS)
   ============================================================
   BEHAVIOR:
   - GENERAL button: Press once → sends ONE alert
   - CRITICAL button: Press once → sends ONE alert
   - NO auto-repeat, NO toggle mode
   - Each press sends exactly ONE message
   ============================================================ */

#include <Arduino.h>
#include "BluetoothSerial.h"
#include <EEPROM.h>

/* ================= PINOUT ================= */
#define LORA_TX       17
#define LORA_RX       16
#define LORA_MD0      25
#define LORA_MD1      26
#define BTN_GENERAL   33
#define BTN_CRITICAL  32
#define LED_STATUS    2
#define LED_REPLY     4

HardwareSerial LoRaSerial(2);
BluetoothSerial SerialBT;

/* ================= DEMONSTRATION WHITELIST ================= */
// Public examples only. These identifiers do not provide secure authentication.
const String VALID_KEYS[] = {
  "DEMO0001",   // NODE 01
  "DEMO0002",   // NODE 02
  "DEMO0003",   // NODE 03
  "DEMO0004",   // NODE 04
  "DEMO0005"    // NODE 05
};
const int VALID_KEY_COUNT = 5;

/* ================= NODE CONFIG ================= */
String NODE_KEY = "";
String NODE_NUMBER = "";
bool NODE_REGISTERED = false;
String LATITUDE = "6.9271";
String LONGITUDE = "79.8612";

// EEPROM addresses
#define EEPROM_KEY_ADDR    0
#define EEPROM_KEY_LEN     16
#define EEPROM_REG_FLAG    32

/* ================= MESSAGE CONFIG ================= */
uint8_t seqCounter = 1;
const int DEFAULT_TTL = 5;

/* ================= BUFFERS ================= */
String rxBuffer = "";

/* ================= DEDUP ================= */
String dedupList[30];
uint8_t dedupIndex = 0;

/* ================= BUTTON STATE ================= */
bool lastGeneralState = HIGH;
bool lastCriticalState = HIGH;

/* ================= DEBOUNCE ================= */
unsigned long lastGeneralDebounce = 0;
unsigned long lastCriticalDebounce = 0;
const unsigned long DEBOUNCE_DELAY = 200;

/* ================= LED PATTERNS ================= */
enum LEDPattern {
  LED_NORMAL,
  LED_SENDING,
  LED_REPLY_RECEIVED,
  LED_INTRUDER
};
LEDPattern currentPattern = LED_NORMAL;
unsigned long lastLedBlink = 0;
int intruderCount = 0;

/* ============================================================
   KEY VALIDATION
   ============================================================ */

bool isValidNodeKey(String key) {
  for (int i = 0; i < VALID_KEY_COUNT; i++) {
    if (VALID_KEYS[i] == key) return true;
  }
  return false;
}

void logIntruder(String key) {
  intruderCount++;
  Serial.println("╔═══════════════════════════════════════╗");
  Serial.println("║   ⚠️ INTRUDER ALERT ⚠️                 ║");
  Serial.println("╠═══════════════════════════════════════╣");
  Serial.print("║ INVALID KEY: ");
  Serial.println(key);
  Serial.print("║ COUNT:       ");
  Serial.println(String(intruderCount));
  Serial.println("╚═══════════════════════════════════════╝");
  currentPattern = LED_INTRUDER;
  lastLedBlink = millis();
}

/* ============================================================
   SYMBOL ENCRYPTION/DECRYPTION
   ============================================================ */

String encryptPayload(String plain) {
  String encrypted = "";
  for (int i = 0; i < plain.length(); i++) {
    char c = plain.charAt(i);
    switch (c) {
      case 'A': case 'a': encrypted += "@!"; break;
      case 'B': case 'b': encrypted += "#$"; break;
      case 'C': case 'c': encrypted += "%^"; break;
      case 'D': case 'd': encrypted += "&*"; break;
      case 'E': case 'e': encrypted += "(-"; break;
      case 'F': case 'f': encrypted += ")+"; break;
      case 'G': case 'g': encrypted += "[="; break;
      case 'H': case 'h': encrypted += "]{"; break;
      case 'I': case 'i': encrypted += "}|"; break;
      case 'J': case 'j': encrypted += "\\:"; break;
      case 'K': case 'k': encrypted += ";'"; break;
      case 'L': case 'l': encrypted += "\"<"; break;
      case 'M': case 'm': encrypted += ">?"; break;
      case 'N': case 'n': encrypted += "/~"; break;
      case 'O': case 'o': encrypted += "`1"; break;
      case 'P': case 'p': encrypted += "2@"; break;
      case 'Q': case 'q': encrypted += "3#"; break;
      case 'R': case 'r': encrypted += "4$"; break;
      case 'S': case 's': encrypted += "5%"; break;
      case 'T': case 't': encrypted += "6^"; break;
      case 'U': case 'u': encrypted += "7&"; break;
      case 'V': case 'v': encrypted += "8*"; break;
      case 'W': case 'w': encrypted += "9("; break;
      case 'X': case 'x': encrypted += "0)"; break;
      case 'Y': case 'y': encrypted += "-="; break;
      case 'Z': case 'z': encrypted += "+="; break;
      case '0': encrypted += "aa"; break;
      case '1': encrypted += "bb"; break;
      case '2': encrypted += "cc"; break;
      case '3': encrypted += "dd"; break;
      case '4': encrypted += "ee"; break;
      case '5': encrypted += "ff"; break;
      case '6': encrypted += "gg"; break;
      case '7': encrypted += "hh"; break;
      case '8': encrypted += "ii"; break;
      case '9': encrypted += "jj"; break;
      case '!': encrypted += "kk"; break;
      case '?': encrypted += "ll"; break;
      case '.': encrypted += "mm"; break;
      case ',': encrypted += "nn"; break;
      case ':': encrypted += "oo"; break;
      case ';': encrypted += "pp"; break;
      case ' ': encrypted += "**"; break;
      default:  encrypted += "??"; break;
    }
  }
  return encrypted;
}

String decryptPayload(String enc) {
  String decrypted = "";
  for (int i = 0; i + 1 < enc.length(); i += 2) {
    String sym = enc.substring(i, i + 2);
    if      (sym == "@!") decrypted += "A";
    else if (sym == "#$") decrypted += "B";
    else if (sym == "%^") decrypted += "C";
    else if (sym == "&*") decrypted += "D";
    else if (sym == "(-") decrypted += "E";
    else if (sym == ")+") decrypted += "F";
    else if (sym == "[=") decrypted += "G";
    else if (sym == "]{") decrypted += "H";
    else if (sym == "}|") decrypted += "I";
    else if (sym == "\\:") decrypted += "J";
    else if (sym == ";'") decrypted += "K";
    else if (sym == "\"<") decrypted += "L";
    else if (sym == ">?") decrypted += "M";
    else if (sym == "/~") decrypted += "N";
    else if (sym == "`1") decrypted += "O";
    else if (sym == "2@") decrypted += "P";
    else if (sym == "3#") decrypted += "Q";
    else if (sym == "4$") decrypted += "R";
    else if (sym == "5%") decrypted += "S";
    else if (sym == "6^") decrypted += "T";
    else if (sym == "7&") decrypted += "U";
    else if (sym == "8*") decrypted += "V";
    else if (sym == "9(") decrypted += "W";
    else if (sym == "0)") decrypted += "X";
    else if (sym == "-=") decrypted += "Y";
    else if (sym == "+=") decrypted += "Z";
    else if (sym == "aa") decrypted += "0";
    else if (sym == "bb") decrypted += "1";
    else if (sym == "cc") decrypted += "2";
    else if (sym == "dd") decrypted += "3";
    else if (sym == "ee") decrypted += "4";
    else if (sym == "ff") decrypted += "5";
    else if (sym == "gg") decrypted += "6";
    else if (sym == "hh") decrypted += "7";
    else if (sym == "ii") decrypted += "8";
    else if (sym == "jj") decrypted += "9";
    else if (sym == "kk") decrypted += "!";
    else if (sym == "ll") decrypted += "?";
    else if (sym == "mm") decrypted += ".";
    else if (sym == "nn") decrypted += ",";
    else if (sym == "oo") decrypted += ":";
    else if (sym == "pp") decrypted += ";";
    else if (sym == "**") decrypted += " ";
    else                  decrypted += "?";
  }
  return decrypted;
}

/* ============================================================
   EEPROM STORAGE
   ============================================================ */

void saveNodeToEEPROM() {
  for (int i = 0; i < EEPROM_KEY_LEN; i++) {
    EEPROM.write(EEPROM_KEY_ADDR + i, 0);
  }
  for (int i = 0; i < NODE_KEY.length() && i < EEPROM_KEY_LEN; i++) {
    EEPROM.write(EEPROM_KEY_ADDR + i, NODE_KEY[i]);
  }
  EEPROM.write(EEPROM_REG_FLAG, 1);
  EEPROM.commit();
  Serial.print("[EEPROM] Node key saved: ");
  Serial.println(NODE_KEY);
}

void loadNodeFromEEPROM() {
  EEPROM.begin(512);
  byte registered = EEPROM.read(EEPROM_REG_FLAG);
  
  if (registered == 1) {
    char keyBuf[EEPROM_KEY_LEN + 1];
    bool hasValidKey = false;
    
    for (int i = 0; i < EEPROM_KEY_LEN; i++) {
      char c = EEPROM.read(EEPROM_KEY_ADDR + i);
      if (c >= 'A' && c <= 'Z' || c >= '0' && c <= '9') {
        keyBuf[i] = c;
        hasValidKey = true;
      } else {
        keyBuf[i] = '\0';
        break;
      }
    }
    keyBuf[EEPROM_KEY_LEN] = '\0';
    
    if (hasValidKey && String(keyBuf).length() >= 6) {
      NODE_KEY = String(keyBuf);
      NODE_NUMBER = "Node_" + NODE_KEY.substring(0, 4);
      NODE_REGISTERED = true;
      Serial.print("✓ Loaded from EEPROM: ");
      Serial.println(NODE_KEY);
    } else {
      NODE_REGISTERED = false;
    }
  }
}

/* ============================================================
   REGISTRATION
   ============================================================ */

void registerNode() {
  Serial.println("\n╔═══════════════════════════════════════╗");
  Serial.println("║   NODE NOT REGISTERED                 ║");
  Serial.println("║   Valid keys:                         ║");
  for (int i = 0; i < VALID_KEY_COUNT; i++) {
    Serial.print("║     ");
    Serial.println(VALID_KEYS[i]);
  }
  Serial.println("║                                       ║");
  Serial.println("║   Enter 8-char key from above:        ║");
  Serial.println("╚═══════════════════════════════════════╝");
  Serial.print("Enter key: ");
  
  unsigned long startTime = millis();
  String inputKey = "";
  bool inputComplete = false;
  
  while (millis() - startTime < 60000) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        if (inputKey.length() > 0) {
          inputComplete = true;
          break;
        }
      } else if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
        inputKey += c;
        Serial.print(c);
      }
    }
    delay(10);
  }
  
  if (inputComplete && isValidNodeKey(inputKey)) {
    NODE_KEY = inputKey;
    NODE_NUMBER = "Node_" + NODE_KEY.substring(0, 4);
    NODE_REGISTERED = true;
    saveNodeToEEPROM();
    
    Serial.println();
    Serial.println("✓ Registration successful!");
    Serial.print("  KEY: ");
    Serial.println(NODE_KEY);
    Serial.print("  NAME: ");
    Serial.println(NODE_NUMBER);
    delay(2000);
  } else {
    Serial.println();
    Serial.println("✗ Invalid key! Restart to try again.");
    while(1) {
      digitalWrite(LED_STATUS, HIGH);
      delay(200);
      digitalWrite(LED_STATUS, LOW);
      delay(200);
    }
  }
}

/* ============================================================
   BLUETOOTH SETUP
   ============================================================ */

void setupBluetooth() {
  String btName = "FloodGuard_User";
  
  if (NODE_REGISTERED && NODE_KEY.length() > 0) {
    String uniqueId = NODE_KEY.substring(NODE_KEY.length() - 4);
    btName = "FloodGuard_" + uniqueId;
  }
  
  SerialBT.begin(btName);
  
  Serial.print("[BLUETOOTH] Name: ");
  Serial.println(btName);
  Serial.println("[BLUETOOTH] Ready to pair. No PIN required.");
}

/* ============================================================
   LED STATUS
   ============================================================ */

void updateLED() {
  unsigned long now = millis();
  
  switch (currentPattern) {
    case LED_NORMAL:
      digitalWrite(LED_STATUS, HIGH);
      digitalWrite(LED_REPLY, LOW);
      break;
      
    case LED_SENDING:
      if (now - lastLedBlink < 100) digitalWrite(LED_STATUS, HIGH);
      else if (now - lastLedBlink < 200) digitalWrite(LED_STATUS, LOW);
      else lastLedBlink = now;
      break;
      
    case LED_REPLY_RECEIVED:
      if (now - lastLedBlink < 500) {
        digitalWrite(LED_STATUS, HIGH);
        digitalWrite(LED_REPLY, HIGH);
      } else if (now - lastLedBlink < 1000) {
        digitalWrite(LED_STATUS, LOW);
        digitalWrite(LED_REPLY, LOW);
      } else {
        currentPattern = LED_NORMAL;
      }
      break;
      
    case LED_INTRUDER:
      if (now - lastLedBlink < 100) digitalWrite(LED_STATUS, HIGH);
      else if (now - lastLedBlink < 200) digitalWrite(LED_STATUS, LOW);
      else if (now - lastLedBlink < 300) digitalWrite(LED_STATUS, HIGH);
      else if (now - lastLedBlink < 400) digitalWrite(LED_STATUS, LOW);
      else lastLedBlink = now;
      break;
  }
}

/* ============================================================
   DISPLAY REPLY
   ============================================================ */

void displayReply(String decryptedMessage, String msgId) {
  currentPattern = LED_REPLY_RECEIVED;
  lastLedBlink = millis();
  
  Serial.println("\n");
  Serial.println("╔══════════════════════════════════════════════════════════════╗");
  Serial.println("║                    📨 REPLY FROM RESCUE TEAM                  ║");
  Serial.println("╠══════════════════════════════════════════════════════════════╣");
  Serial.print("║ MSG ID:     ");
  Serial.println(msgId);
  Serial.print("║ TO NODE:    ");
  Serial.print(NODE_KEY);
  Serial.print(" (");
  Serial.print(NODE_NUMBER);
  Serial.println(")");
  Serial.println("╠══════════════════════════════════════════════════════════════╣");
  Serial.println("║ MESSAGE:                                                    ║");
  
  String remaining = decryptedMessage;
  while (remaining.length() > 50) {
    Serial.print("║ ");
    Serial.println(remaining.substring(0, 50));
    remaining = remaining.substring(50);
  }
  if (remaining.length() > 0) {
    Serial.print("║ ");
    Serial.println(remaining);
  }
  
  Serial.println("╚══════════════════════════════════════════════════════════════╝");
  Serial.println();
  
  if (SerialBT.hasClient()) {
    SerialBT.print("REPLY: ");
    SerialBT.println(decryptedMessage);
  }
}

/* ============================================================
   DEDUP
   ============================================================ */

bool isDuplicate(String id) {
  for (int i = 0; i < 30; i++) {
    if (dedupList[i] == id) return true;
  }
  return false;
}

void storeDedupID(String id) {
  dedupList[dedupIndex] = id;
  dedupIndex = (dedupIndex + 1) % 30;
}

/* ============================================================
   SEND MESSAGE
   ============================================================ */

void sendLoRaMessage(String payload, String flag) {
  if (!NODE_REGISTERED) {
    Serial.println("[ERROR] Node not registered!");
    return;
  }
  
  currentPattern = LED_SENDING;
  lastLedBlink = millis();
  
  String seqStr = (seqCounter < 10) ? "0" + String(seqCounter) : String(seqCounter);
  seqCounter++;
  
  String encPayload = encryptPayload(payload);
  
  String fullMsg = "FG|" + NODE_KEY + "|" + seqStr + "|" + 
                   String(DEFAULT_TTL) + "|" + NODE_KEY + "|" +
                   flag + "|" + LATITUDE + "|" + LONGITUDE + "|" +
                   encPayload + "\n";
  
  LoRaSerial.write(fullMsg.c_str(), fullMsg.length());
  
  Serial.println("────────────────────────────────────────");
  Serial.print("[SENT] From: ");
  Serial.println(NODE_KEY);
  Serial.print("[FLAG] ");
  Serial.println(flag);
  Serial.print("[MSG ] ");
  Serial.println(payload);
  Serial.println("────────────────────────────────────────");
  
  delay(100);
  currentPattern = LED_NORMAL;
}

/* ============================================================
   BUTTON HANDLERS (SIMPLE — ONE PRESS = ONE MESSAGE)
   ============================================================ */

void handleButtons() {
  unsigned long now = millis();
  
  // Read current button states (LOW = pressed, because INPUT_PULLUP)
  bool currentGeneral = digitalRead(BTN_GENERAL);
  bool currentCritical = digitalRead(BTN_CRITICAL);
  
  // GENERAL BUTTON — Rising edge detection (released after press)
  if (lastGeneralState == LOW && currentGeneral == HIGH) {
    if (now - lastGeneralDebounce > DEBOUNCE_DELAY) {
      lastGeneralDebounce = now;
      sendLoRaMessage("General safety issue reported", "BTN_GENERAL");
      Serial.println("[BTN] GENERAL alert sent");
    }
  }
  lastGeneralState = currentGeneral;
  
  // CRITICAL BUTTON — Rising edge detection (released after press)
  if (lastCriticalState == LOW && currentCritical == HIGH) {
    if (now - lastCriticalDebounce > DEBOUNCE_DELAY) {
      lastCriticalDebounce = now;
      sendLoRaMessage("CRITICAL EMERGENCY SOS", "BTN_CRITICAL");
      Serial.println("[BTN] CRITICAL alert sent");
    }
  }
  lastCriticalState = currentCritical;
}

/* ============================================================
   PROCESS RECEIVED
   ============================================================ */

void processReceived(String raw) {
  raw.trim();
  if (raw.length() == 0) return;
  
  /* ========== HANDLE BACKEND REPLY ========== */
  if (raw.startsWith("FGR|")) {
    int p1 = raw.indexOf('|');
    int p2 = raw.indexOf('|', p1 + 1);
    int p3 = raw.indexOf('|', p2 + 1);
    
    if (p1 > 0 && p2 > p1 && p3 > p2) {
      String destKey = raw.substring(p1 + 1, p2);
      String msgId = raw.substring(p2 + 1, p3);
      String encPayload = raw.substring(p3 + 1);
      encPayload.trim();
      
      if (destKey == NODE_KEY) {
        String decrypted = decryptPayload(encPayload);
        displayReply(decrypted, msgId);
        Serial.println("[REPLY] ✅ For THIS node - DISPLAYED");
        return;
      } else if (isValidNodeKey(destKey)) {
        int delayMs = random(1000, 5001);
        delay(delayMs);
        LoRaSerial.write((raw + "\n").c_str(), raw.length() + 1);
        return;
      }
    }
    return;
  }
  
  /* ========== HANDLE LOCATION UPDATE ========== */
  if (raw.startsWith("LOC|")) {
    int p1 = raw.indexOf('|');
    int p2 = raw.indexOf('|', p1 + 1);
    int p3 = raw.indexOf('|', p2 + 1);
    
    if (p1 > 0 && p2 > p1 && p3 > p2) {
      String destKey = raw.substring(p1 + 1, p2);
      
      if (destKey == NODE_KEY) {
        LATITUDE = raw.substring(p2 + 1, p3);
        LONGITUDE = raw.substring(p3 + 1);
        Serial.print("[LOCATION] Updated: ");
        Serial.print(LATITUDE);
        Serial.print(", ");
        Serial.println(LONGITUDE);
        return;
      } else if (isValidNodeKey(destKey)) {
        int delayMs = random(1000, 5001);
        delay(delayMs);
        LoRaSerial.write((raw + "\n").c_str(), raw.length() + 1);
        return;
      }
    }
    return;
  }
  
  /* ========== HANDLE FIELD MESSAGE ========== */
  if (!raw.startsWith("FG|")) return;
  
  int p1 = raw.indexOf('|');
  int p2 = raw.indexOf('|', p1 + 1);
  int p3 = raw.indexOf('|', p2 + 1);
  int p4 = raw.indexOf('|', p3 + 1);
  int p5 = raw.indexOf('|', p4 + 1);
  int p6 = raw.indexOf('|', p5 + 1);
  int p7 = raw.indexOf('|', p6 + 1);
  int p8 = raw.indexOf('|', p7 + 1);
  
  if (p1 < 0 || p2 < 0 || p3 < 0 || p4 < 0 || p5 < 0 || p6 < 0 || p7 < 0 || p8 < 0) return;
  
  String nodeKey = raw.substring(p1 + 1, p2);
  String seqNo   = raw.substring(p2 + 1, p3);
  int    ttl     = raw.substring(p3 + 1, p4).toInt();
  String path    = raw.substring(p4 + 1, p5);
  String flag    = raw.substring(p5 + 1, p6);
  String lat     = raw.substring(p6 + 1, p7);
  String lon     = raw.substring(p7 + 1, p8);
  String payload = raw.substring(p8 + 1);
  
  if (!isValidNodeKey(nodeKey)) {
    logIntruder(nodeKey);
    return;
  }
  
  if (ttl <= 0) return;
  if (nodeKey == NODE_KEY) return;
  
  String dedupID = nodeKey + seqNo;
  if (isDuplicate(dedupID)) return;
  storeDedupID(dedupID);
  
  String newPath = path + ">" + NODE_KEY;
  int delayMs = random(1000, 5001);
  delay(delayMs);
  
  String relayMsg = "FG|" + nodeKey + "|" + seqNo + "|" +
                    String(ttl - 1) + "|" + newPath + "|" +
                    "RELAY" + "|" + lat + "|" + lon + "|" +
                    payload + "\n";
  
  LoRaSerial.write(relayMsg.c_str(), relayMsg.length());
}

/* ============================================================
   SETUP
   ============================================================ */

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(LORA_MD0, OUTPUT);
  pinMode(LORA_MD1, OUTPUT);
  digitalWrite(LORA_MD0, LOW);
  digitalWrite(LORA_MD1, LOW);
  
  pinMode(BTN_GENERAL, INPUT_PULLUP);
  pinMode(BTN_CRITICAL, INPUT_PULLUP);
  pinMode(LED_STATUS, OUTPUT);
  pinMode(LED_REPLY, OUTPUT);
  
  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);
  randomSeed(micros());
  
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║     FloodGuard AIoT — Field Node    ║");
  Serial.println("║         Disaster Communication       ║");
  Serial.println("╚══════════════════════════════════════╝");
  
  loadNodeFromEEPROM();
  
  if (!NODE_REGISTERED) {
    registerNode();
  }
  
  setupBluetooth();
  
  // Initialize button states
  lastGeneralState = digitalRead(BTN_GENERAL);
  lastCriticalState = digitalRead(BTN_CRITICAL);
  
  Serial.println("\n────────────────────────────────────────");
  Serial.println("SYSTEM READY");
  Serial.println("────────────────────────────────────────");
  Serial.println("🔘 GENERAL  : GPIO33 — Press to send general alert");
  Serial.println("🔘 CRITICAL : GPIO32 — Press to send critical alert");
  
  String btStatus = "📱 Bluetooth: ";
  if (SerialBT.hasClient()) {
    btStatus += "Connected";
  } else {
    btStatus += "Waiting for connection";
  }
  Serial.println(btStatus);
  
  Serial.println("💡 LED      : Solid = normal, Blink = sending");
  Serial.println("────────────────────────────────────────\n");
  
  digitalWrite(LED_STATUS, HIGH);
}

/* ============================================================
   LOOP
   ============================================================ */

void loop() {
  handleButtons();
  updateLED();
  
  // Bluetooth input
  if (SerialBT.available()) {
    String btMsg = SerialBT.readStringUntil('\n');
    btMsg.trim();
    if (btMsg.length() > 0 && btMsg.length() < 200) {
      sendLoRaMessage(btMsg, "BT");
      Serial.print("[BT] Sent: ");
      Serial.println(btMsg);
      SerialBT.print("Sent: ");
      SerialBT.println(btMsg);
    }
  }
  
  // Serial input
  if (Serial.available()) {
    String userMsg = Serial.readStringUntil('\n');
    userMsg.trim();
    if (userMsg.length() > 0) {
      if (userMsg == "STATUS") {
        Serial.println("=== NODE STATUS ===");
        Serial.print("KEY: ");
        Serial.println(NODE_KEY);
        Serial.print("NAME: ");
        Serial.println(NODE_NUMBER);
        Serial.print("BT NAME: FloodGuard_");
        Serial.println(NODE_KEY.substring(NODE_KEY.length() - 4));
        Serial.print("INTRUDER COUNT: ");
        Serial.println(intruderCount);
        Serial.println("==================");
      } else if (userMsg == "KEYS") {
        Serial.println("=== VALID KEYS ===");
        for (int i = 0; i < VALID_KEY_COUNT; i++) {
          Serial.println(VALID_KEYS[i]);
        }
        Serial.println("==================");
      } else {
        sendLoRaMessage(userMsg, "SERIAL");
      }
    }
  }
  
  // LoRa receive
  while (LoRaSerial.available()) {
    char c = LoRaSerial.read();
    if (c == '\n') {
      if (rxBuffer.length() > 0) {
        processReceived(rxBuffer);
        rxBuffer = "";
      }
    } else if (c != '\r') {
      rxBuffer += c;
    }
  }
  
  delay(10);
}
