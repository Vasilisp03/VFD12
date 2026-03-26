# FutabaVFD

An Arduino library for the **Futaba 012BT231Q1NK** 12-character dot matrix vacuum fluorescent display (VFD) module.

This library was reverse engineered from scratch — the display has no public datasheet. The icon register map and 12-digit mode unlock were discovered through systematic address scanning.

---

## Hardware

| Display | Futaba 012BT231Q1NK |
|---|---|
| Characters | 12 dot matrix |
| Interface | SPI (bit-banged) |
| Supply | 5V (3.3V works) |
| Logic | 3.3V compatible |

### Wiring

| VFD Pin | ESP32 Pin |
|---|---|
| DIN | GPIO 19 (MOSI) |
| CK | GPIO 5 (SCK) |
| CS | Any GPIO (e.g. 27) |
| EN | Any GPIO (e.g. 15) |
| VCC | 5V or 3.3V |
| GND | GND |

---

## Installation

1. Download this repository as a ZIP
2. Arduino IDE → Sketch → Include Library → Add .ZIP Library
3. Select the downloaded ZIP

---

## Quick Start

```cpp
#include "FutabaVFD.h"

FutabaVFD vfd(19, 5, 27, 15); // DIN, CLK, CS, EN

void setup() {
  vfd.begin();
  vfd.print(0, "HELLO WORLD!");
  vfd.showPlay(true);
}

void loop() {}
```

---

## API Reference

### Initialisation
```cpp
vfd.begin();
```

### Display
```cpp
vfd.print(uint8_t pos, const char* text); // print at position 0-11
vfd.clear();                              // clear all characters
```

### Brightness
```cpp
vfd.setBrightness(uint8_t level); // 0x10 (dim) to 0xFF (max)
```

### Icons - by index
```cpp
vfd.iconOn(uint8_t iconIndex);
vfd.iconOff(uint8_t iconIndex);
vfd.allIconsOn();
vfd.allIconsOff();
vfd.cascadeIconsOn(uint16_t delayMs);
vfd.cascadeIconsOff(uint16_t delayMs);
```

### Icon indices
```cpp
VFD_ICON_USB          // 0
VFD_ICON_HD           // 1
VFD_ICON_HDD          // 2
VFD_ICON_DOLBY        // 3
VFD_ICON_MP3          // 4
VFD_ICON_MUTE         // 5
VFD_ICON_REWIND       // 6
VFD_ICON_PLAY         // 7
VFD_ICON_PAUSE        // 8
VFD_ICON_FASTFORWARD  // 9
VFD_ICON_REC          // 10
VFD_ICON_CLOCK        // 11
VFD_ICON_COLON_LEFT   // 12
VFD_ICON_COLON_RIGHT  // 13
```

### Icons - named shortcuts
```cpp
vfd.showUSB(bool on);
vfd.showHD(bool on);
vfd.showHDD(bool on);
vfd.showDolby(bool on);
vfd.showMP3(bool on);
vfd.showMute(bool on);
vfd.showRewind(bool on);
vfd.showPlay(bool on);
vfd.showPause(bool on);
vfd.showFastForward(bool on);
vfd.showREC(bool on);
vfd.showClock(bool on);
vfd.showColons(bool on); // both colons together
```

### Low level access
```cpp
vfd.sendCommand(uint8_t cmd);
vfd.sendData(uint8_t addr, uint8_t data);
```

---

## Key discoveries

- **12-digit mode** must be set with `0xe0, 0x0B` — the default 8-digit mode locks out icon addresses `0x68-0x6B` (Pause, Fast Forward, REC, Clock)
- **Brightness register** is at `0xe4` — minimum safe value is `0x10`, writing `0x00` kills the display until power cycle
- **Icon register map** — all icons use bit value `0x02` except the two colons which use `0x01`

---

## Notes

- Tested on ESP32 Feather V2
- Should work on any 3.3V Arduino with SPI capability
- The Vfd12 library by shabaz was used as a starting reference for character display
- Icon map was reverse engineered by scanning all addresses with all bit values

---

## License

MIT
