#ifndef STORAGE_H
#define STORAGE_H

#include <LittleFS.h>
#include <ArduinoJson.h>
#include <vector>

// Version compatibility macros for ArduinoJson
#if ARDUINOJSON_VERSION_MAJOR >= 7
  #define ALLOCATE_JSON_DOCUMENT(doc, size) JsonDocument doc
  #define CREATE_NESTED_OBJECT(parent, key) parent[key].to<JsonObject>()
  #define ADD_NESTED_OBJECT(parent) parent.add<JsonObject>()
#else
  #define ALLOCATE_JSON_DOCUMENT(doc, size) DynamicJsonDocument doc(size)
  #define CREATE_NESTED_OBJECT(parent, key) parent.createNestedObject(key)
  #define ADD_NESTED_OBJECT(parent) parent.createNestedObject()
#endif

struct Card {
  String uid;
  String name;
  String userId;
  float balance;
};

struct Transaction {
  int id;
  String timestamp;
  String uid;
  String name;
  float amount;
  float prevBal;
  float remBal;
  String status;
  bool synced;
};

struct Settings {
  String stationName;
};

// RAM cache variables
inline std::vector<Card> ramCards;
inline std::vector<Transaction> ramTransactions;
inline Settings ramSettings;
inline String cachedCardsJson = "[]";
inline String cachedTransactionsJson = "[]";

inline void updateCachedCardsJson() {
  ALLOCATE_JSON_DOCUMENT(doc, 8192);
  JsonArray arr = doc.to<JsonArray>();
  for (const auto &c : ramCards) {
    JsonObject obj = ADD_NESTED_OBJECT(arr);
    obj["uid"] = c.uid;
    obj["name"] = c.name;
    obj["userId"] = c.userId;
    obj["balance"] = c.balance;
  }
  cachedCardsJson = "";
  serializeJson(doc, cachedCardsJson);
}

inline void updateCachedTransactionsJson() {
  ALLOCATE_JSON_DOCUMENT(doc, 16384);
  JsonArray arr = doc.to<JsonArray>();
  for (const auto &t : ramTransactions) {
    JsonObject obj = ADD_NESTED_OBJECT(arr);
    obj["id"] = t.id;
    obj["timestamp"] = t.timestamp;
    obj["uid"] = t.uid;
    obj["name"] = t.name;
    obj["amount"] = t.amount;
    obj["prevBal"] = t.prevBal;
    obj["remBal"] = t.remBal;
    obj["status"] = t.status;
    obj["synced"] = t.synced;
  }
  cachedTransactionsJson = "";
  serializeJson(doc, cachedTransactionsJson);
}

inline bool loadCardsFromDisk() {
  ramCards.clear();
  File f = LittleFS.open("/cards.json", "r");
  if (!f) return false;
  
  ALLOCATE_JSON_DOCUMENT(doc, 8192);
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  
  if (err) {
    Serial.println("loadCardsFromDisk: JSON Deserialization error!");
    return false;
  }
  
  JsonArray arr = doc.as<JsonArray>();
  for (JsonObject obj : arr) {
    Card c;
    c.uid = obj["uid"].as<String>();
    c.name = obj["name"].as<String>();
    c.userId = obj["userId"].as<String>();
    c.balance = obj["balance"].as<float>();
    ramCards.push_back(c);
  }
  updateCachedCardsJson();
  return true;
}

inline bool loadTransactionsFromDisk() {
  ramTransactions.clear();
  File f = LittleFS.open("/transactions.json", "r");
  if (!f) return false;
  
  ALLOCATE_JSON_DOCUMENT(doc, 16384);
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  
  if (err) {
    Serial.println("loadTransactionsFromDisk: JSON Deserialization error!");
    return false;
  }
  
  JsonArray arr = doc.as<JsonArray>();
  for (JsonObject obj : arr) {
    Transaction t;
    t.id = obj["id"].as<int>();
    t.timestamp = obj["timestamp"].as<String>();
    t.uid = obj["uid"].as<String>();
    t.name = obj["name"].as<String>();
    t.amount = obj["amount"].as<float>();
    t.prevBal = obj["prevBal"].as<float>();
    t.remBal = obj["remBal"].as<float>();
    t.status = obj["status"].as<String>();
    t.synced = obj["synced"].as<bool>();
    ramTransactions.push_back(t);
  }
  updateCachedTransactionsJson();
  return true;
}

inline bool initStorage() {
  if (!LittleFS.begin()) {
    Serial.println("LittleFS Mount Failed! Formatting...");
    LittleFS.format();
    if (!LittleFS.begin()) {
      Serial.println("LittleFS Mount Failed even after formatting!");
      return false;
    }
  }
  
  // Create default files if they don't exist
  if (!LittleFS.exists("/cards.json")) {
    File f = LittleFS.open("/cards.json", "w");
    if (f) {
      f.print("[]");
      f.close();
    }
  }
  
  if (!LittleFS.exists("/transactions.json")) {
    File f = LittleFS.open("/transactions.json", "w");
    if (f) {
      f.print("[]");
      f.close();
    }
  }
  
  if (!LittleFS.exists("/settings.json")) {
    File f = LittleFS.open("/settings.json", "w");
    if (f) {
      f.print("{\"stationName\":\"BondPay Station 1\"}");
      f.close();
    }
  }
  
  // Load data into RAM
  loadCardsFromDisk();
  loadTransactionsFromDisk();
  
  File f = LittleFS.open("/settings.json", "r");
  if (f) {
    ALLOCATE_JSON_DOCUMENT(doc, 256);
    if (deserializeJson(doc, f) == DeserializationError::Ok) {
      ramSettings.stationName = doc["stationName"] | "BondPay Station 1";
    } else {
      ramSettings.stationName = "BondPay Station 1";
    }
    f.close();
  } else {
    ramSettings.stationName = "BondPay Station 1";
  }
  
  return true;
}

inline bool loadSettings(Settings &settings) {
  settings = ramSettings;
  return true;
}

inline bool saveSettings(const Settings &settings) {
  ramSettings = settings;
  File f = LittleFS.open("/settings.json", "w");
  if (!f) return false;
  
  ALLOCATE_JSON_DOCUMENT(doc, 256);
  doc["stationName"] = settings.stationName;
  
  size_t bytesWritten = serializeJson(doc, f);
  f.close();
  return bytesWritten > 0;
}

inline bool loadCards(std::vector<Card> &cards) {
  cards = ramCards;
  return true;
}

inline bool saveCards(const std::vector<Card> &cards) {
  ramCards = cards;
  updateCachedCardsJson();
  
  File f = LittleFS.open("/cards.json", "w");
  if (!f) return false;
  
  ALLOCATE_JSON_DOCUMENT(doc, 8192);
  JsonArray arr = doc.to<JsonArray>();
  for (const auto &c : cards) {
    JsonObject obj = ADD_NESTED_OBJECT(arr);
    obj["uid"] = c.uid;
    obj["name"] = c.name;
    obj["userId"] = c.userId;
    obj["balance"] = c.balance;
  }
  
  size_t bytesWritten = serializeJson(doc, f);
  f.close();
  return bytesWritten > 0;
}

inline bool loadTransactions(std::vector<Transaction> &transactions) {
  transactions = ramTransactions;
  return true;
}

inline bool saveTransactions(const std::vector<Transaction> &transactions) {
  ramTransactions = transactions;
  updateCachedTransactionsJson();
  
  File f = LittleFS.open("/transactions.json", "w");
  if (!f) return false;
  
  ALLOCATE_JSON_DOCUMENT(doc, 16384);
  JsonArray arr = doc.to<JsonArray>();
  for (const auto &t : transactions) {
    JsonObject obj = ADD_NESTED_OBJECT(arr);
    obj["id"] = t.id;
    obj["timestamp"] = t.timestamp;
    obj["uid"] = t.uid;
    obj["name"] = t.name;
    obj["amount"] = t.amount;
    obj["prevBal"] = t.prevBal;
    obj["remBal"] = t.remBal;
    obj["status"] = t.status;
    obj["synced"] = t.synced;
  }
  
  size_t bytesWritten = serializeJson(doc, f);
  f.close();
  return bytesWritten > 0;
}

inline void clearStorage() {
  LittleFS.remove("/cards.json");
  LittleFS.remove("/transactions.json");
  LittleFS.remove("/settings.json");
  ramCards.clear();
  ramTransactions.clear();
  ramSettings.stationName = "BondPay Station 1";
  updateCachedCardsJson();
  updateCachedTransactionsJson();
  initStorage();
}

#endif
