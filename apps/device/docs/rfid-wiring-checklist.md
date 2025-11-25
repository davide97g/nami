# RFID Reader Wiring Checklist

## Quick Wiring Reference

```
MFRC522 Module    ESP32 (D pins)
VCC           ->  3.3V
GND           ->  GND
RST           ->  D4  (changed from D2 - D2 has built-in LED)
SDA (SS)      ->  D5
MOSI          ->  D23
MISO          ->  D19
SCK           ->  D18
```

## Step-by-Step Wiring Checklist

### Power Connections (CRITICAL)

- [ ] **VCC → 3.3V**: Module MUST be powered with 3.3V, NOT 5V!
- [ ] **GND → GND**: Common ground connection
- [ ] **Red LED (D1) glows**: This confirms power is connected

### Control Pins

- [ ] **RST → D4**: Reset pin connection (NOT D2 - D2 has built-in LED)
- [ ] **SDA (SS) → D5**: Slave Select / Chip Select pin

### SPI Communication Pins

- [ ] **MOSI → D23**: Master Out Slave In (data from ESP32 to RFID)
- [ ] **MISO → D19**: Master In Slave Out (data from RFID to ESP32)
- [ ] **SCK → D18**: Serial Clock (synchronization signal)

## Common Issues and Solutions

### Issue: Red LED (D1) glows but module doesn't work

**Possible causes:**

1. **Wrong voltage**: Module is getting 5V instead of 3.3V
   - **Solution**: Double-check VCC is connected to 3.3V, not 5V
2. **SPI pins swapped**: MOSI/MISO might be reversed
   - **Solution**: Verify MOSI→D23, MISO→D19
3. **SS pin not connected**: SDA (SS) pin must be connected to D5
   - **Solution**: Check SDA connection to D5
4. **Loose connections**: Wires not making good contact
   - **Solution**: Re-seat all connections, ensure wires are firmly inserted

### Issue: No LED at all

**Possible causes:**

1. **No power**: VCC or GND not connected
   - **Solution**: Check VCC→3.3V and GND→GND connections
2. **Wrong power pin**: Connected to wrong pin on ESP32
   - **Solution**: Use 3.3V pin, not 5V or VIN

### Issue: Version register reads 0x00 or 0xFF

**This indicates communication failure:**

1. **Check all SPI pins**: MOSI, MISO, SCK, SS
2. **Check RST pin**: Must be connected to D4 (NOT D2 - D2 has built-in LED)
3. **Verify pin numbers**: Ensure you're using D4, D5, D23, D19, D18
4. **Check for loose connections**: Re-seat all wires

### Issue: Blue LED on ESP32 is active

**This indicates GPIO 2 (D2) is being used:**

- **Problem**: D2 has a built-in blue LED on many ESP32 boards
- **Solution**: Use D4 for RST pin instead of D2 (already updated in code)

### Issue: Self-test fails

**Module is detected but self-test fails:**

1. **Power supply quality**: Try a different 3.3V source or add a capacitor
2. **Loose connections**: Re-check all connections
3. **Module damage**: Try a different RFID module if available

## Pin Verification

### ESP32 Pin Locations

- **3.3V**: Usually labeled "3V3" or "3.3V"
- **GND**: Multiple GND pins available
- **D4**: GPIO 4 (RST pin - NOT D2 which has built-in LED)
- **D5**: GPIO 5
- **D18**: GPIO 18 (SPI SCK)
- **D19**: GPIO 19 (SPI MISO)
- **D23**: GPIO 23 (SPI MOSI)

**Important**: D2 has a built-in blue LED on many ESP32 boards. Do NOT use D2 for RFID RST pin!

### MFRC522 Module Pin Labels

Common labels on the module:

- **VCC** or **3.3V**: Power supply
- **GND**: Ground
- **RST** or **RESET**: Reset pin
- **SDA** or **SS** or **NSS**: Slave Select
- **MOSI** or **SI**: Master Out Slave In
- **MISO** or **SO**: Master In Slave Out
- **SCK** or **SCLK**: Serial Clock

## Testing Procedure

1. **Power Check**: Verify red LED (D1) is glowing
2. **Upload Code**: Upload the sketch and open Serial Monitor (115200 baud)
3. **Check Serial Output**: Look for detailed error messages
4. **Verify Connections**: If errors persist, go through checklist above
5. **Try Different Module**: If available, test with another RFID module

## Serial Monitor Output

### Successful Initialization

```
[RFID] Starting initialization...
[RFID] Pin configuration: RST=D2, SS=D5
[RFID] Version register read: 0x92
[RFID] Performing self-test...
[RFID] MFRC522 version: 0x92
[RFID] Reader initialized successfully!
```

### Failed Initialization

```
[RFID] Starting initialization...
[RFID] Pin configuration: RST=D2, SS=D5
[RFID] Version register read: 0x00
[RFID] ERROR: Cannot communicate with module!
[RFID] Troubleshooting:
  1. Check VCC -> 3.3V (NOT 5V!)
  2. Check GND -> GND
  ...
```

## Additional Tips

- **Use a breadboard**: Makes connections easier to verify and modify
- **Color-code wires**: Use different colors for different signal types
- **Check wire continuity**: Use a multimeter to verify connections
- **Keep wires short**: Long wires can cause communication issues
- **Avoid interference**: Keep RFID module away from strong electromagnetic sources
