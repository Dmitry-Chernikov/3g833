#pragma once

#include <Adafruit_RGBLCDShield.h>
#include <Arduino.h>

#define NOT_CHANGE_COLOR -1
#define OFF 0
#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define VIOLET 5
#define TEAL 6
#define WHITE 7

extern Adafruit_RGBLCDShield _lcd;

void initDisplay();

void lcdPrintString(Adafruit_RGBLCDShield &lcd, String msg = "", String msgData = "", String msgAfterData = "", uint8_t colorBefore = -1, uint8_t colorAfter = -1, uint8_t posLineOne = 0,
                    uint8_t posLineTwo = 0, unsigned long msgDelay = 0, bool clearBeforeRendering = false, bool clearAfterRendering = false);

void lcdPrintString(String msg = "", String msgData = "", String msgAfterData = "", uint8_t colorBefore = -1, uint8_t colorAfter = -1, uint8_t posLineOne = 0, uint8_t posLineTwo = 0,
                    unsigned long msgDelay = 0, bool clearBeforeRendering = false, bool clearAfterRendering = false);