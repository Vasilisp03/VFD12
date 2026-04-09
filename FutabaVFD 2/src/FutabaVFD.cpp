#include "FutabaVFD.h"

// Icon map: {address, bit value}
const uint8_t FutabaVFD::_iconMap[VFD_NUM_ICONS][2] = {
  {0x60, 0x02}, //  0 USB
  {0x61, 0x02}, //  1 HD
  {0x62, 0x02}, //  2 HDD
  {0x63, 0x02}, //  3 Dolby Atmos
  {0x64, 0x02}, //  4 MP3
  {0x65, 0x02}, //  5 Mute
  {0x66, 0x02}, //  6 Rewind
  {0x67, 0x02}, //  7 Play
  {0x68, 0x02}, //  8 Pause
  {0x69, 0x02}, //  9 Fast Forward
  {0x6A, 0x02}, // 10 REC
  {0x6B, 0x02}, // 11 Clock
  {0x67, 0x01}, // 12 Colon Left
  {0x68, 0x01}, // 13 Colon Right
};

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────
FutabaVFD::FutabaVFD(uint8_t dinPin, uint8_t clkPin, uint8_t csPin, uint8_t enPin) {
  _dinPin    = dinPin;
  _clkPin    = clkPin;
  _csPin     = csPin;
  _enPin     = enPin;
  _brightness = VFD_BRIGHTNESS_MAX;
}

// ─────────────────────────────────────────────
// Private: write one byte LSB first
// ─────────────────────────────────────────────
void FutabaVFD::_writeByte(uint8_t data) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(_clkPin, LOW);
    digitalWrite(_dinPin, (data >> i) & 0x01);
    digitalWrite(_clkPin, HIGH);
  }
}

// ─────────────────────────────────────────────
// Private: write to an icon register
// ─────────────────────────────────────────────
void FutabaVFD::_setIcon(uint8_t addr, uint8_t val) {
  digitalWrite(_csPin, LOW);
  _writeByte(addr);
  _writeByte(val);
  digitalWrite(_csPin, HIGH);
  delayMicroseconds(5);
}

// ─────────────────────────────────────────────
// Private: restore brightness after icon writes
// ─────────────────────────────────────────────
void FutabaVFD::_restoreBrightness() {
  digitalWrite(_csPin, LOW);
  _writeByte(0xe4);
  delayMicroseconds(5);
  _writeByte(_brightness);
  digitalWrite(_csPin, HIGH);
  delayMicroseconds(5);
}

// ─────────────────────────────────────────────
// Public low level
// ─────────────────────────────────────────────
void FutabaVFD::sendCommand(uint8_t cmd) {
  digitalWrite(_csPin, LOW);
  _writeByte(cmd);
  digitalWrite(_csPin, HIGH);
  delayMicroseconds(5);
}

void FutabaVFD::sendData(uint8_t addr, uint8_t data) {
  _setIcon(addr, data);
}

// ─────────────────────────────────────────────
// Initialisation
// ─────────────────────────────────────────────
void FutabaVFD::begin() {
  pinMode(_dinPin, OUTPUT);
  pinMode(_clkPin, OUTPUT);
  pinMode(_csPin,  OUTPUT);
  pinMode(_enPin,  OUTPUT);

  digitalWrite(_csPin,  HIGH);
  digitalWrite(_clkPin, HIGH);

  // Hard reset
  digitalWrite(_enPin, LOW);
  delay(100);
  digitalWrite(_enPin, HIGH);
  delay(100);

  // Set to 12 digit mode - required to unlock all icon addresses
  digitalWrite(_csPin, LOW);
  _writeByte(0xe0);
  delayMicroseconds(5);
  _writeByte(0x0B);
  digitalWrite(_csPin, HIGH);
  delay(10);

  // Set brightness to max
  setBrightness(VFD_BRIGHTNESS_MAX);

  // Display on
  digitalWrite(_csPin, LOW);
  _writeByte(0xe8);
  digitalWrite(_csPin, HIGH);
  delay(10);

  // Clear display
  clear();
  allIconsOff();
}

// ─────────────────────────────────────────────
// Display
// ─────────────────────────────────────────────
void FutabaVFD::print(uint8_t pos, const char* text) {
  digitalWrite(_csPin, LOW);
  _writeByte(0x20 + pos);
  while (*text) {
    _writeByte(*text);
    text++;
  }
  digitalWrite(_csPin, HIGH);
  delayMicroseconds(5);

  // Trigger display refresh
  digitalWrite(_csPin, LOW);
  _writeByte(0xe8);
  digitalWrite(_csPin, HIGH);
  delayMicroseconds(5);
}

void FutabaVFD::clear() {
  print(0, "            "); // 12 spaces
}

// ─────────────────────────────────────────────
// Brightness
// ─────────────────────────────────────────────
void FutabaVFD::setBrightness(uint8_t level) {
  // Clamp to safe range
  if (level < VFD_BRIGHTNESS_MIN) level = VFD_BRIGHTNESS_MIN;
  _brightness = level;
  digitalWrite(_csPin, LOW);
  _writeByte(0xe4);
  delayMicroseconds(5);
  _writeByte(_brightness);
  digitalWrite(_csPin, HIGH);
  delay(10);
}

// ─────────────────────────────────────────────
// Icons - by index
// ─────────────────────────────────────────────
void FutabaVFD::iconOn(uint8_t iconIndex) {
  if (iconIndex >= VFD_NUM_ICONS) return;
  _setIcon(_iconMap[iconIndex][0], _iconMap[iconIndex][1]);
  _restoreBrightness();
}

void FutabaVFD::iconOff(uint8_t iconIndex) {
  if (iconIndex >= VFD_NUM_ICONS) return;
  _setIcon(_iconMap[iconIndex][0], 0x00);
  _restoreBrightness();
}

void FutabaVFD::allIconsOn() {
  for (uint8_t i = 0; i < VFD_NUM_ICONS; i++) {
    _setIcon(_iconMap[i][0], _iconMap[i][1]);
  }
  _restoreBrightness();
}

void FutabaVFD::allIconsOff() {
  for (uint8_t addr = 0x60; addr <= 0x6B; addr++) {
    _setIcon(addr, 0x00);
  }
  _restoreBrightness();
}

void FutabaVFD::cascadeIconsOn(uint16_t delayMs) {
  for (uint8_t i = 0; i < VFD_NUM_ICONS; i++) {
    iconOn(i);
    delay(delayMs);
  }
}

void FutabaVFD::cascadeIconsOff(uint16_t delayMs) {
  for (uint8_t i = 0; i < VFD_NUM_ICONS; i++) {
    iconOff(i);
    delay(delayMs);
  }
}

// ─────────────────────────────────────────────
// Icons - named shortcuts
// ─────────────────────────────────────────────
void FutabaVFD::showUSB(bool on)         { on ? iconOn(VFD_ICON_USB)         : iconOff(VFD_ICON_USB); }
void FutabaVFD::showHD(bool on)          { on ? iconOn(VFD_ICON_HD)          : iconOff(VFD_ICON_HD); }
void FutabaVFD::showHDD(bool on)         { on ? iconOn(VFD_ICON_HDD)         : iconOff(VFD_ICON_HDD); }
void FutabaVFD::showDolby(bool on)       { on ? iconOn(VFD_ICON_DOLBY)       : iconOff(VFD_ICON_DOLBY); }
void FutabaVFD::showMP3(bool on)         { on ? iconOn(VFD_ICON_MP3)         : iconOff(VFD_ICON_MP3); }
void FutabaVFD::showMute(bool on)        { on ? iconOn(VFD_ICON_MUTE)        : iconOff(VFD_ICON_MUTE); }
void FutabaVFD::showRewind(bool on)      { on ? iconOn(VFD_ICON_REWIND)      : iconOff(VFD_ICON_REWIND); }
void FutabaVFD::showPlay(bool on)        { on ? iconOn(VFD_ICON_PLAY)        : iconOff(VFD_ICON_PLAY); }
void FutabaVFD::showPause(bool on)       { on ? iconOn(VFD_ICON_PAUSE)       : iconOff(VFD_ICON_PAUSE); }
void FutabaVFD::showFastForward(bool on) { on ? iconOn(VFD_ICON_FASTFORWARD) : iconOff(VFD_ICON_FASTFORWARD); }
void FutabaVFD::showREC(bool on)         { on ? iconOn(VFD_ICON_REC)         : iconOff(VFD_ICON_REC); }
void FutabaVFD::showClock(bool on)       { on ? iconOn(VFD_ICON_CLOCK)       : iconOff(VFD_ICON_CLOCK); }

void FutabaVFD::showColons(bool on) {
  on ? iconOn(VFD_ICON_COLON_LEFT)  : iconOff(VFD_ICON_COLON_LEFT);
  on ? iconOn(VFD_ICON_COLON_RIGHT) : iconOff(VFD_ICON_COLON_RIGHT);
}

// ─────────────────────────────────────────────
// 5x7 font bitmap table (96 printable ASCII chars, space=0x20)
// Each char = 5 bytes, one byte per column, bit0=top bit6=bottom
// ─────────────────────────────────────────────
static const uint8_t FONT5X7[][5] PROGMEM = {
  {0x00,0x00,0x00,0x00,0x00}, // space
  {0x00,0x00,0x5F,0x00,0x00}, // !
  {0x00,0x07,0x00,0x07,0x00}, // "
  {0x14,0x7F,0x14,0x7F,0x14}, // #
  {0x24,0x2A,0x7F,0x2A,0x12}, // $
  {0x23,0x13,0x08,0x64,0x62}, // %
  {0x36,0x49,0x55,0x22,0x50}, // &
  {0x00,0x05,0x03,0x00,0x00}, // '
  {0x00,0x1C,0x22,0x41,0x00}, // (
  {0x00,0x41,0x22,0x1C,0x00}, // )
  {0x08,0x2A,0x1C,0x2A,0x08}, // *
  {0x08,0x08,0x3E,0x08,0x08}, // +
  {0x00,0x50,0x30,0x00,0x00}, // ,
  {0x08,0x08,0x08,0x08,0x08}, // -
  {0x00,0x60,0x60,0x00,0x00}, // .
  {0x20,0x10,0x08,0x04,0x02}, // /
  {0x3E,0x51,0x49,0x45,0x3E}, // 0
  {0x00,0x42,0x7F,0x40,0x00}, // 1
  {0x42,0x61,0x51,0x49,0x46}, // 2
  {0x21,0x41,0x45,0x4B,0x31}, // 3
  {0x18,0x14,0x12,0x7F,0x10}, // 4
  {0x27,0x45,0x45,0x45,0x39}, // 5
  {0x3C,0x4A,0x49,0x49,0x30}, // 6
  {0x01,0x71,0x09,0x05,0x03}, // 7
  {0x36,0x49,0x49,0x49,0x36}, // 8
  {0x06,0x49,0x49,0x29,0x1E}, // 9
  {0x00,0x36,0x36,0x00,0x00}, // :
  {0x00,0x56,0x36,0x00,0x00}, // ;
  {0x00,0x08,0x14,0x22,0x41}, // <
  {0x14,0x14,0x14,0x14,0x14}, // =
  {0x41,0x22,0x14,0x08,0x00}, // >
  {0x02,0x01,0x51,0x09,0x06}, // ?
  {0x32,0x49,0x79,0x41,0x3E}, // @
  {0x7E,0x11,0x11,0x11,0x7E}, // A
  {0x7F,0x49,0x49,0x49,0x36}, // B
  {0x3E,0x41,0x41,0x41,0x22}, // C
  {0x7F,0x41,0x41,0x22,0x1C}, // D
  {0x7F,0x49,0x49,0x49,0x41}, // E
  {0x7F,0x09,0x09,0x01,0x01}, // F
  {0x3E,0x41,0x41,0x49,0x7A}, // G
  {0x7F,0x08,0x08,0x08,0x7F}, // H
  {0x00,0x41,0x7F,0x41,0x00}, // I
  {0x20,0x40,0x41,0x3F,0x01}, // J
  {0x7F,0x08,0x14,0x22,0x41}, // K
  {0x7F,0x40,0x40,0x40,0x40}, // L
  {0x7F,0x02,0x04,0x02,0x7F}, // M
  {0x7F,0x04,0x08,0x10,0x7F}, // N
  {0x3E,0x41,0x41,0x41,0x3E}, // O
  {0x7F,0x09,0x09,0x09,0x06}, // P
  {0x3E,0x41,0x51,0x21,0x5E}, // Q
  {0x7F,0x09,0x19,0x29,0x46}, // R
  {0x46,0x49,0x49,0x49,0x31}, // S
  {0x01,0x01,0x7F,0x01,0x01}, // T
  {0x3F,0x40,0x40,0x40,0x3F}, // U
  {0x1F,0x20,0x40,0x20,0x1F}, // V
  {0x7F,0x20,0x18,0x20,0x7F}, // W
  {0x63,0x14,0x08,0x14,0x63}, // X
  {0x03,0x04,0x78,0x04,0x03}, // Y
  {0x61,0x51,0x49,0x45,0x43}, // Z
  {0x00,0x00,0x7F,0x41,0x00}, // [
  {0x02,0x04,0x08,0x10,0x20}, // backslash
  {0x00,0x41,0x7F,0x00,0x00}, // ]
  {0x04,0x02,0x01,0x02,0x04}, // ^
  {0x40,0x40,0x40,0x40,0x40}, // _
  {0x00,0x01,0x02,0x04,0x00}, // `
  {0x20,0x54,0x54,0x54,0x78}, // a
  {0x7F,0x48,0x44,0x44,0x38}, // b
  {0x38,0x44,0x44,0x44,0x20}, // c
  {0x38,0x44,0x44,0x48,0x7F}, // d
  {0x38,0x54,0x54,0x54,0x18}, // e
  {0x08,0x7E,0x09,0x01,0x02}, // f
  {0x08,0x54,0x54,0x54,0x3C}, // g
  {0x7F,0x08,0x04,0x04,0x78}, // h
  {0x00,0x44,0x7D,0x40,0x00}, // i
  {0x20,0x40,0x44,0x3D,0x00}, // j
  {0x7F,0x10,0x28,0x44,0x00}, // k
  {0x00,0x41,0x7F,0x40,0x00}, // l
  {0x7C,0x04,0x18,0x04,0x78}, // m
  {0x7C,0x08,0x04,0x04,0x78}, // n
  {0x38,0x44,0x44,0x44,0x38}, // o
  {0x7C,0x14,0x14,0x14,0x08}, // p
  {0x08,0x14,0x14,0x18,0x7C}, // q
  {0x7C,0x08,0x04,0x04,0x08}, // r
  {0x48,0x54,0x54,0x54,0x20}, // s
  {0x04,0x3F,0x44,0x40,0x20}, // t
  {0x3C,0x40,0x40,0x40,0x7C}, // u
  {0x1C,0x20,0x40,0x20,0x1C}, // v
  {0x3C,0x40,0x30,0x40,0x3C}, // w
  {0x44,0x28,0x10,0x28,0x44}, // x
  {0x0C,0x50,0x50,0x50,0x3C}, // y
  {0x44,0x64,0x54,0x4C,0x44}, // z
  {0x00,0x08,0x36,0x41,0x00}, // {
  {0x00,0x00,0x7F,0x00,0x00}, // |
  {0x00,0x41,0x36,0x08,0x00}, // }
  {0x08,0x04,0x08,0x10,0x08}, // ~
  {0x00,0x00,0x00,0x00,0x00}, // del
};

// ─────────────────────────────────────────────
// Get 5 column bytes for a character
// ─────────────────────────────────────────────
void FutabaVFD::_getCharColumns(char c, uint8_t cols[5]) {
  if (c < 0x20 || c > 0x7F) c = 0x20; // default to space
  memcpy_P(cols, FONT5X7[c - 0x20], 5);
}

// ─────────────────────────────────────────────
// Write a custom character into slot (0-7)
// ─────────────────────────────────────────────
void FutabaVFD::_writeCustomChar(uint8_t slot, uint8_t cols[5]) {
  digitalWrite(_csPin, LOW);
  _writeByte(0x40 + slot); // custom char address
  for (int i = 0; i < 5; i++) {
    _writeByte(cols[i]);
  }
  digitalWrite(_csPin, HIGH);
  delayMicroseconds(5);

  // Trigger display refresh
  digitalWrite(_csPin, LOW);
  _writeByte(0xe8);
  digitalWrite(_csPin, HIGH);
  delayMicroseconds(5);
}

// ─────────────────────────────────────────────
// Print raw char codes (including custom 0x00-0x07)
// ─────────────────────────────────────────────
void FutabaVFD::_printRaw(uint8_t pos, uint8_t* charCodes, uint8_t len) {
  digitalWrite(_csPin, LOW);
  _writeByte(0x20 + pos);
  for (int i = 0; i < len; i++) {
    _writeByte(charCodes[i]);
  }
  digitalWrite(_csPin, HIGH);

  digitalWrite(_csPin, LOW);
  _writeByte(0xe8);
  digitalWrite(_csPin, HIGH);
  delayMicroseconds(5);
}

// ─────────────────────────────────────────────
// Smooth pixel-level scroll
// Scrolls text across the display one column at a time
// speedMs = delay between column shifts in milliseconds
// ─────────────────────────────────────────────
void FutabaVFD::smoothScroll(const char* text, uint16_t speedMs) {
  int textLen = strlen(text);

  // Build a full pixel buffer: each char = 5 cols + 1 gap col
  // Total columns = textLen * 6
  int totalCols = textLen * 6;
  uint8_t* pixBuf = (uint8_t*)malloc(totalCols);
  if (!pixBuf) return;

  // Fill pixel buffer from font
  for (int i = 0; i < textLen; i++) {
    uint8_t cols[5];
    _getCharColumns(text[i], cols);
    for (int c = 0; c < 5; c++) {
      pixBuf[i * 6 + c] = cols[c];
    }
    pixBuf[i * 6 + 5] = 0x00; // gap between chars
  }

  // Display is 12 chars wide = 12 * 6 = 72 columns visible
  // We scroll the pixel buffer across the display
  int displayCols = 12 * 6;

  // Start with blank display, scroll text in from the right
  for (int offset = -displayCols; offset < totalCols; offset++) {
    // Build frame: 12 chars worth of columns
    uint8_t frame[72];
    for (int col = 0; col < displayCols; col++) {
      int srcCol = offset + col;
      if (srcCol < 0 || srcCol >= totalCols) {
        frame[col] = 0x00; // blank
      } else {
        frame[col] = pixBuf[srcCol];
      }
    }

    // Now render 12 chars from frame
    // Each char uses 6 columns — we pack them into custom chars
    // Use custom char slot 0 for the leading partial char
    // and print remaining as normal ASCII where possible
    uint8_t charCodes[12];

    for (int charPos = 0; charPos < 12; charPos++) {
      uint8_t* charCols = &frame[charPos * 6];

      // Check if this matches a font character
      bool matched = false;
      for (int fc = 0; fc < 96; fc++) {
        uint8_t fontCols[5];
        memcpy_P(fontCols, FONT5X7[fc], 5);
        if (memcmp(charCols, fontCols, 5) == 0) {
          charCodes[charPos] = fc + 0x20;
          matched = true;
          break;
        }
      }

      if (!matched) {
        // Write as custom char in slot (charPos % 8)
        uint8_t slot = charPos % 8;
        uint8_t cols5[5];
        memcpy(cols5, charCols, 5);
        _writeCustomChar(slot, cols5);
        charCodes[charPos] = slot; // custom chars are 0x00-0x07
      }
    }

    _printRaw(0, charCodes, 12);
    delay(speedMs);
  }

  free(pixBuf);
}
