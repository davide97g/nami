# RFID Reader Module (MFRC522)

## Overview
The MFRC522 is a highly integrated reader/writer IC for contactless communication at 13.56 MHz. It's a low-voltage, low-cost, and compact solution ideal for smart meters and portable devices. The module communicates via SPI interface and supports various MiFare card types.

## Key Specifications

### General Characteristics
- **Operating Frequency**: 13.56 MHz
- **Operating Voltage**: 3.3V DC
- **Current Consumption**:
  - Operating: 13-26 mA
  - Idle: 10-13 mA
  - Sleep: <80 µA
  - Peak: <30 mA
- **Communication Interface**: SPI (Serial Peripheral Interface)
- **Maximum SPI Speed**: 10 Mbit/s
- **Reading Range**: 0-35 mm (for MiFare1 S50 cards)

### Supported Card Types
- MiFare1 S50
- MiFare1 S70
- MiFare Ultralight
- MiFare Pro

### Physical Characteristics
- **Dimensions**: 40 x 60 mm
- **Operating Temperature**: -20°C to 80°C
- **Storage Temperature**: -40°C to 85°C
- **Relative Humidity**: 5% - 95%

### Pin Configuration
The MFRC522 module typically has the following pins:
- **VCC**: Power supply (3.3V)
- **GND**: Ground
- **RST**: Reset pin
- **SDA (SS)**: Slave Select / Chip Select (SPI)
- **MOSI**: Master Out Slave In (SPI)
- **MISO**: Master In Slave Out (SPI)
- **SCK**: Serial Clock (SPI)
- **IRQ**: Interrupt pin (optional, not used in basic setup)

## Usage with ESP32

### Wiring
```
MFRC522 Module    ESP32
VCC           ->  3.3V
GND           ->  GND
RST           ->  D4  (changed from D2 - D2 has built-in LED)
SDA (SS)      ->  D5
MOSI          ->  D23 (VSPI MOSI)
MISO          ->  D19 (VSPI MISO)
SCK           ->  D18 (VSPI SCK)
```

**Note**: 
- If your board uses "D" labels (like D2, D5, etc.), use those for wiring
- The code still uses GPIO numbers internally (2, 5, 23, 19, 18)
- The ESP32 uses VSPI (default SPI) with the following default pins:
  - MOSI: D23 (GPIO 23)
  - MISO: D19 (GPIO 19)
  - SCK: D18 (GPIO 18)

### Code Example (Arduino)
```cpp
#include <SPI.h>
#include <MFRC522.h>

#define RFID_RST_PIN  2
#define RFID_SS_PIN   5

MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  
  // Check if module is connected
  if (mfrc522.PCD_PerformSelfTest()) {
    Serial.println("RFID reader initialized!");
  } else {
    Serial.println("RFID initialization failed!");
  }
}

void loop() {
  // Look for new cards
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Print UID
    Serial.print("Card UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      if (i < mfrc522.uid.size - 1) Serial.print(":");
    }
    Serial.println();
    
    // Halt card to prevent multiple reads
    mfrc522.PICC_HaltA();
  }
  delay(100);
}
```

## Features
- **Contactless Communication**: No physical contact required
- **Fast Data Transfer**: Up to 424 kbps bidirectional communication
- **Low Power**: Suitable for battery-powered applications
- **Easy Integration**: Simple SPI interface
- **Multiple Card Support**: Compatible with various MiFare card types
- **Open Source Libraries**: Well-supported Arduino libraries available

## Important Notes
- **Voltage Compatibility**: The module requires 3.3V. Do not connect to 5V as it may damage the module.
- **SPI Interface**: Ensure proper SPI wiring and that no other devices conflict with the SPI pins.
- **Reading Distance**: Optimal reading distance is typically 2-5 cm. Distance may vary based on card type and antenna design.
- **Multiple Cards**: If multiple cards are present, the reader will detect the closest one.
- **Card Halt**: Always call `PICC_HaltA()` after reading to prevent multiple detections of the same card.

## Troubleshooting

### Module Not Detected
- Check all wiring connections
- Verify 3.3V power supply (not 5V)
- Ensure SPI pins are correctly connected
- Check that RST and SS pins are properly connected

### No Card Detection
- Ensure card is within reading range (2-5 cm)
- Try different card types
- Check if card is MiFare compatible
- Verify antenna is not damaged

### Communication Errors
- Check SPI wiring (MOSI, MISO, SCK)
- Verify SS (SDA) pin connection
- Ensure no other devices are using the same SPI bus
- Try reducing SPI speed if issues persist

## Library Installation
Install the MFRC522 library using Arduino CLI:
```bash
arduino-cli lib install "MFRC522"
```

Or add to `libraries.txt`:
```
MFRC522
```

## Further Documentation
- [MFRC522 Datasheet](https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf)
- [MFRC522 Arduino Library](https://github.com/miguelbalboa/rfid)
- [ESP32 SPI Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/spi_master.html)

## Product Links
- [Amazon Product Page](https://www.amazon.it/dp/B0DLGVNX93)

