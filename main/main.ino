#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <vector>

// ══════════════════════════════════════════════════════════════════════
// ── HARDCODED WIFI CREDENTIALS ───────────────────────────────────────
// Change these to your WiFi network name and password
// ══════════════════════════════════════════════════════════════════════
const char* WIFI_SSID     = "SUNIMA 9706";
const char* WIFI_PASSWORD = "r1:705F7";

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

// ── Global State ─────────────────────────────────────────────────────
enum StationMode {
  MODE_READY,
  MODE_PAYMENT,
  MODE_ADD_CARD
};

StationMode currentMode = MODE_READY;

// Clock sync
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

// Scan cooldown
unsigned long lastScanTime = 0;
const unsigned long COOLDOWN_DELAY = 1500;
bool readyDisplayed = false;
unsigned long buttonPressStart = 0;

// Cached pending sync count
int pendingSyncCount = 0;

// Store assigned IP for display
String deviceIP = "Connecting...";

// ── Include Custom Modules ───────────────────────────────────────────
#include "storage.h"
#include "rfid.h"
#include "payment.h"
#include "web.h"

// ══════════════════════════════════════════════════════════════════════
// ── WiFi STA Connection (connects to your router) ────────────────────
// ══════════════════════════════════════════════════════════════════════
void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Connecting WiFi");
  lcd.setCursor(0, 1); lcd.print(WIFI_SSID);

  WiFi.persistent(false);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Wait up to 20 seconds for connection
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;

    // Blink WiFi LED while connecting
    digitalWrite(WIFI_LED, (attempts % 2 == 0) ? HIGH : LOW);
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    deviceIP = WiFi.localIP().toString();
    Serial.println("WiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(deviceIP);

    digitalWrite(WIFI_LED, LOW);  // ON (Active Low)

    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("WiFi Connected!");
    lcd.setCursor(0, 1); lcd.print(deviceIP);
    delay(2000);
  } else {
    Serial.println("WiFi connection FAILED!");
    Serial.println("Check SSID and password.");

    digitalWrite(WIFI_LED, HIGH);  // OFF

    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("WiFi FAILED!");
    lcd.setCursor(0, 1); lcd.print("Check Config");
    delay(3000);

    // Still start server — user might connect later
    deviceIP = "No Connection";
  }
}

// ══════════════════════════════════════════════════════════════════════
// ── API ROUTES (JSON only, no HTML) ──────────────────────────────────
// ══════════════════════════════════════════════════════════════════════
void setupRoutes() {

  // ── Root — just returns device info ───────────────────────────────
  server.on("/", HTTP_GET, []() {
    sendCORS(server);
    server.send(200, "application/json", "{\"device\":\"BondPay Terminal\",\"version\":\"2.0\",\"status\":\"online\"}");
  });

  // ── CORS preflight for all /api/* routes ──────────────────────────
  server.onNotFound([]() {
    if (server.method() == HTTP_OPTIONS) {
      handleCORSPreflight(server);
      return;
    }
    sendCORS(server);
    server.send(404, "application/json", "{\"error\":\"Not Found\"}");
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
    doc["ip"] = deviceIP;

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
    sendCORS(server);
    server.send(200, "application/json", response);
  });

  // Acknowledge processed event
  server.on("/api/status/acknowledge-event", HTTP_POST, []() {
    lastEvent.processed = true;
    sendCORS(server);
    server.send(200, "application/json", "{\"ok\":true}");
  });

  // Sync client time
  server.on("/api/time", HTTP_POST, []() {
    sendCORS(server);
    if (server.hasArg("epoch")) {
      timeSyncEpoch = server.arg("epoch").toInt();
      timeSyncMillis = millis();
      server.send(200, "application/json", "{\"ok\":true}");
    } else {
      server.send(400, "application/json", "{\"error\":\"Missing epoch\"}");
    }
  });

  // Get cards
  server.on("/api/cards", HTTP_GET, []() {
    sendCORS(server);
    File f = LittleFS.open("/cards.json", "r");
    if (f) {
      String content = f.readString();
      f.close();
      server.send(200, "application/json", content);
    } else {
      server.send(200, "application/json", "[]");
    }
  });

  // Start card registration
  server.on("/api/cards/add", HTTP_POST, []() {
    sendCORS(server);
    activeRegName = server.arg("name");
    activeRegUserId = server.arg("userId");
    activeRegBalance = server.arg("balance").toFloat();
    currentMode = MODE_ADD_CARD;
    readyDisplayed = false;
    server.send(200, "application/json", "{\"ok\":true}");
  });

  // Cancel registration
  server.on("/api/cards/cancel", HTTP_POST, []() {
    sendCORS(server);
    currentMode = MODE_READY;
    readyDisplayed = false;
    server.send(200, "application/json", "{\"ok\":true}");
  });

  // Delete card
  server.on("/api/cards/delete", HTTP_POST, []() {
    sendCORS(server);
    String uid = server.arg("uid");
    std::vector<Card> cards;
    if (loadCards(cards)) {
      int idx = findCardIndex(cards, uid);
      if (idx != -1) {
        cards.erase(cards.begin() + idx);
        saveCards(cards);
        server.send(200, "application/json", "{\"ok\":true}");
      } else {
        server.send(404, "application/json", "{\"error\":\"Card Not Found\"}");
      }
    } else {
      server.send(500, "application/json", "{\"error\":\"DB Error\"}");
    }
  });

  // Update card balance
  server.on("/api/cards/update-balance", HTTP_POST, []() {
    sendCORS(server);
    String uid = server.arg("uid");
    float balance = server.arg("balance").toFloat();
    std::vector<Card> cards;
    if (loadCards(cards)) {
      int idx = findCardIndex(cards, uid);
      if (idx != -1) {
        cards[idx].balance = balance;
        saveCards(cards);
        server.send(200, "application/json", "{\"ok\":true}");
      } else {
        server.send(404, "application/json", "{\"error\":\"Card Not Found\"}");
      }
    } else {
      server.send(500, "application/json", "{\"error\":\"DB Error\"}");
    }
  });

  // Start payment
  server.on("/api/payment/start", HTTP_POST, []() {
    sendCORS(server);
    activeAmount = server.arg("amount").toFloat();
    currentMode = MODE_PAYMENT;
    readyDisplayed = false;
    server.send(200, "application/json", "{\"ok\":true}");
  });

  // Cancel payment
  server.on("/api/payment/cancel", HTTP_POST, []() {
    sendCORS(server);
    currentMode = MODE_READY;
    readyDisplayed = false;
    server.send(200, "application/json", "{\"ok\":true}");
  });

  // Get transactions
  server.on("/api/transactions", HTTP_GET, []() {
    sendCORS(server);
    File f = LittleFS.open("/transactions.json", "r");
    if (f) {
      String content = f.readString();
      f.close();
      server.send(200, "application/json", content);
    } else {
      server.send(200, "application/json", "[]");
    }
  });

  // Clear transactions
  server.on("/api/transactions/clear", HTTP_POST, []() {
    sendCORS(server);
    File f = LittleFS.open("/transactions.json", "w");
    if (f) {
      f.print("[]");
      f.close();
      pendingSyncCount = 0;
      server.send(200, "application/json", "{\"ok\":true}");
    } else {
      server.send(500, "application/json", "{\"error\":\"DB Error\"}");
    }
  });

  // Sync transactions
  server.on("/api/transactions/sync", HTTP_POST, []() {
    sendCORS(server);
    std::vector<Transaction> transactions;
    if (loadTransactions(transactions)) {
      for (auto &t : transactions) {
        t.synced = true;
      }
      saveTransactions(transactions);
      pendingSyncCount = 0;
      server.send(200, "application/json", "{\"ok\":true}");
    } else {
      server.send(500, "application/json", "{\"error\":\"DB Error\"}");
    }
  });

  // Get settings
  server.on("/api/settings", HTTP_GET, []() {
    sendCORS(server);
    File f = LittleFS.open("/settings.json", "r");
    if (f) {
      String content = f.readString();
      f.close();
      server.send(200, "application/json", content);
    } else {
      server.send(200, "application/json", "{}");
    }
  });

  // Save settings
  server.on("/api/settings", HTTP_POST, []() {
    sendCORS(server);
    Settings settings;
    settings.stationName = server.arg("stationName");
    if (saveSettings(settings)) {
      server.send(200, "application/json", "{\"ok\":true}");
    } else {
      server.send(500, "application/json", "{\"error\":\"Save Failed\"}");
    }
  });

  // Factory reset
  server.on("/api/settings/factory-reset", HTTP_POST, []() {
    sendCORS(server);
    clearStorage();
    server.send(200, "application/json", "{\"ok\":true,\"message\":\"Resetting...\"}");
    delay(500);
    ESP.restart();
  });
}

// ══════════════════════════════════════════════════════════════════════
// SETUP
// ══════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\n==============================");
  Serial.println("  BondPay Terminal v2.0");
  Serial.println("  API-Only Mode (WiFi STA)");
  Serial.println("==============================");

  // Setup GPIO
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(WIFI_LED, OUTPUT);

  digitalWrite(BUZZER, LOW);
  digitalWrite(LED, LOW);
  digitalWrite(WIFI_LED, HIGH);  // OFF initially

  // Initialize LCD
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Boot splash
  lcd.setCursor(0, 0); lcd.print("BondPay Station");
  lcd.setCursor(0, 1); lcd.print("v2.0 Starting...");
  delay(1000);

  // ── Connect to WiFi (STA mode) ────────────────────────────────────
  connectToWiFi();

  // ── Storage Init ──────────────────────────────────────────────────
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Loading DB...");

  if (!initStorage()) {
    Serial.println("Storage Initialization Failed!");
    lcd.setCursor(0, 1); lcd.print("DB Error!");
    delay(2000);
  } else {
    Serial.println("Storage initialized.");
  }

  // Cache pending sync count
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

  // ── Start API Server ──────────────────────────────────────────────
  setupRoutes();
  server.begin();

  Serial.println("API server started on port 80.");
  Serial.print("Access API at: http://");
  Serial.println(deviceIP);
  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("BondPay Ready");
  lcd.setCursor(0, 1); lcd.print(deviceIP);

  Serial.println("Setup complete.");
}

// ══════════════════════════════════════════════════════════════════════
// MAIN LOOP
// ══════════════════════════════════════════════════════════════════════
void loop() {
  // Handle API requests
  server.handleClient();

  // Auto-reconnect WiFi if disconnected
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastReconnect = 0;
    if (millis() - lastReconnect > 10000) {  // Try every 10 seconds
      lastReconnect = millis();
      Serial.println("WiFi lost. Reconnecting...");
      WiFi.reconnect();
    }
  }

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
      lcd.setCursor(0, 1); lcd.print(deviceIP);
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
