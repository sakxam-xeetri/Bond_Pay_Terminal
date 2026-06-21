#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <vector>

// Pins mapping
#define RST_PIN    255
#define SS_PIN     2    // D4 (GPIO2)
#define BUZZER     16   // D0 (GPIO16)
#define LED        15   // D8 (GPIO15)
#define BUTTON_PIN 0    // D3 (GPIO0)
#define WIFI_LED   LED_BUILTIN

// Instantiate hardware
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN);
ESP8266WebServer server(80);

// Global settings & states
enum StationMode {
  MODE_READY,
  MODE_PAYMENT,
  MODE_ADD_CARD
};

StationMode currentMode = MODE_READY;

// Setup global clock variables used in payment.h
uint32_t timeSyncEpoch = 0;
unsigned long timeSyncMillis = 0;

// Mode variables
float activeAmount = 0.0f;
String activeRegName = "";
String activeRegUserId = "";
float activeRegBalance = 0.0f;

// Event logger for UI polling
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

// Cooldown and LCD ready display states
unsigned long lastScanTime = 0;
const unsigned long COOLDOWN_DELAY = 1500;
bool readyDisplayed = false;
unsigned long buttonPressStart = 0;

// Include our custom modules
#include "storage.h"
#include "rfid.h"
#include "payment.h"
#include "web.h"

// Define REST API routes
void setupRoutes() {
  // Main page HTML
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", MAIN_PAGE);
  });

  // API Status & Heartbeat
  server.on("/api/status", HTTP_GET, []() {
    ALLOCATE_JSON_DOCUMENT(doc, 512);
    doc["mode"] = currentMode == MODE_READY ? "READY" : (currentMode == MODE_PAYMENT ? "PAYMENT" : "ADD_CARD");
    doc["activeAmount"] = activeAmount;
    doc["activeUserId"] = activeRegUserId;
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["uptime"] = millis();

    std::vector<Transaction> transactions;
    loadTransactions(transactions);
    int pending = 0;
    for (const auto &t : transactions) {
      if (!t.synced) pending++;
    }
    doc["pendingSyncCount"] = pending;

    JsonObject evt = doc.createNestedObject("lastEvent");
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

  // Get Card database
  server.on("/api/cards", HTTP_GET, []() {
    File f = LittleFS.open("/cards.json", "r");
    if (f) {
      server.streamFile(f, "application/json");
      f.close();
    } else {
      server.send(500, "application/json", "[]");
    }
  });

  // Put terminal in Registration Mode
  server.on("/api/cards/add", HTTP_POST, []() {
    activeRegName = server.arg("name");
    activeRegUserId = server.arg("userId");
    activeRegBalance = server.arg("balance").toFloat();

    currentMode = MODE_ADD_CARD;
    readyDisplayed = false;
    server.send(200, "text/plain", "OK");
  });

  // Cancel registration mode
  server.on("/api/cards/cancel", HTTP_POST, []() {
    currentMode = MODE_READY;
    readyDisplayed = false;
    server.send(200, "text/plain", "OK");
  });

  // Delete Card
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

  // Put terminal in Payment Mode
  server.on("/api/payment/start", HTTP_POST, []() {
    activeAmount = server.arg("amount").toFloat();
    currentMode = MODE_PAYMENT;
    readyDisplayed = false;
    server.send(200, "text/plain", "OK");
  });

  // Cancel payment mode
  server.on("/api/payment/cancel", HTTP_POST, []() {
    currentMode = MODE_READY;
    readyDisplayed = false;
    server.send(200, "text/plain", "OK");
  });

  // Get Transaction Logs
  server.on("/api/transactions", HTTP_GET, []() {
    File f = LittleFS.open("/transactions.json", "r");
    if (f) {
      server.streamFile(f, "application/json");
      f.close();
    } else {
      server.send(500, "application/json", "[]");
    }
  });

  // Clear Transaction Logs
  server.on("/api/transactions/clear", HTTP_POST, []() {
    File f = LittleFS.open("/transactions.json", "w");
    if (f) {
      f.print("[]");
      f.close();
      server.send(200, "text/plain", "OK");
    } else {
      server.send(500, "text/plain", "DB Error");
    }
  });

  // Simulated transaction sync
  server.on("/api/transactions/sync", HTTP_POST, []() {
    std::vector<Transaction> transactions;
    if (loadTransactions(transactions)) {
      for (auto &t : transactions) {
        t.synced = true;
      }
      saveTransactions(transactions);
      server.send(200, "text/plain", "OK");
    } else {
      server.send(500, "text/plain", "DB Error");
    }
  });

  // Get Settings
  server.on("/api/settings", HTTP_GET, []() {
    File f = LittleFS.open("/settings.json", "r");
    if (f) {
      server.streamFile(f, "application/json");
      f.close();
    } else {
      server.send(500, "application/json", "{}");
    }
  });

  // Save Settings
  server.on("/api/settings", HTTP_POST, []() {
    Settings settings;
    settings.stationName = server.arg("stationName");
    if (saveSettings(settings)) {
      server.send(200, "text/plain", "OK");
    } else {
      server.send(500, "text/plain", "Save Fail");
    }
  });

  // Factory Reset Station
  server.on("/api/settings/factory-reset", HTTP_POST, []() {
    clearStorage();
    server.send(200, "text/plain", "Resetting...");
    delay(500);
    ESP.restart();
  });
}

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println("\nBondPay Terminal Starting...");

  // Setup GPIO pins
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(WIFI_LED, OUTPUT);

  digitalWrite(BUZZER, LOW);
  digitalWrite(LED, LOW);
  digitalWrite(WIFI_LED, HIGH); // OFF initially (Active Low)

  // Initialize LCD
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Boot screens
  lcd.setCursor(0, 0); lcd.print("BondPay Station");
  lcd.setCursor(0, 1); lcd.print("Starting...");
  delay(1500);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Creating WiFi");
  delay(500);

  // Configure Soft Access Point
  IPAddress local_IP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP("BondPay", "12345678");

  digitalWrite(WIFI_LED, LOW); // ON (Active Low)

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("BondPay Ready");
  lcd.setCursor(0, 1); lcd.print("192.168.4.1");
  delay(2000);

  // Initialize local file storage
  if (!initStorage()) {
    Serial.println("Storage Initialization Failed!");
  }

  // Initialize MFRC522 RFID
  initRFID();

  // Configure Web Server Routes
  setupRoutes();
  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {
  // Handle web server requests
  server.handleClient();

  // Handle D3 Reset Button (3 seconds hold triggers factory reset)
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (buttonPressStart == 0) {
      buttonPressStart = millis();
    } else if (millis() - buttonPressStart > 3000) {
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Factory Resetting");
      lcd.setCursor(0, 1); lcd.print("Please Wait...");
      delay(1000);
      clearStorage();
      ESP.restart();
    }
  } else {
    buttonPressStart = 0;
  }

  // Maintain screen states
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

  // Scan cooldown validation
  if (millis() - lastScanTime < COOLDOWN_DELAY) {
    return;
  }

  // Attempt reading RFID
  String uid;
  if (!readRFID(uid)) {
    return;
  }

  Serial.println("Scanned card UID: " + uid);

  // Handle scans based on current Mode
  if (currentMode == MODE_READY) {
    // Quick Balance Check Mode
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

    // Save event for front-end polling
    lastEvent.processed = false;
    lastEvent.uid = uid;
    lastEvent.amount = activeAmount;

    if (success) {
      lastEvent.status = "Success";
      lastEvent.name = name;
      lastEvent.remBal = newBal;
      lastEvent.prevBal = newBal + activeAmount;
      lastEvent.message = "Payment Successful";

      // Show Payment Success Screen on LCD
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Payment Success");
      
      char successLine2[17];
      snprintf(successLine2, sizeof(successLine2), "Amt:%.0f Bal:%.0f", activeAmount, newBal);
      lcd.setCursor(0, 1); lcd.print(successLine2);

      triggerSuccessFeedback();
    } else {
      lastEvent.status = "Failed";
      lastEvent.message = errorMsg;

      // Show Payment Failed Screen on LCD
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
    // Register Card
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
