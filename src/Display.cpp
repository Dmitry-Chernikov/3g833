#include "Display.h"

Adafruit_RGBLCDShield _lcd = Adafruit_RGBLCDShield();

void initDisplay() {
  _lcd.begin(16, 2);
  _lcd.setBacklight(WHITE);
}

void lcdPrintString(Adafruit_RGBLCDShield &lcd, const String &msg, const String &msgData, const String &msgAfterData, const uint8_t colorBefore, const uint8_t colorAfter, const uint8_t posLineOne, const uint8_t posLineTwo,
                    const unsigned long msgDelay, const bool clearBeforeRendering, const bool clearAfterRendering) {
  if (clearBeforeRendering)
    lcd.clear();
  if (colorBefore != -1)
    lcd.setBacklight(colorBefore);
  if (msg != "") {
    lcd.setCursor(posLineOne, 0);
    lcd.print(msg);
  }
  if (msgData != "") {
    lcd.setCursor(posLineTwo, 1);
    lcd.print(msgData);
    if (msgAfterData != "")
      lcd.print(msgAfterData);
  }
  if (msgDelay > 0)
    delay(msgDelay);
  if (colorAfter != -1)
    lcd.setBacklight(colorAfter);
  if (clearAfterRendering)
    lcd.clear();
}

void lcdPrintString(const String &msg, const String &msgData, const String &msgAfterData, const uint8_t colorBefore, const uint8_t colorAfter, const uint8_t posLineOne, const uint8_t posLineTwo, const unsigned long msgDelay, const bool clearBeforeRendering,
                    const bool clearAfterRendering) {
  lcdPrintString(_lcd, msg, msgData, msgAfterData, colorBefore, colorAfter, posLineOne, posLineTwo, msgDelay, clearBeforeRendering, clearAfterRendering);
}