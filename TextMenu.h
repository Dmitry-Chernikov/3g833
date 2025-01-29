#pragma once

#include <Arduino.h>
#include <LiquidMenu.h>

#include "Display.h"
#include "Encoder.h"
#include "MemoryEeprom.h"
#include "StatesActuators.h"
#include "TechnicalSpecifications3G833.h"
#include "VariablesProject.h"
#include "config.h"

const unsigned long _intervals[] = {3000, 3000, 3000};
const char _symbolDegree = (char)223;

extern unsigned long previousMillisMenu;
extern bool startMenu;
extern unsigned long intervalMenu;
extern unsigned long updateMenu;
extern uint8_t buttons, second;

extern unsigned long _previousMillisSped;
extern bool _speeds[];

// Переменные для инициализации меню, текст, используемый для Меню индикации для
// линий сохранения.
extern volatile bool IncDecMode;

// Объекты Liquidline может быть использован больше, чем один раз.
extern LiquidLine backLine;

extern LiquidLine welcomeLine1;
extern LiquidLine welcomeLine2;
extern LiquidScreen welcomeScreen;

// Эти строки направлены на другие меню.
extern LiquidLine limitsLine;
extern LiquidLine cylinderLine;
extern LiquidScreen settingsScreen;

// Это первое меню.
extern LiquidMenu mainMenu;

extern LiquidLine linearMoveLine;
extern LiquidLine limitTopLine;
extern LiquidLine limitBootomLine;

extern LiquidScreen topScreen;
extern LiquidScreen bootomScreen;

extern LiquidLine oSaveLine;
extern LiquidScreen oSecondaryScreen;

// Это второе меню.
extern LiquidMenu limitMenu;

extern LiquidLine diametrTitleLine;
extern LiquidLine diametrValueLine;
extern LiquidScreen diametrScreen;

extern LiquidLine angleTitleLine;
extern LiquidLine angleValueLine;
extern LiquidScreen angleScreen;

// И это последнее третье меню.
extern LiquidMenu cylinderMenu;

/*
 * Объект LiquidSystem объединяет объекты LiquidMenu для формирования системы
 * меню. Он обеспечивает те же функции, что и LiquidMenu с добавлением add_menu
 * () и change_menu ().
 */
extern LiquidSystem menuSystem;

void settingTextMenu();

///////////////////////////Процедуры меню begin///////////////////////////////////
// Функция для проверки выхода за пределы
bool isOutOfBounds(float paramManipulation, float maxParam, float minParam);

// Функция для обновления сообщения об ошибке
void updateErrorMessage(Adafruit_RGBLCDShield lcd, String mesLineOne, String mesLineTwo, uint8_t posLineOne, uint8_t posLineTwo, DecIncrTypes typeOperation, float &paramManipulation, float maxParam,
                        float minParam, String messInc, String messDec);

// Функция для проверки достижения порога
bool hasReachedThreshold(float paramManipulation, float maxParam, float minParam, float threshold, DecIncrTypes typeOperation);

// Функция для управления скоростью
void manageSpeed(bool currentSpeed, bool &nextSpeed, unsigned long &previousMillisSped, unsigned long interval);

bool allSpeedsInactive(bool *speeds, int size);

void changeParamMenu(DecIncrTypes typeOperation, float &paramManipulation, float maxParam, float minParam, bool *speeds, StartLevelSpeed startSpeed, unsigned long &previousMillisSped,
                     const unsigned long *intervals, String messInc, String messDec, uint8_t posLineOne, uint8_t posLineTwo);

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
