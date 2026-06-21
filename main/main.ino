#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <vector>

// ── Pin Mapping ──────────────────────────────────────────────────────
#define RST_PIN    255
#define SS_PIN     2    // D4 (GPIO2)
extern const int BUZZER = 16;  // D0 (GPIO16)
extern const int LED = 15;     // D8 (GPIO15)
#define BUTTON_PIN 0    // D3 (GPIO0)
#define WIFI_LED   LED_BUILTIN

// ── Hardware Instances ───────────────────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN);
ESP8266WebServer server(80);
DNSServer dnsServer;

// ── Global State ─────────────────────────────────────────────────────
enum StationMode {
  MODE_READY,
  MODE_PAYMENT,
  MODE_ADD_CARD
};

StationMode currentMode = MODE_READY;

// Clock sync variables
uint32_t timeSyncEpoch = 0;
unsigned long timeSyncMillis = 0;

// Active mode variables
float activeAmount = 0.0f;
String activeRegName = "";
String activeRegUserId = "";
float activeRegBalance = 0.0f;

// Event for UI polling
struct LastEvent {
  bool processed;
  String status;
  String uid;
  String name;
  float amount;
  float prevBal;
  float remBal;
  String message;
};

LastEvent lastEvent = {true, "", "", "", 0.0f, 0.0f, 0.0f, ""};

// Scan cooldown and display states
unsigned long lastScanTime = 0;
const unsigned long COOLDOWN_DELAY = 1500;
bool readyDisplayed = false;
unsigned long buttonPressStart = 0;

// Cached pending sync count
int pendingSyncCount = 0;

// ── Include Custom Modules ───────────────────────────────────────────
#include "storage.h"
#include "rfid.h"
#include "payment.h"
#include "web.h"

// ── Captive Portal Detection Responses ───────────────────────────────
// These specific paths are checked by Android/iOS/Windows/macOS
// to determine if they're behind a captive portal.
// Returning a redirect makes the device show the login page popup.
void handleCaptivePortalDetect() {
  server.sendHeader("Location", "http://192.168.4.1/", true);
  server.send(302, "text/html", "");
}

// ── REST API Routes ──────────────────────────────────────────────────
void setupRoutes() {
  // Main page — served in small chunks to avoid OOM
  server.on("/", HTTP_GET, []() {
    sendChunkedPage(server);
  });

  // ── Captive Portal Detection Endpoints ────────────────────────────
  // Android
  server.on("/generate_204", HTTP_GET, []() {
    handleCaptivePortalDetect();
  });
  server.on("/gen_204", HTTP_GET, []() {
    handleCaptivePortalDetect();
  });
  // Apple
  server.on("/hotspot-detect.html", HTTP_GET, []() {
    handleCaptivePortalDetect();
  });
  server.on("/library/test/success.html", HTTP_GET, []() {
    handleCaptivePortalDetect();
  });
  // Microsoft
  server.on("/connecttest.txt", HTTP_GET, []() {
    handleCaptivePortalDetect();
  });
  server.on("/ncsi.txt", HTTP_GET, []() {
    handleCaptivePortalDetect();
  });
  server.on("/redirect", HTTP_GET, []() {
    handleCaptivePortalDetect();
  });
  // Firefox
  server.on("/canonical.html", HTTP_GET, []() {
    handleCaptivePortalDetect();
  });
  server.on("/success.txt", HTTP_GET, []() {
    handleCaptivePortalDetect();
  });

  // ── 404 / Captive Portal catch-all ────────────────────────────────
  server.onNotFound([]() {
    if (server.uri().startsWith("/api/")) {
      server.send(404, "application/json", "{\"error\":\"Not Found\"}");
    } else {
      // Redirect everything else to root for captive portal
      server.sendHeader("Location", "http://192.168.4.1/", true);
      server.send(302, "text/plain", "");
    }
  });

  // ── API: Status (lightweight — no flash I/O) ─────────────────────
  server.on("/api/status", HTTP_GET, []() {
    ALLOCATE_JSON_DOCUMENT(doc, 512);
    doc["mode"] = currentMode == MODE_READY ? "READY" : (currentMode == MODE_PAYMENT ? "PAYMENT" : "ADD_CARD");
    doc["activeAmount"] = activeAmount;
    doc["activeUserId"] = activeRegUserId;
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["uptime"] = millis();
    doc["pendingSyncCount"] = pendingSyncCount;

    JsonObject evt = CREATE_NESTED_OBJECT(doc, "lastEvent");
    evt["processed"] = lastEvent.processed;
    evt["status"] = lastEvent.status;
    evt["uid"] = lastEvent.uid;
    evt["name"] = lastEvent.name;
    evt["amount"] = lastEvent.amount;
    evt["prevBal"] = lastEvent.prevBal;
    evt["remBal"] = lastEvent.remBal;
    evt["message"] = lastEvent.message;

    String response;
    serializeJson(doc, response);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(200, "application/json", response);
  });

  // Acknowledge processed event
  server.on("/api/status/acknowledge-event", HTTP_POST, []() {
    lastEvent.processed = true;
    server.send(200, "text/plain", "OK");
  });

  // Sync client time
  server.on("/api/time", HTTP_POST, []() {
    if (server.hasArg("epoch")) {
      timeSyncEpoch = server.arg("epoch").toInt();
      timeSyncMillis = millis();
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Missing epoch");
    }
  });

  // Get cards
  server.on("/api/cards", HTTP_GET, []() {
    File f = LittleFS.open("/cards.json", "r");
    if (f) {
      server.streamFile(f, "application/json");
      f.close();
    } else {
      server.send(200, "application/json", "[]");
    }
  });

  // Start card registration (enters ADD_CARD mode)
  server.on("/api/cards/add", HTTP_POST, []() {
    activeRegName = server.arg("name");
    activeRegUserId = server.arg("userId");
    activeRegBalance = server.arg("balance").toFloat();
    currentMode = MODE_ADD_CARD;
    readyDisplayed = false;
    server.send(200, "text/plain", "OK");
  });

  // Cancel registration
  server.on("/api/cards/cancel", HTTP_POST, []() {
    currentMode = MODE_READY;
    readyDisplayed = false;
    server.send(200, "text/plain", "OK");
  });

  // Delete card
  server.on("/api/cards/delete", HTTP_POST, []() {
    String uid = server.arg("uid");
    std::vector<Card> cards;
    if (loadCards(cards)) {
      int idx = findCardIndex(cards, uid);
      if (idx != -1) {
        cards.erase(cards.begin() + idx);
        saveCards(cards);
        server.send(200, "text/plain", "OK");
      } else {
        server.send(404, "text/plain", "Card Not Found");
      }
    } else {
      server.send(500, "text/plain", "DB Error");
    }
  });

  // Update card balance
  server.on("/api/cards/update-balance", HTTP_POST, []() {
    String uid = server.arg("uid");
    float balance = server.arg("balance").toFloat();
    std::vector<Card> cards;
    if (loadCards(cards)) {
      int idx = findCardIndex(cards, uid);
      if (idx != -1) {
        cards[idx].balance = balance;
        saveCards(cards);
        server.send(200, "text/plain", "OK");
      } else {
        server.send(404, "text/plain", "Card Not Found");
      }
    } else {
      server.send(500, "text/plain", "DB Error");
    }
  });

  // Start payment (enters PAYMENT mode)
  server.on("/api/payment/start", HTTP_POST, []() {
    activeAmount = server.arg("amount").toFloat();
    currentMode = MODE_PAYMENT;
    readyDisplayed = false;
    server.send(200, "text/plain", "OK");
  });

  // Cancel payment
  server.on("/api/payment/cancel", HTTP_POST, []() {
    currentMode = MODE_READY;
    readyDisplayed = false;
    server.send(200, "text/plain", "OK");
  });

  // Get transactions
  server.on("/api/transactions", HTTP_GET, []() {
    File f = LittleFS.open("/transactions.json", "r");
    if (f) {
      server.streamFile(f, "application/json");
      f.close();
    } else {
      server.send(200, "application/json", "[]");
    }
  });

  // Clear transactions
  server.on("/api/transactions/clear", HTTP_POST, []() {
    File f = LittleFS.open("/transactions.json", "w");
    if (f) {
      f.print("[]");
      f.close();
      pendingSyncCount = 0;
      server.send(200, "text/plain", "OK");
    } else {
      server.send(500, "text/plain", "DB Error");
    }
  });

  // Sync transactions
  server.on("/api/transactions/sync", HTTP_POST, []() {
    std::vector<Transaction> transactions;
    if (loadTransactions(transactions)) {
      for (auto &t : transactions) {
        t.synced = true;
      }
      saveTransactions(transactions);
      pendingSyncCount = 0;
      server.send(200, "text/plain", "OK");
    } else {
      server.send(500, "text/plain", "DB Error");
    }
  });

  // Get settings
  server.on("/api/settings", HTTP_GET, []() {
    File f = LittleFS.open("/settings.json", "r");
    if (f) {
      server.streamFile(f, "application/json");
      f.close();
    } else {
      server.send(200, "application/json", "{}");
    }
  });

  // Save settings
  server.on("/api/settings", HTTP_POST, []() {
    Settings settings;
    settings.stationName = server.arg("stationName");
    if (saveSettings(settings)) {
      server.send(200, "text/plain", "OK");
    } else {
      server.send(500, "text/plain", "Save Fail");
    }
  });

  // Factory reset
  server.on("/api/settings/factory-reset", HTTP_POST, []() {
    clearStorage();
    server.send(200, "text/plain", "Resetting...");
    delay(500);
    ESP.restart();
  });
}

// ── WiFi AP Setup (Robust) ────────────────────────────────────────────
void setupWiFiAP() {
  // Step 1: Completely clean WiFi state
  WiFi.persistent(false);       // Don't save WiFi config to flash
  WiFi.disconnect(true);        // Disconnect any existing connections
  WiFi.mode(WIFI_OFF);          // Turn off WiFi completely first
  delay(100);                   // Let hardware settle

  // Step 2: Set AP mode
  WiFi.mode(WIFI_AP);
  delay(100);                   // Let mode change take effect

  // Step 3: Configure the AP IP address BEFORE starting AP
  IPAddress apIP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(apIP, gateway, subnet);
  delay(50);

  // Step 4: Start the Access Point (open, no password)
  bool apStarted = WiFi.softAP("BondPay", "", 1, false, 4);
  // Channel 1, not hidden, max 4 connections

  if (apStarted) {
    Serial.println("WiFi AP 'BondPay' started successfully!");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("ERROR: WiFi AP failed to start!");
    // Retry once
    delay(1000);
    WiFi.softAP("BondPay", "", 1, false, 4);
    Serial.print("Retry AP IP: ");
    Serial.println(WiFi.softAPIP());
  }

  // Step 5: Wait for AP to be fully ready
  delay(500);

  // Step 6: Start DNS server for captive portal
  // Resolve ALL domain names to our IP (captive portal)
  dnsServer.setTTL(0);  // No caching — always redirect
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", apIP);

  Serial.println("DNS captive portal started.");
}

// ══════════════════════════════════════════════════════════════════════
// SETUP
// ══════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\n==============================");
  Serial.println("  BondPay Terminal v2.0");
  Serial.println("==============================");

  // Setup GPIO pins
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(WIFI_LED, OUTPUT);

  digitalWrite(BUZZER, LOW);
  digitalWrite(LED, LOW);
  digitalWrite(WIFI_LED, HIGH);  // OFF initially (Active Low)

  // Initialize LCD
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Boot splash
  lcd.setCursor(0, 0); lcd.print("BondPay Station");
  lcd.setCursor(0, 1); lcd.print("Starting...");
  delay(1000);

  // ── WiFi AP Setup ─────────────────────────────────────────────────
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Creating WiFi...");
  lcd.setCursor(0, 1); lcd.print("Please wait");

  setupWiFiAP();

  digitalWrite(WIFI_LED, LOW);  // ON (Active Low) — WiFi is ready

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("WiFi: BondPay");
  lcd.setCursor(0, 1); lcd.print("IP: 192.168.4.1");
  delay(2000);

  // ── Storage Init ──────────────────────────────────────────────────
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Loading DB...");

  if (!initStorage()) {
    Serial.println("Storage Initialization Failed!");
    lcd.setCursor(0, 1); lcd.print("DB Error!");
    delay(2000);
  } else {
    Serial.println("Storage initialized OK.");
  }

  // Cache pending sync count at boot
  {
    std::vector<Transaction> txns;
    if (loadTransactions(txns)) {
      pendingSyncCount = 0;
      for (const auto &t : txns) {
        if (!t.synced) pendingSyncCount++;
      }
    }
  }

  // ── RFID Init ─────────────────────────────────────────────────────
  initRFID();
  Serial.println("RFID reader initialized.");

  // ── Web Server ────────────────────────────────────────────────────
  setupRoutes();
  server.begin();
  Serial.println("HTTP server started on port 80.");
  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("BondPay Ready");
  lcd.setCursor(0, 1); lcd.print("Tap Card");
  delay(1000);

  Serial.println("Setup complete. Ready for connections.");
}

// ══════════════════════════════════════════════════════════════════════
// MAIN LOOP
// ══════════════════════════════════════════════════════════════════════
void loop() {
  // Handle DNS for captive portal — MUST be called every loop
  dnsServer.processNextRequest();

  // Handle web server requests
  server.handleClient();

  // Handle D3 Reset Button (3 second hold = factory reset)
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (buttonPressStart == 0) {
      buttonPressStart = millis();
    } else if (millis() - buttonPressStart > 3000) {
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Factory Reset...");
      lcd.setCursor(0, 1); lcd.print("Please Wait");
      delay(1000);
      clearStorage();
      ESP.restart();
    }
  } else {
    buttonPressStart = 0;
  }

  // Maintain LCD screen states
  if (!readyDisplayed) {
    lcd.clear();
    if (currentMode == MODE_READY) {
      lcd.setCursor(0, 0); lcd.print("Tap Card");
      lcd.setCursor(0, 1); lcd.print("BondPay Ready");
    } else if (currentMode == MODE_PAYMENT) {
      lcd.setCursor(0, 0); lcd.print("Waiting Card...");
      char amtStr[17];
      snprintf(amtStr, sizeof(amtStr), "Amt: NPR %.2f", activeAmount);
      lcd.setCursor(0, 1); lcd.print(amtStr);
    } else if (currentMode == MODE_ADD_CARD) {
      lcd.setCursor(0, 0); lcd.print("Scan New Card...");
      lcd.setCursor(0, 1); lcd.print("ID: " + activeRegUserId);
    }
    readyDisplayed = true;
  }

  // Scan cooldown
  if (millis() - lastScanTime < COOLDOWN_DELAY) {
    return;
  }

  // Attempt RFID read
  String uid;
  if (!readRFID(uid)) {
    return;
  }

  Serial.println("Scanned card UID: " + uid);

  // ── Handle scan based on current mode ─────────────────────────────
  if (currentMode == MODE_READY) {
    // Quick Balance Check
    std::vector<Card> cards;
    if (loadCards(cards)) {
      int idx = findCardIndex(cards, uid);
      if (idx != -1) {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print(cards[idx].name);
        char balStr[17];
        snprintf(balStr, sizeof(balStr), "Bal: NPR %.2f", cards[idx].balance);
        lcd.setCursor(0, 1); lcd.print(balStr);
        triggerSuccessFeedback();
      } else {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Unregistered");
        lcd.setCursor(0, 1); lcd.print("UID: " + uid);
        triggerFailureFeedback();
      }
    }
    delay(2000);
    readyDisplayed = false;
    lastScanTime = millis();

  } else if (currentMode == MODE_PAYMENT) {
    // Process payment
    String errorMsg;
    float newBal = 0.0f;
    String name;
    bool success = processPayment(uid, activeAmount, errorMsg, newBal, name);

    lastEvent.processed = false;
    lastEvent.uid = uid;
    lastEvent.amount = activeAmount;

    if (success) {
      lastEvent.status = "Success";
      lastEvent.name = name;
      lastEvent.remBal = newBal;
      lastEvent.prevBal = newBal + activeAmount;
      lastEvent.message = "Payment Successful";

      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Payment Success");
      char sLine[17];
      snprintf(sLine, sizeof(sLine), "Amt:%.0f Bal:%.0f", activeAmount, newBal);
      lcd.setCursor(0, 1); lcd.print(sLine);
      triggerSuccessFeedback();
    } else {
      lastEvent.status = "Failed";
      lastEvent.message = errorMsg;

      lcd.clear();
      if (errorMsg.equalsIgnoreCase("Insufficient Bal")) {
        lcd.setCursor(0, 0); lcd.print("Insufficient");
        lcd.setCursor(0, 1); lcd.print("Balance");
      } else {
        lcd.setCursor(0, 0); lcd.print("Payment Failed");
        lcd.setCursor(0, 1); lcd.print(errorMsg);
      }
      triggerFailureFeedback();
    }

    delay(2000);
    currentMode = MODE_READY;
    readyDisplayed = false;
    lastScanTime = millis();

  } else if (currentMode == MODE_ADD_CARD) {
    // Register card
    String errorMsg;
    bool success = registerCard(uid, activeRegName, activeRegUserId, activeRegBalance, errorMsg);

    lastEvent.processed = false;
    lastEvent.uid = uid;
    lastEvent.name = activeRegName;
    lastEvent.amount = 0.0f;

    if (success) {
      lastEvent.status = "Success";
      lastEvent.prevBal = activeRegBalance;
      lastEvent.remBal = activeRegBalance;
      lastEvent.message = "Card Registered";

      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Card Registered");
      lcd.setCursor(0, 1); lcd.print(activeRegName);
      triggerSuccessFeedback();
    } else {
      lastEvent.status = "Failed";
      lastEvent.message = errorMsg;

      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Register Failed");
      lcd.setCursor(0, 1); lcd.print(errorMsg);
      triggerFailureFeedback();
    }

    delay(2000);
    currentMode = MODE_READY;
    readyDisplayed = false;
    lastScanTime = millis();
  }
}
