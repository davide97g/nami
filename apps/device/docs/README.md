# Device Documentation

This folder contains technical documentation for all devices, sensors, and components used in the Nami project.

## Available Documentation

### Microcontrollers & Development Boards

- **[ESP32 Development Board](./esp32.md)** - ELEGOO ESP32 WROOM-32 development board with Wi-Fi and Bluetooth

### Sensors & Input Devices

- **[Touch Sensor Module](./touch-sensor.md)** - Capacitive touch sensor for user input
- **[RFID Reader Module](./rfid-reader.md)** - MFRC522 RFID reader/writer for contactless card reading

### Display Components

- **[OLED Display Module](./oled-display.md)** - SSD1306-based 128x64 monochrome OLED display

### Accessories & Components

- **[Jumper Wires](./jumper-wires.md)** - Breadboard jumper wires for prototyping

## Quick Reference

### ESP32 Pin Connections

#### OLED Display (I²C)

```
OLED    ESP32
VCC  -> 3.3V
GND  -> GND
SDA  -> D21
SCL  -> D22
```

#### Touch Sensor

```
Touch Sensor    ESP32
VCC         ->  3.3V
GND         ->  GND
OUT         ->  D4 (or any D pin)
```

#### RFID Reader (MFRC522)

```
MFRC522 Module    ESP32
VCC           ->  3.3V
GND           ->  GND
RST           ->  D4  (changed from D2 - D2 has built-in LED)
SDA (SS)      ->  D5
MOSI          ->  D23
MISO          ->  D19
SCK           ->  D18
```

**Note**: If your board uses "D" labels (D2, D5, etc.), use those for wiring. The code uses GPIO numbers internally.

## Adding New Documentation

When adding documentation for a new device or component:

1. Create a new markdown file in this directory
2. Follow the existing documentation format:
   - Overview section
   - Key Specifications
   - Usage with ESP32 (if applicable)
   - Wiring diagrams
   - Code examples
   - Important notes
   - Further documentation links
   - Product links
3. Update this README with a link to the new documentation

## Resources

- [ESP32 Official Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [Arduino ESP32 Core](https://github.com/espressif/arduino-esp32)
- [Adafruit Learning System](https://learn.adafruit.com/)
