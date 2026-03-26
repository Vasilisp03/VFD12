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
