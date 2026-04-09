// BasicDemo.ino
// Demonstrates core features of the FutabaVFD library
// Tested with Futaba 012BT231Q1NK VFD module on ESP32 Feather V2

#include "FutabaVFD.h"

// Pin definitions - adjust to match your wiring
#define DIN_PIN 19
#define CLK_PIN 5
#define CS_PIN  27
#define EN_PIN  15

FutabaVFD vfd(DIN_PIN, CLK_PIN, CS_PIN, EN_PIN);

void setup() {
  Serial.begin(115200);
  vfd.begin();

  // Flash all icons twice
  vfd.allIconsOn();  delay(500);
  vfd.allIconsOff(); delay(200);
  vfd.allIconsOn();  delay(500);
  vfd.allIconsOff(); delay(200);

  // Cascade icons on then off
  vfd.cascadeIconsOn(80);
  delay(300);
  vfd.cascadeIconsOff(80);
  delay(300);

  // Print a message
  vfd.print(0, "  HELLO VFD ");
  delay(1000);

  // Show individual named icons
  vfd.showPlay(true);
  delay(500);
  vfd.showREC(true);
  delay(500);
  vfd.showColons(true);
  delay(1000);

  vfd.allIconsOff();
  vfd.print(0, "   READY    ");
  vfd.showPlay(true);
}

void loop() {
  // Blink the clock icon as a heartbeat
  vfd.showClock(true);
  delay(500);
  vfd.showClock(false);
  delay(500);
}
