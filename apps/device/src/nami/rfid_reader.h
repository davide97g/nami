#ifndef RFID_READER_H
#define RFID_READER_H

#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_SSD1306.h>

// RFID Reader Pin Configuration
#define RFID_RST_PIN  4   // Reset pin for MFRC522 (changed from D2 to avoid built-in LED conflict)
#define RFID_SS_PIN   5   // Slave Select (SDA) pin for MFRC522

// SPI pins for ESP32 (default VSPI)
// MOSI: GPIO 23
// MISO: GPIO 19
// SCK:  GPIO 18

// Create MFRC522 instance
MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);

/**
 * Initialize the RFID reader module with detailed diagnostics
 * @return true if initialization successful, false otherwise
 */
bool initRFID() {
  Serial.println("[RFID] Starting initialization...");
  Serial.print("[RFID] Pin configuration: RST=D");
  Serial.print(RFID_RST_PIN);
  Serial.print(" (GPIO ");
  Serial.print(RFID_RST_PIN);
  Serial.print("), SS=D");
  Serial.print(RFID_SS_PIN);
  Serial.print(" (GPIO ");
  Serial.print(RFID_SS_PIN);
  Serial.println(")");
  Serial.println("[RFID] SPI pins: MOSI=D23, MISO=D19, SCK=D18");
  
  // Configure SS pin as output and set it HIGH (inactive)
  pinMode(RFID_SS_PIN, OUTPUT);
  digitalWrite(RFID_SS_PIN, HIGH);
  
  // Configure RST pin as output
  pinMode(RFID_RST_PIN, OUTPUT);
  
  // Perform hardware reset
  Serial.println("[RFID] Performing hardware reset...");
  digitalWrite(RFID_RST_PIN, LOW);
  delay(10);
  digitalWrite(RFID_RST_PIN, HIGH);
  delay(10);
  
  // Initialize SPI bus
  Serial.println("[RFID] Initializing SPI bus...");
  SPI.begin();
  delay(50); // Give module time to stabilize after reset
  
  // Initialize MFRC522 (library handles SPI transactions internally)
  Serial.println("[RFID] Initializing MFRC522...");
  mfrc522.PCD_Init();
  delay(50);
  
  // Try to read version register multiple times (sometimes first read fails)
  byte version = 0x00;
  for (int i = 0; i < 3; i++) {
    version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
    Serial.print("[RFID] Version register read attempt ");
    Serial.print(i + 1);
    Serial.print(": 0x");
    Serial.println(version, HEX);
    if (version != 0x00 && version != 0xFF) {
      break; // Got valid version
    }
    delay(100);
  }
  
  // Check for communication failure
  if (version == 0x00 || version == 0xFF) {
    Serial.println("[RFID] ERROR: Cannot communicate with module!");
    Serial.println("[RFID] Version register reads 0x00 or 0xFF = No SPI communication");
    Serial.println("");
    Serial.println("[RFID] CRITICAL CHECKS:");
    Serial.println("  ✓ Red LED (D1) should be ON (power OK)");
    Serial.print("  ? RST pin connected to D");
    Serial.print(RFID_RST_PIN);
    Serial.println(" (NOT D2!)");
    Serial.print("  ? SDA/SS pin connected to D");
    Serial.println(RFID_SS_PIN);
    Serial.println("  ? MOSI connected to D23");
    Serial.println("  ? MISO connected to D19");
    Serial.println("  ? SCK connected to D18");
    Serial.println("");
    Serial.println("[RFID] COMMON MISTAKES:");
    Serial.println("  1. SDA/SS pin NOT connected (most common!)");
    Serial.println("  2. MOSI and MISO swapped");
    Serial.println("  3. Using wrong D pin numbers");
    Serial.println("  4. Loose breadboard connections");
    Serial.println("  5. Wrong power pin (must be 3.3V, not 5V)");
    Serial.println("");
    Serial.println("[RFID] TEST: Unplug and replug all wires, especially SDA/SS!");
    return false;
  }
  
  // Version 0x82 is valid (MFRC522 v1.0), so communication is working!
  // Perform self-test (but don't fail if it doesn't pass - card reading may still work)
  Serial.println("[RFID] Performing self-test...");
  delay(100); // Give module more time before self-test
  
  bool selfTestPassed = false;
  for (int i = 0; i < 3; i++) {
    if (mfrc522.PCD_PerformSelfTest()) {
      selfTestPassed = true;
      Serial.println("[RFID] Self-test PASSED!");
      break;
    }
    Serial.print("[RFID] Self-test attempt ");
    Serial.print(i + 1);
    Serial.println(" failed, retrying...");
    delay(200);
  }
  
  if (!selfTestPassed) {
    Serial.println("[RFID] WARNING: Self-test failed, but version is valid (0x82)");
    Serial.println("[RFID] Module communication is working - card reading may still work!");
    Serial.println("[RFID] This could be due to:");
    Serial.println("  - Power supply current limits (module needs ~13-26mA)");
    Serial.println("  - Module needs more stabilization time");
    Serial.println("  - Antenna tuning (some modules need adjustment)");
    Serial.println("[RFID] Proceeding anyway - try reading a card to test functionality.");
  }
  
  // Success! Version is valid, so we can proceed
  Serial.print("[RFID] MFRC522 version: 0x");
  Serial.println(version, HEX);
  Serial.println("[RFID] Reader initialized successfully!");
  Serial.println("[RFID] Ready to read cards - hold a card near the reader!");
  return true;
}

/**
 * Check if a new RFID card is present and read its UID
 * @param uidBuffer Buffer to store the UID (must be at least 4 bytes)
 * @param uidLength Pointer to store the UID length
 * @return true if a card is detected and UID is read, false otherwise
 */
bool readRFIDCard(byte* uidBuffer, byte* uidLength) {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }
  
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return false;
  }
  
  // Copy UID
  *uidLength = mfrc522.uid.size;
  for (byte i = 0; i < *uidLength; i++) {
    uidBuffer[i] = mfrc522.uid.uidByte[i];
  }
  
  // Halt PICC (card) to prevent multiple reads
  mfrc522.PICC_HaltA();
  
  return true;
}

/**
 * Convert UID byte array to hexadecimal string
 * @param uidBuffer UID byte array
 * @param uidLength Length of UID
 * @return Hexadecimal string representation of UID
 */
String uidToString(byte* uidBuffer, byte uidLength) {
  String uidString = "";
  for (byte i = 0; i < uidLength; i++) {
    if (uidBuffer[i] < 0x10) {
      uidString += "0";
    }
    uidString += String(uidBuffer[i], HEX);
    if (i < uidLength - 1) {
      uidString += ":";
    }
  }
  uidString.toUpperCase();
  return uidString;
}

/**
 * Display RFID card UID on OLED display
 * @param display Reference to the Adafruit_SSD1306 display object
 * @param uidString UID string to display
 */
void displayRFIDCard(Adafruit_SSD1306& display, const String& uidString) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Center "RFID Card Detected"
  String title = "RFID Card";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(title.c_str(), 0, 0, &x1, &y1, &w, &h);
  int xTitle = (128 - w) / 2;
  display.setCursor(xTitle, 5);
  display.println(title);
  
  // Draw line separator
  display.drawLine(0, 15, 128, 15, SSD1306_WHITE);
  
  // Display "UID:" label
  display.setCursor(0, 25);
  display.print("UID:");
  
  // Display UID (split if too long)
  if (uidString.length() <= 20) {
    // UID fits on one line
    display.setCursor(0, 40);
    display.print(uidString);
  } else {
    // Split UID across two lines
    int midPoint = uidString.length() / 2;
    // Find a good split point (preferably at a colon)
    int splitPoint = midPoint;
    for (int i = midPoint; i < uidString.length() && i < midPoint + 5; i++) {
      if (uidString.charAt(i) == ':') {
        splitPoint = i + 1;
        break;
      }
    }
    
    display.setCursor(0, 40);
    display.print(uidString.substring(0, splitPoint));
    display.setCursor(0, 50);
    display.print(uidString.substring(splitPoint));
  }
  
  display.display();
}

/**
 * Display "Waiting for card" message on OLED
 * @param display Reference to the Adafruit_SSD1306 display object
 */
void displayWaitingForCard(Adafruit_SSD1306& display) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Center "Waiting for RFID"
  String title = "Waiting for";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(title.c_str(), 0, 0, &x1, &y1, &w, &h);
  int xTitle = (128 - w) / 2;
  display.setCursor(xTitle, 20);
  display.println(title);
  
  String subtitle = "RFID Card...";
  display.getTextBounds(subtitle.c_str(), 0, 0, &x1, &y1, &w, &h);
  int xSubtitle = (128 - w) / 2;
  display.setCursor(xSubtitle, 35);
  display.println(subtitle);
  
  display.display();
}

#endif // RFID_READER_H

