#ifndef PAYMENT_H
#define PAYMENT_H

#include "storage.h"
#include <Arduino.h>

extern const int BUZZER;
extern const int LED;

// Global clock syncing variables
extern uint32_t timeSyncEpoch;
extern unsigned long timeSyncMillis;

inline uint32_t getEpochTime() {
  if (timeSyncEpoch == 0) return 0;
  return timeSyncEpoch + (millis() - timeSyncMillis) / 1000;
}

inline String formatEpochTime(uint32_t epoch) {
  if (epoch == 0) return "Pending Sync Time";
  
  uint32_t rawTime = epoch;
  uint8_t second = rawTime % 60;
  rawTime /= 60;
  uint8_t minute = rawTime % 60;
  rawTime /= 60;
  uint8_t hour = rawTime % 24;
  rawTime /= 24;
  
  uint32_t days = rawTime;
  uint32_t year = 1970;
  while (true) {
    bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
    uint16_t daysInYear = leap ? 366 : 365;
    if (days >= daysInYear) {
      days -= daysInYear;
      year++;
    } else {
      break;
    }
  }
  
  uint8_t month = 1;
  uint8_t daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
    daysInMonth[1] = 29;
  }
  
  for (int i = 0; i < 12; i++) {
    if (days >= daysInMonth[i]) {
      days -= daysInMonth[i];
      month++;
    } else {
      break;
    }
  }
  uint8_t day = days + 1;
  
  char buf[20];
  sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
  return String(buf);
}

// Find a card by UID
inline int findCardIndex(const std::vector<Card> &cards, const String &uid) {
  for (size_t i = 0; i < cards.size(); i++) {
    if (cards[i].uid.equalsIgnoreCase(uid)) {
      return (int)i;
    }
  }
  return -1;
}

// Success feedback: Green LED Flash + Double Beep
inline void triggerSuccessFeedback() {
  digitalWrite(LED, HIGH);
  tone(BUZZER, 1200, 100);
  delay(120);
  tone(BUZZER, 1200, 100);
  delay(120);
  digitalWrite(LED, LOW);
}

// Failed feedback: Red/Status LED Flash + Long Error Beep
inline void triggerFailureFeedback() {
  digitalWrite(LED, HIGH);
  tone(BUZZER, 500, 500);
  delay(550);
  digitalWrite(LED, LOW);
}

// Log a transaction (keeping last 100 elements to avoid OOM)
inline void logTransaction(const String &uid, const String &name, float amount, float prevBal, float remBal, const String &status) {
  std::vector<Transaction> transactions;
  loadTransactions(transactions);
  
  Transaction t;
  t.id = 1;
  if (!transactions.empty()) {
    t.id = transactions.back().id + 1;
  }
  t.timestamp = formatEpochTime(getEpochTime());
  t.uid = uid;
  t.name = name;
  t.amount = amount;
  t.prevBal = prevBal;
  t.remBal = remBal;
  t.status = status;
  t.synced = false;
  
  transactions.push_back(t);
  
  if (transactions.size() > 100) {
    transactions.erase(transactions.begin());
  }
  
  saveTransactions(transactions);
}

// Process a payment
inline bool processPayment(const String &uid, float amount, String &errorMessage, float &newBalance, String &cardholderName) {
  if (amount <= 0.0f) {
    errorMessage = "Invalid Amount";
    return false;
  }

  std::vector<Card> cards;
  if (!loadCards(cards)) {
    errorMessage = "DB Error";
    return false;
  }

  int index = findCardIndex(cards, uid);
  if (index == -1) {
    errorMessage = "Card Not Found";
    logTransaction(uid, "Unknown", amount, 0.0f, 0.0f, "Failed (Not Found)");
    return false;
  }

  cardholderName = cards[index].name;
  float prevBal = cards[index].balance;

  if (prevBal < amount) {
    errorMessage = "Insufficient Bal";
    logTransaction(uid, cardholderName, amount, prevBal, prevBal, "Failed (Insufficient)");
    return false;
  }

  // Deduct
  cards[index].balance -= amount;
  newBalance = cards[index].balance;
  
  if (!saveCards(cards)) {
    errorMessage = "DB Save Fail";
    return false;
  }

  logTransaction(uid, cardholderName, amount, prevBal, newBalance, "Success");
  return true;
}

// Register a card
inline bool registerCard(const String &uid, const String &name, const String &userId, float balance, String &errorMessage) {
  if (name.length() == 0 || userId.length() == 0) {
    errorMessage = "Name & ID Required";
    return false;
  }
  if (balance < 0.0f) {
    errorMessage = "Negative Balance";
    return false;
  }
  
  std::vector<Card> cards;
  if (!loadCards(cards)) {
    errorMessage = "DB Error";
    return false;
  }
  
  if (findCardIndex(cards, uid) != -1) {
    errorMessage = "Duplicate Card";
    return false;
  }
  
  Card c;
  c.uid = uid;
  c.name = name;
  c.userId = userId;
  c.balance = balance;
  
  cards.push_back(c);
  if (!saveCards(cards)) {
    errorMessage = "DB Save Fail";
    return false;
  }
  
  return true;
}

#endif
