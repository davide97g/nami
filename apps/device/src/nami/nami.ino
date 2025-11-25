#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include "wifi_connection.h"
#include "websocket_client.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Bitmap data - 40x30px
const unsigned char epd_bitmap_25 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x01, 0x80, 0x3c, 0x00, 0x00, 0x01, 0x80, 0x7c, 
	0x00, 0x00, 0x01, 0x0c, 0xf8, 0x00, 0x00, 0x07, 0x9d, 0xf0, 0x00, 0x00, 0x0f, 0xfb, 0xe0, 0x00, 
	0x00, 0x0f, 0xf9, 0xc0, 0x00, 0x00, 0x1f, 0xb8, 0xe0, 0x00, 0x00, 0x1f, 0x3c, 0x60, 0x00, 0x00, 
	0x0f, 0xfe, 0xc0, 0x00, 0x00, 0x17, 0xfc, 0x00, 0x00, 0x00, 0x03, 0xdf, 0x00, 0x00, 0x00, 0x01, 
	0xb6, 0x00, 0x00, 0x00, 0x01, 0xcf, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x1e, 
	0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// System info fetch interval (in milliseconds)
#define INFO_FETCH_INTERVAL 30000  // Fetch every 30 seconds
unsigned long lastInfoFetch = 0;

// Touch sensor configuration
#define TOUCH_SENSOR_PIN 4  // GPIO pin connected to touch sensor OUT pin
int touchState = LOW;
int lastTouchState = LOW;
unsigned long touchCount = 0;
unsigned long lastTouchTime = 0;
unsigned long lastTouchDisplayUpdate = 0;
const unsigned long TOUCH_DISPLAY_UPDATE_INTERVAL = 200; // Update display every 200ms
bool touchSensorConnected = false; // Connection status from diagnostic test

// Display mode management
bool showingSystemInfo = false;
unsigned long systemInfoDisplayStart = 0;
const unsigned long SYSTEM_INFO_DISPLAY_DURATION = 5000; // Show system info for 5 seconds

void setup() {
  // Initialize Serial for logging
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=== Nami ESP32 Starting ===");

  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();

  // --- Startup Display ---
  // Display "nami" text centered and enlarged with bitmap
  display.setTextSize(2); // Enlarged text
  display.setTextColor(SSD1306_WHITE);
  
  // Center "nami" text (text size 2: ~12 pixels per character)
  int textWidth = 4 * 12; // "nami" is 4 characters at size 2
  int xText = (SCREEN_WIDTH - textWidth) / 2;
  display.setCursor(xText, 10);
  display.print("nami");
  
  // Center bitmap below text (40x30px bitmap)
  int xBitmap = (SCREEN_WIDTH - 40) / 2;
  display.drawXBitmap(xBitmap, 30, epd_bitmap_25, 40, 30, SSD1306_WHITE);
  display.display();
  
  // 2 second delay
  delay(2000);

  // --- Setup Animation ---
  // Display "setting up your nami" with animated dots for 5 seconds
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  String setupText = "setting up your nami";
  int xSetup = centerText(display, setupText, 0);
  display.setCursor(xSetup, 25);
  display.print(setupText);
  
  unsigned long setupStartTime = millis();
  const unsigned long setupDuration = 5000; // 5 seconds
  int dotCount = 0;
  
  while (millis() - setupStartTime < setupDuration) {
    // Clear dots area
    display.fillRect(0, 35, SCREEN_WIDTH, 10, SSD1306_BLACK);
    
    // Display dots (one at a time, looping through 3)
    String dots = "";
    for (int i = 0; i < dotCount; i++) {
      dots += ".";
    }
    
    int xDots = centerText(display, dots, 0);
    display.setCursor(xDots, 35);
    display.print(dots);
    display.display();
    
    // Update dot count (0, 1, 2, then back to 0)
    delay(500); // Change dot every 500ms
    dotCount = (dotCount + 1) % 3;
  }
  
  // Clear display before WiFi connection
  display.clearDisplay();
  display.display();
  delay(200);

  // --- WiFi Connection ---
  // Connect to WiFi and display status on screen
  bool wifiConnected = connectToWiFi(display);
  
  // Only proceed with other logic after WiFi connection succeeds
  if (!wifiConnected) {
    // If connection failed, retry once more
    delay(1000);
    wifiConnected = connectToWiFi(display);
  }

  // Wait for WiFi connection before proceeding
  // If still not connected, show error and wait
  if (!wifiConnected) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    int x1 = centerText(display, "WiFi Failed", 0);
    display.setCursor(x1, 10);
    display.println("WiFi Failed");
    
    int x2 = centerText(display, "Check config", 0);
    display.setCursor(x2, 25);
    display.println("Check config");
    
    int x3 = centerText(display, "Restarting...", 0);
    display.setCursor(x3, 40);
    display.println("Restarting...");
    display.display();
    delay(3000);
    ESP.restart(); // Restart ESP32 to retry connection
    return;
  }

  // Clear display after successful WiFi connection
  display.clearDisplay();
  display.display();
  delay(500);

  // --- WebSocket Connection ---
  Serial.println("[Setup] Connecting to WebSocket...");
  bool wsConnected = connectWebSocket(display);
  if (wsConnected) {
    Serial.println("[Setup] WebSocket connection succeeded!");
  } else {
    Serial.println("[Setup] WebSocket connection failed!");
  }

  delay(1000);

  // --- Touch Sensor Initialization ---
  pinMode(TOUCH_SENSOR_PIN, INPUT);
  Serial.println("[Setup] Touch sensor initialized on GPIO " + String(TOUCH_SENSOR_PIN));
  
  // Test touch sensor connection
  touchSensorConnected = testTouchSensorConnection(display);
  if (!touchSensorConnected) {
    Serial.println("[Setup] WARNING: Touch sensor connection test failed!");
    Serial.println("[Setup] Please check wiring:");
    Serial.println("  - Touch Sensor VCC -> ESP32 3.3V");
    Serial.println("  - Touch Sensor GND -> ESP32 GND");
    Serial.println("  - Touch Sensor OUT -> ESP32 GPIO " + String(TOUCH_SENSOR_PIN));
  } else {
    Serial.println("[Setup] Touch sensor connection test passed!");
  }

  // --- Fetch System Info ---
  Serial.println("[Setup] Fetching system info from /info endpoint...");
  fetchAndDisplaySystemInfo(display);
  showingSystemInfo = true;
  systemInfoDisplayStart = millis();
  lastInfoFetch = millis();
  
  // Initial touch sensor display will be shown after system info duration
}

/**
 * Test touch sensor connection and display diagnostic information
 * @param display Reference to the Adafruit_SSD1306 display object
 * @return true if touch sensor appears to be connected, false otherwise
 */
bool testTouchSensorConnection(Adafruit_SSD1306& display) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  int x1 = centerText(display, "Testing Touch", 0);
  display.setCursor(x1, 10);
  display.println("Testing Touch");
  
  int x2 = centerText(display, "Sensor...", 0);
  display.setCursor(x2, 25);
  display.println("Sensor...");
  display.display();
  
  delay(500);
  
  // Read initial state
  int initialRead = digitalRead(TOUCH_SENSOR_PIN);
  Serial.print("[Touch Test] Initial pin state: ");
  Serial.println(initialRead);
  
  // Take multiple readings to check for stability
  const int testReadings = 20;
  int readings[testReadings];
  int highCount = 0;
  int lowCount = 0;
  int stateChanges = 0;
  
  for (int i = 0; i < testReadings; i++) {
    readings[i] = digitalRead(TOUCH_SENSOR_PIN);
    if (readings[i] == HIGH) {
      highCount++;
    } else {
      lowCount++;
    }
    
    // Check for state changes
    if (i > 0 && readings[i] != readings[i-1]) {
      stateChanges++;
    }
    
    delay(50);
  }
  
  // Analyze results
  bool isConnected = true;
  String statusMessage = "";
  String diagnosticInfo = "";
  
  // Check if pin is floating (unconnected) - would read random values
  if (stateChanges > testReadings * 0.8) {
    // Too many state changes - likely floating/unconnected
    isConnected = false;
    statusMessage = "NOT CONNECTED";
    diagnosticInfo = "Pin floating";
    Serial.println("[Touch Test] ERROR: Pin appears to be floating (unconnected)");
  } else if (highCount == testReadings || lowCount == testReadings) {
    // Pin stuck at one value - could be connected but not responding
    isConnected = true; // Sensor might be connected but not touched
    statusMessage = "CONNECTED";
    if (highCount == testReadings) {
      diagnosticInfo = "Pin stuck HIGH";
      Serial.println("[Touch Test] WARNING: Pin stuck at HIGH - check wiring");
    } else {
      diagnosticInfo = "Pin stuck LOW";
      Serial.println("[Touch Test] INFO: Pin at LOW (normal when not touched)");
    }
  } else {
    // Some variation - sensor is likely connected
    isConnected = true;
    statusMessage = "CONNECTED";
    diagnosticInfo = "Reading OK";
    Serial.println("[Touch Test] SUCCESS: Touch sensor appears to be connected");
  }
  
  // Display results
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  int xTitle = centerText(display, "Touch Sensor", 0);
  display.setCursor(xTitle, 0);
  display.println("Touch Sensor");
  
  display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
  
  int xStatus = centerText(display, "Status:", 0);
  display.setCursor(xStatus, 18);
  display.println("Status:");
  
  int xStatusVal = centerText(display, statusMessage, 0);
  display.setCursor(xStatusVal, 28);
  display.setTextColor(isConnected ? SSD1306_WHITE : SSD1306_WHITE);
  display.println(statusMessage);
  display.setTextColor(SSD1306_WHITE);
  
  // Diagnostic info
  if (diagnosticInfo.length() > 0) {
    if (diagnosticInfo.length() > 21) {
      diagnosticInfo = diagnosticInfo.substring(0, 18) + "...";
    }
    int xDiag = centerText(display, diagnosticInfo, 0);
    display.setCursor(xDiag, 38);
    display.println(diagnosticInfo);
  }
  
  // Pin info
  String pinInfo = "Pin: GPIO " + String(TOUCH_SENSOR_PIN);
  int xPin = centerText(display, pinInfo, 0);
  display.setCursor(xPin, 48);
  display.println(pinInfo);
  
  // Reading info
  String readInfo = "Read: " + String(initialRead);
  int xRead = centerText(display, readInfo, 0);
  display.setCursor(xRead, 56);
  display.println(readInfo);
  
  display.display();
  
  // Log detailed diagnostic info
  Serial.println("[Touch Test] Diagnostic Results:");
  Serial.print("  High readings: ");
  Serial.println(highCount);
  Serial.print("  Low readings: ");
  Serial.println(lowCount);
  Serial.print("  State changes: ");
  Serial.println(stateChanges);
  Serial.print("  Connection status: ");
  Serial.println(isConnected ? "OK" : "FAILED");
  
  delay(3000); // Show diagnostic for 3 seconds
  
  return isConnected;
}

/**
 * Send touch sensor data to WebSocket stream
 * @param state Current touch state (HIGH or LOW)
 * @param count Total touch count
 */
void sendTouchSensorToStream(int state, unsigned long count) {
  if (!webSocket.isConnected()) {
    return; // Don't send if WebSocket is not connected
  }
  
  // Create JSON message
  StaticJsonDocument<256> doc;
  doc["type"] = "touch_sensor";
  doc["data"]["state"] = (state == HIGH) ? "touched" : "not_touched";
  doc["data"]["value"] = state;
  doc["data"]["count"] = count;
  
  // Use lastTouchTime if available (for touch events), otherwise use current time
  unsigned long eventTimestamp = (lastTouchTime > 0) ? lastTouchTime : millis();
  doc["data"]["timestamp"] = eventTimestamp;
  doc["data"]["timestamp_ms"] = eventTimestamp;
  
  // Also include time since last touch (if applicable)
  if (lastTouchTime > 0) {
    doc["data"]["time_since_touch_ms"] = millis() - lastTouchTime;
  }
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Send to WebSocket
  webSocket.sendTXT(jsonString);
  Serial.println("[Touch] Sent to stream: " + jsonString);
}

/**
 * Display touch sensor status on OLED display
 */
void displayTouchSensorStatus() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Title
  int xTitle = centerText(display, "Touch Sensor", 0);
  display.setCursor(xTitle, 0);
  display.println("Touch Sensor");
  
  // Draw a line separator
  display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
  
  // Connection status indicator (small text at top right)
  String connStatus = touchSensorConnected ? "OK" : "ERR";
  display.setCursor(SCREEN_WIDTH - 20, 0);
  if (!touchSensorConnected) {
    display.setTextColor(SSD1306_WHITE);
  }
  display.println(connStatus);
  display.setTextColor(SSD1306_WHITE);
  
  // Status line
  String statusText = touchState == HIGH ? "Status: TOUCHED" : "Status: NOT TOUCHED";
  int xStatus = centerText(display, statusText, 0);
  display.setCursor(xStatus, 18);
  display.println(statusText);
  
  // Touch count
  String countText = "Touches: " + String(touchCount);
  int xCount = centerText(display, countText, 0);
  display.setCursor(xCount, 28);
  display.println(countText);
  
  // Timestamp display
  if (lastTouchTime > 0) {
    unsigned long timeSinceTouch = millis() - lastTouchTime;
    String timeText;
    
    if (timeSinceTouch < 1000) {
      timeText = "Last: " + String(timeSinceTouch) + "ms ago";
    } else if (timeSinceTouch < 60000) {
      timeText = "Last: " + String(timeSinceTouch / 1000) + "s ago";
    } else {
      unsigned long minutes = timeSinceTouch / 60000;
      unsigned long seconds = (timeSinceTouch % 60000) / 1000;
      timeText = "Last: " + String(minutes) + "m " + String(seconds) + "s ago";
    }
    
    // Truncate if too long
    if (timeText.length() > 21) {
      timeText = timeText.substring(0, 18) + "...";
    }
    
    int xTime = centerText(display, timeText, 0);
    display.setCursor(xTime, 48);
    display.println(timeText);
  } else {
    int xTime = centerText(display, "Last: Never", 0);
    display.setCursor(xTime, 48);
    display.println("Last: Never");
  }
  
  // Visual indicator (filled circle when touched)
  int indicatorX = SCREEN_WIDTH / 2 - 5;
  int indicatorY = 58;
  if (touchState == HIGH) {
    // Draw filled circle when touched
    display.fillCircle(indicatorX, indicatorY, 5, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(indicatorX - 2, indicatorY - 3);
    display.print("!");
    display.setTextColor(SSD1306_WHITE);
  } else {
    // Draw empty circle when not touched
    display.drawCircle(indicatorX, indicatorY, 5, SSD1306_WHITE);
  }
  
  display.display();
}

void loop() {
  // --- Maintain WebSocket Connection ---
  maintainWebSocket();

  // --- Touch Sensor Reading ---
  touchState = digitalRead(TOUCH_SENSOR_PIN);
  
  // Periodic diagnostic check (every 30 seconds) if sensor not connected
  static unsigned long lastDiagnosticCheck = 0;
  if (!touchSensorConnected && (millis() - lastDiagnosticCheck > 30000)) {
    Serial.println("[Touch] Running periodic diagnostic check...");
    touchSensorConnected = testTouchSensorConnection(display);
    lastDiagnosticCheck = millis();
  }
  
  // Detect touch state change (edge detection)
  if (touchState == HIGH && lastTouchState == LOW) {
    // Touch detected (rising edge)
    touchCount++;
    lastTouchTime = millis();
    Serial.println("[Touch] Touch detected! Count: " + String(touchCount));
    
    // Send touch event to WebSocket stream
    sendTouchSensorToStream(touchState, touchCount);
  } else if (touchState == LOW && lastTouchState == HIGH) {
    // Touch released (falling edge)
    Serial.println("[Touch] Touch released");
    
    // Send release event to WebSocket stream
    sendTouchSensorToStream(touchState, touchCount);
  }
  
  lastTouchState = touchState;
  
  // --- System Info Fetching ---
  unsigned long currentTime = millis();
  
  // Check if system info display duration has elapsed, return to touch sensor display
  if (showingSystemInfo && (currentTime - systemInfoDisplayStart >= SYSTEM_INFO_DISPLAY_DURATION)) {
    showingSystemInfo = false;
    lastTouchDisplayUpdate = 0; // Force immediate update of touch sensor display
  }
  
  // Check if it's time to fetch new data
  if (currentTime - lastInfoFetch >= INFO_FETCH_INTERVAL) {
    fetchAndDisplaySystemInfo(display);
    showingSystemInfo = true;
    systemInfoDisplayStart = currentTime;
    lastInfoFetch = currentTime;
  }
  
  // Update touch sensor display periodically (only if not showing system info)
  if (!showingSystemInfo && (currentTime - lastTouchDisplayUpdate >= TOUCH_DISPLAY_UPDATE_INTERVAL)) {
    displayTouchSensorStatus();
    lastTouchDisplayUpdate = currentTime;
  }
  
  // Small delay to prevent excessive CPU usage
  delay(50);
}

