/* ============================================================
   Code 1.X : LoRa Receiver + Backend Upload Test
   Board    : NodeMCU ESP32 CH340 (ESP-WROOM-32)
   Module   : AS32-TTL-100
   Purpose  : Receive LoRa messages, parse by '\n',
              upload to webhook backend

   NOTE:
   - AS32 in General Mode (MD0=0, MD1=0)
   - Messages MUST end with '\n'
   ============================================================ */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

/* ================= PIN DEFINITIONS ================= */
#define LORA_RX_PIN   16   // D17 → AS32 RXD
#define LORA_TX_PIN   17   // D16 → AS32 TXD
#define LORA_MD0_PIN  25   // D25 → MD0
#define LORA_MD1_PIN  26   // D26 → MD1
#define LORA_AUX_PIN  27   // D27 → AUX

/* ================= LOCAL WIFI CONFIG ================= */
// Copy secrets.example.h to secrets.h and enter your own lab settings.
#include "secrets.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* serverUrl = WEBHOOK_URL;

/* ================= UART ================= */
HardwareSerial LoRaSerial(2);

/* ================= BUFFER ================= */
String rxBuffer = "";

void setup() {
  Serial.begin(115200);

  /* LoRa Mode: General Mode */
  pinMode(LORA_MD0_PIN, OUTPUT);
  pinMode(LORA_MD1_PIN, OUTPUT);
  digitalWrite(LORA_MD0_PIN, LOW);
  digitalWrite(LORA_MD1_PIN, LOW);

  pinMode(LORA_AUX_PIN, INPUT);

  /* Start LoRa UART */
  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);

  /* WiFi Connect */
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.println(WiFi.localIP());
}

void loop() {
  /* Read incoming LoRa data */
  while (LoRaSerial.available()) {
    char c = LoRaSerial.read();
    rxBuffer += c;

    /* Complete message detected */
    if (c == '\n') {
      Serial.print("[LoRa RX] ");
      Serial.print(rxBuffer);

      uploadData(rxBuffer);
      rxBuffer = "";  // Clear buffer
    }
  }
}

/* ================= HTTP UPLOAD ================= */
void uploadData(String payload) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "text/plain");

  int responseCode = http.POST(payload);

  Serial.print("[HTTP] Response: ");
  Serial.println(responseCode);

  http.end();
}
