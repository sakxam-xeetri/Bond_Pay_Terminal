#ifndef RFID_H
#define RFID_H

#include <SPI.h>
#include <MFRC522.h>

extern MFRC522 mfrc522;

inline String rfidByteToHex(byte val) {
  char buf[3];
  sprintf(buf, "%02X", val);
  return String(buf);
}

inline void initRFID() {
  SPI.begin();
  mfrc522.PCD_Init();
}

inline bool readRFID(String &uidOut) {
  if (!mfrc522.PICC_IsNewCardPresent()) return false;
  if (!mfrc522.PICC_ReadCardSerial()) return false;
  
  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidString += rfidByteToHex(mfrc522.uid.uidByte[i]);
    if (i < mfrc522.uid.size - 1) uidString += ":";
  }
  uidString.toUpperCase();
  uidOut = uidString;
  
  // Halt PICC
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  return true;
}

#endif
