# Deej32 ESP

A hardware audio mixer controller for Windows, inspired by [deej](https://github.com/omriharel/deej), using an ESP32, rotary encoders, and an OLED display.

---

## Features

- Control master and application audio volumes with rotary encoders
- OLED display shows session names, volume levels, and mute status
- Mute/unmute with encoder button press
- Automatic session sync from PC
- Serial communication with a Windows companion app

---

## Hardware Requirements

- ESP32 (e.g., ESP32-C3 Supermini)
- 2x Rotary encoders (with push button)
- 0.96" or 1.3" I2C OLED display (SSD1315/SSD1306 compatible)
- Breadboard & jumper wires
- Windows PC (for companion app)

---

## Pinout

| Function         | ESP32 Pin |
|------------------|-----------|
| Encoder 1 CLK    | 3         |
| Encoder 1 DT     | 10        |
| Encoder 1 SW     | 8         |
| Encoder 2 CLK    | 4         |
| Encoder 2 DT     | 6         |
| Encoder 2 SW     | 5         |
| OLED SCL         | 1         |
| OLED SDA         | 2         |

---

## Getting Started

1. **Wire up the hardware** according to the pinout above.
2. **Flash the ESP32** with this firmware using PlatformIO or Arduino IDE.
3. **Build and run the Windows companion app** (`DeejV2`).
4. **Connect the ESP32 to your PC** via USB.
5. **Use the encoders** to control volume and mute. The OLED will display session info.

---

## File Structure

- `src/main.cpp` – ESP32 setup/loop, display error handling
- `src/DeejControl.cpp` – Encoder handling, display, serial protocol
- `DeejV2` (PC app) – Windows audio session management and serial communication

---

## Troubleshooting

- **OLED artifacts:** Check wiring, ensure `u8g2.clearBuffer()` is called before drawing.
- **Encoders interfere:** Ensure each encoder uses separate pins and interrupts.
- **No sessions detected:** Make sure the PC app is running and the ESP32 is connected.

---

## Credits

- Inspired by [deej](https://github.com/omriharel/deej)
- Uses [u8g2](https://github.com/olikraus/u8g2) for OLED display

---

##
