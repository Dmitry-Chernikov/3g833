#pragma once

#include <Arduino.h>
#include <LiquidMenu.h>

#include "config.h"
#include "TechnicalSpecifications3G833.h"
#include "StatesActuators.h"
#include "MemoryEeprom.h"
#include "VariablesProject.h"
#include "Display.h"
#include "Encoder.h"

unsigned long previousMillisMenu = 0;
bool startMenu = false;
unsigned long intervalMenu = 2000;
unsigned long updateMenu = 500;
uint8_t buttons, second = 0;

char _symbolDegree = (char)223;

unsigned long _previousMillisSped = 0;
const unsigned long _intervals[] = { 3000, 3000, 3000 };
bool _speeds[] = { false, false, false };

//Переменные для инициализации меню, текст, используемый для Меню индикации для линий сохранения.
volatile bool IncDecMode = false;

//Объекты Liquidline может быть использован больше, чем один раз.
LiquidLine backLine(11, 1, "/BACK");


LiquidLine welcomeLine1(0, 0, "Properties Menu");
LiquidLine welcomeLine2(1, 1, "Machine 3G833");
LiquidScreen welcomeScreen(welcomeLine1, welcomeLine2);

//Эти строки направлены на другие меню.
LiquidLine limitsLine(1, 0, "Limits position");
LiquidLine cylinderLine(1, 1, "Cylinder size");
LiquidScreen settingsScreen(limitsLine, cylinderLine);

//Это первое меню.
LiquidMenu mainMenu(_lcd, welcomeScreen, settingsScreen, 1);


LiquidLine linearMoveLine(0, 0, "Current ", _data.linearMove, "mm");
LiquidLine limitTopLine(1, 1, "Top:", _data.limitTop, "mm");
LiquidLine limitBootomLine(1, 1, "Bottom:", _data.limitBottom, "mm");

LiquidScreen topScreen(linearMoveLine, limitTopLine);
LiquidScreen bootomScreen(linearMoveLine, limitBootomLine);

LiquidLine oSaveLine(0, 0, "Save");
LiquidScreen oSecondaryScreen(oSaveLine, backLine);

//Это второе меню.
LiquidMenu limitMenu(_lcd, bootomScreen, topScreen, oSecondaryScreen, 1);

LiquidLine diametrTitleLine(0, 0, "Diameter");
LiquidLine diametrValueLine(1, 1, "Set ", _data.cylinderDiametr, "mm");
LiquidScreen diametrScreen(diametrTitleLine, diametrValueLine);

LiquidLine angleTitleLine(0, 0, "Grid Angle");
LiquidLine angleValueLine(1, 1, "Set ", _data.cylinderAngle, _symbolDegree);
LiquidScreen angleScreen(angleTitleLine, angleValueLine);

// И это последнее третье меню.
LiquidMenu cylinderMenu(_lcd, diametrScreen, angleScreen, oSecondaryScreen);

/*
 * Объект LiquidSystem объединяет объекты LiquidMenu для формирования системы меню.
 * Он обеспечивает те же функции, что и LiquidMenu с добавлением add_menu () и change_menu ().
 */
LiquidSystem menuSystem(mainMenu, limitMenu, cylinderMenu, 1);


void settingTextMenu();


///////////////////////////Процедуры меню begin///////////////////////////////////
// Функция для проверки выхода за пределы
bool isOutOfBounds(float paramManipulation, float maxParam, float minParam);

// Функция для обновления сообщения об ошибке
void updateErrorMessage(Adafruit_RGBLCDShield lcd, String mesLineOne, String mesLineTwo,
                        uint8_t posLineOne, uint8_t posLineTwo,
                        DecIncrTypes typeOperation, float &paramManipulation,
                        float maxParam, float minParam,
                        String messInc, String messDec);

// Функция для проверки достижения порога
bool hasReachedThreshold(float paramManipulation, float maxParam, float minParam, float threshold, DecIncrTypes typeOperation);

// Функция для управления скоростью
void manageSpeed(bool currentSpeed, bool &nextSpeed, unsigned long &previousMillisSped, unsigned long interval);

bool allSpeedsInactive(bool *speeds, int size);

void changeParamMenu(DecIncrTypes typeOperation, float &paramManipulation,
                     float maxParam, float minParam,
                     bool *speeds, StartLevelSpeed startSpeed,
                     unsigned long &previousMillisSped,
                     const unsigned long *intervals,
                     String messInc, String messDec,
                     uint8_t posLineOne, uint8_t posLineTwo);

void goBack();
void gotoLimitMenu();
void gotoCylinderMenu();
void setLimitTop();
void increaseLimitTop();
void decreaseLimitTop();
void setLimitBootom();
void increaseLimitBootom();
void decreaseLimitBootom();
void modeEditValue();
void increaseDiametr();
void decreaseDiametr();
void increaseAngle();
void decreaseAngle();
///////////////////////////Процедуры меню end/////////////////////////////////////

void Menu();
