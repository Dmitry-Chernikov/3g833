#include "Display.h"

Adafruit_RGBLCDShield _lcd = Adafruit_RGBLCDShield();


void lcdPrintString(Adafruit_RGBLCDShield lcd, String msg, String msgData, String msgAfterData, uint8_t colorBefore, uint8_t colorAfter, uint8_t posLineOne, uint8_t posLineTwo, unsigned long msgDelay, bool clearBeforeRendering, bool clearAfterRendering ) {
  if (clearBeforeRendering) lcd.clear();
  if (colorBefore != -1) lcd.setBacklight(colorBefore);
  if (msg != ""){
    lcd.setCursor(posLineOne, 0);
    lcd.print(msg);
  }
  if (msgData != ""){
    lcd.setCursor(posLineTwo, 1);
    lcd.print(msgData);
    if (msgAfterData != "") lcd.print(msgAfterData);    
  }
  if (msgDelay > 0) delay(msgDelay);
  if (colorAfter != -1) lcd.setBacklight(colorAfter);
  if (clearAfterRendering) lcd.clear();  
}