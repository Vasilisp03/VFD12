#ifndef FUTABAVFD_H
#define FUTABAVFD_H

#include <Arduino.h>

// Icon indices
#define VFD_ICON_USB          0
#define VFD_ICON_HD           1
#define VFD_ICON_HDD          2
#define VFD_ICON_DOLBY        3
#define VFD_ICON_MP3          4
#define VFD_ICON_MUTE         5
#define VFD_ICON_REWIND       6
#define VFD_ICON_PLAY         7
#define VFD_ICON_PAUSE        8
#define VFD_ICON_FASTFORWARD  9
#define VFD_ICON_REC          10
#define VFD_ICON_CLOCK        11
#define VFD_ICON_COLON_LEFT   12
#define VFD_ICON_COLON_RIGHT  13
#define VFD_NUM_ICONS         14

// Brightness limits
#define VFD_BRIGHTNESS_MIN    0x10
#define VFD_BRIGHTNESS_MAX    0xFF

class FutabaVFD {
  public:
    FutabaVFD(uint8_t dinPin, uint8_t clkPin, uint8_t csPin, uint8_t enPin);

    // Initialisation
    void begin();

    // Display
    void print(uint8_t pos, const char* text);
    void clear();

    // Brightness
    void setBrightness(uint8_t level);

    // Icons - by index
    void iconOn(uint8_t iconIndex);
    void iconOff(uint8_t iconIndex);
    void allIconsOn();
    void allIconsOff();
    void cascadeIconsOn(uint16_t delayMs);
    void cascadeIconsOff(uint16_t delayMs);

    // Icons - named shortcuts
    void showUSB(bool on);
    void showHD(bool on);
    void showHDD(bool on);
    void showDolby(bool on);
    void showMP3(bool on);
    void showMute(bool on);
    void showRewind(bool on);
    void showPlay(bool on);
    void showPause(bool on);
    void showFastForward(bool on);
    void showREC(bool on);
    void showClock(bool on);
    void showColons(bool on);

    // Low level access
    void sendCommand(uint8_t cmd);
    void sendData(uint8_t addr, uint8_t data);

  private:
    uint8_t _dinPin;
    uint8_t _clkPin;
    uint8_t _csPin;
    uint8_t _enPin;

    void _writeByte(uint8_t data);
    void _setIcon(uint8_t addr, uint8_t val);
    void _restoreBrightness();

    uint8_t _brightness;

    // Icon map: {address, bit}
    static const uint8_t _iconMap[VFD_NUM_ICONS][2];
};

#endif
