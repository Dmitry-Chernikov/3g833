#pragma once

#include <Adafruit_RGBLCDShield.h>
#include <LiquidMenu.h>
#include "StatesActuators.h"

Adafruit_RGBLCDShield _lcd = Adafruit_RGBLCDShield();

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

#if defined ENABLE_EEPROM || defined CLEAR_EEPROM
struct Data {
  char initData;

  float linearMove;     //Длина линейного перемещения от концевика порковки от энкодера
  float anglePrevious;  //Угол предыдущий от экодера
  float absoluteAngle;  //Обсалютный угол или инкрементн и декремент угла

  float limitTop;         //Верхняя позиция цикла
  float limitBottom;      //Нижная позиция цикла
  float cylinderDiametr;  //Диаметр обрабатываемого цилиндра
  float cylinderAngle;    //Желаемый угол сетки в цилиндре

  bool stateElectromagnetTop;     //Сохранение состояния муфты движения вверх, если электричество отключили
  bool stateElectromagnetBottom;  //Сохранение состояния муфты движения вниз, если электричество отключили
  bool stateIntermediate;         //Сохранение состояния интервала в режиме цикла, если электричество отключили

  bool operator!=(const Data &otcher) const {
    return initData != otcher.initData ||
          abs(linearMove - otcher.linearMove) > 0.1 ||
          abs(anglePrevious - otcher.anglePrevious) > 0.1 ||
          abs(absoluteAngle - otcher.absoluteAngle) > 0.1 ||
          abs(limitTop - otcher.limitTop) > 0.1 ||
          abs(limitBottom - otcher.limitBottom) > 0.1 ||
          abs(cylinderDiametr - otcher.cylinderDiametr) > 0.1 ||
          abs(cylinderAngle - otcher.cylinderAngle) > 0.1 ||

          stateElectromagnetTop != otcher.stateElectromagnetTop ||
          stateElectromagnetBottom != otcher.stateElectromagnetBottom ||
          stateIntermediate != otcher.stateIntermediate;
  }
};

Data _data;
Data _dataBuffer;  // временная переменная для проверки данных в EEPROM с data, чтобы не писать в EEPROM часто
#endif

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


void settingTextMenu(){
  backLine.set_focusPosition(Position::LEFT);
  //backLine.attach_function(1, goBack);
  //backLine.attach_function(2, goBack);
  backLine.attach_function(FunctionTypes::edit, goBack);

  limitsLine.set_focusPosition(Position::LEFT);
  //limitsLine.attach_function(1, gotoLimitMenu);
  //limitsLine.attach_function(2, gotoLimitMenu);
  limitsLine.attach_function(FunctionTypes::edit, gotoLimitMenu);

  cylinderLine.set_focusPosition(Position::LEFT);
  //cylinderLine.attach_function(1, gotoCylinderMenu);
  //cylinderLine.attach_function(2, gotoCylinderMenu);
  cylinderLine.attach_function(FunctionTypes::edit, gotoCylinderMenu);

  //mainMenu.switch_focus(true);

  limitTopLine.set_focusPosition(Position::LEFT);
  limitTopLine.attach_function(FunctionTypes::increase, IncreaselimitTop);
  limitTopLine.attach_function(FunctionTypes::decrease, decreaselimitTop);
  limitTopLine.attach_function(FunctionTypes::edit, setlimitTop);

  limitBootomLine.set_focusPosition(Position::LEFT);
  limitBootomLine.attach_function(FunctionTypes::increase, increaseLimitBootom);
  limitBootomLine.attach_function(FunctionTypes::decrease, decrease_limit_bootom);
  limitBootomLine.attach_function(FunctionTypes::edit, setLimitBootom);

  diametrValueLine.set_focusPosition(Position::LEFT);
  diametrValueLine.attach_function(FunctionTypes::increase, increaseDiametr);
  diametrValueLine.attach_function(FunctionTypes::decrease, decreaseDiametr);
  diametrValueLine.attach_function(FunctionTypes::edit, modeEditValue);

  angleValueLine.set_focusPosition(Position::LEFT);
  angleValueLine.set_decimalPlaces(0); //Количество цифр после запятой в значении линии
  angleValueLine.attach_function(FunctionTypes::increase, increaseAngle);
  angleValueLine.attach_function(FunctionTypes::decrease, decreaseAngle);
  angleValueLine.attach_function(FunctionTypes::edit, modeEditValue);
}

///////////////////////////Prototype function///////////////////////////
void lcdPrintString(Adafruit_RGBLCDShield lcd, String msg = "", String msgData = "", String msgAfterData = "", 
                    uint8_t colorBefore = -1, uint8_t colorAfter = -1, 
                    uint8_t posLineOne = 0, uint8_t posLineTwo = 0, 
                    unsigned long msgDelay = 0, bool clearBeforeRendering = false, bool clearAfterRendering = false);

///////////////////////////Процедуры меню begin///////////////////////////////////
// Функция для проверки выхода за пределы
bool isOutOfBounds(float paramManipulation, float maxParam, float minParam) {
  //Serial.println(maxParam - paramManipulation);
  return (maxParam - paramManipulation) < 0 || (paramManipulation - minParam) < 0;
}

// Функция для обновления сообщения об ошибке
void updateErrorMessage(Adafruit_RGBLCDShield lcd, String mesLineOne, String mesLineTwo,
                        uint8_t posLineOne, uint8_t posLineTwo,
                        DecIncrTypes typeOperation, float &paramManipulation,
                        float maxParam, float minParam,
                        String messInc, String messDec) {

  mesLineTwo = (typeOperation == DecIncrTypes::inc) ? messInc : messDec;
  mesLineTwo += (paramManipulation = (typeOperation == DecIncrTypes::inc) ? maxParam : minParam);
  lcdPrintString(lcd, mesLineOne, mesLineTwo, "", RED, GREEN, posLineOne, posLineTwo, 2000, true, false);
  menuSystem.update();
}

// Функция для проверки достижения порога
bool hasReachedThreshold(float paramManipulation, float maxParam, float minParam, float threshold, DecIncrTypes typeOperation) {
  return (typeOperation == DecIncrTypes::inc) ? paramManipulation >= maxParam - threshold : paramManipulation <= minParam + threshold;
}

// Функция для управления скоростью
void manageSpeed(bool currentSpeed, bool &nextSpeed, unsigned long &previousMillisSped, unsigned long interval) {
  currentSpeed = stateMillisDelay(&previousMillisSped, &interval);
  nextSpeed = false;
}

bool allSpeedsInactive(bool *speeds, int size) {
    for (int i = 0; i < size; i++) {
        if (speeds[i]) {
            return false; // Если хоть один элемент активен, возвращаем false
        }
    }
    return true; // Все элементы неактивны
}

void changeParamMenu(DecIncrTypes typeOperation, float &paramManipulation,
                     float maxParam, float minParam,
                     bool *speeds, StartLevelSpeed startSpeed,
                     unsigned long &previousMillisSped,
                     const unsigned long *intervals,
                     String messInc, String messDec,
                     uint8_t posLineOne, uint8_t posLineTwo) {

  // Определение массивов для инкрементов и интервалов
  const float increments[] = { 0.01, 0.10, 1.00, 10.00 };
  const float THRESHOLDS[] = { 0.002f, 0.004f, 0.006f };
  const float thresholds[] = {
    (maxParam - minParam) * THRESHOLDS[0],
    (maxParam - minParam) * THRESHOLDS[1],
    (maxParam - minParam) * THRESHOLDS[2]
  };


  // Проверка выхода за пределы
  if (isOutOfBounds(paramManipulation, maxParam, minParam)) {
    String mesLineTwo;
    updateErrorMessage(_lcd, "ERROR", mesLineTwo, posLineOne, posLineTwo, typeOperation, paramManipulation, maxParam, minParam, messInc, messDec);
    return;
  }

  for (int i = startSpeed; i < 3; ++i) {
    if (speeds[i]) {
      // Проверка на достижение порога
      if (hasReachedThreshold(paramManipulation, maxParam, minParam, thresholds[i], typeOperation)) {
        if (i > 0) speeds[i - 1] = true; // Активируем предыдущую скорость, если порог достигнут
        speeds[i] = false; // Отключаем текущую скорость
        previousMillisSped = 0; // Сбрасываем время
        break; // Выход из цикла
      }

      // Изменение параметра
      paramManipulation += (typeOperation == DecIncrTypes::inc) ? increments[i + 1] : -increments[i + 1];
      if (i != 2) { // Проверяем, чтобы не выйти за пределы массива
        //Serial.println(i);
        speeds[i] = !(speeds[i + 1] = stateMillisDelay(&previousMillisSped, &intervals[i + 1]));
      }
      break; // Выход из цикла
    }

    // Если ни одна скорость не активна
    if (i == startSpeed && allSpeedsInactive(speeds, 3)) {
      paramManipulation += (typeOperation == DecIncrTypes::inc) ? increments[i] : -increments[i];
      speeds[i] = stateMillisDelay(&previousMillisSped, &intervals[i]);
      break; // Выход из цикла
    }
  }
}

void goBack() {  // Функция обратного вызова, которая будет прикреплена к backLine. Выход в основное меню
  // Эта функция принимает ссылку на разыскиваемое меню.
  if (!stateAutoCycleManual && stateStartFeed && !stateTopSlider) {
    startMenu = false;
  }

  if (stateGeneralStop) {
    menuSystem.change_menu(mainMenu);
    if (menuSystem.get_currentScreen() == &settingsScreen) {
      menuSystem.set_focusedLine(0);
    }
  }
}

void gotoLimitMenu() {  // Процедура вызывает экран меню Лимиты
  menuSystem.change_menu(limitMenu);

  if (menuSystem.get_currentScreen() == &oSecondaryScreen) {
    menuSystem.change_screen(&bootomScreen);
  }

  menuSystem.set_focusedLine(1);
}

void gotoCylinderMenu() {  // Процедура вызывает экран меню Цилиндр
  menuSystem.change_menu(cylinderMenu);

  if (menuSystem.get_currentScreen() == &oSecondaryScreen) {
    menuSystem.change_screen(&diametrScreen);
  }

  menuSystem.set_focusedLine(1);
}

void setlimitTop() {  //Процедура копирует значение энкодера в значение верхнего лимита программмного концевика
  if (!stateAutoCycleManual && stateStartFeed && !stateTopSlider) {
    if (_data.linearMove > _data.limitBottom) {
      lcdPrintString(_lcd, "ERROR", "TOP > BOOTOM", "", RED, WHITE, 5, 2, 2000, true, false);
      menuSystem.update();
    } else {
      _lcd.setBacklight(GREEN);
      _data.limitTop = _data.linearMove;
      menuSystem.update();
      delay(500);
      _lcd.setBacklight(WHITE);
    }
  }

  if (stateGeneralStop) {
    IncDecMode = trigerRS(IncDecMode, true, IncDecMode);

    if ((menuSystem.get_currentScreen() == &topScreen) && IncDecMode) {

      _lcd.setBacklight(GREEN);
      menuSystem.set_focusPosition(Position::RIGHT);

    } else {

      _lcd.setBacklight(WHITE);
      menuSystem.set_focusPosition(Position::LEFT);
    }
  }
}

void IncreaselimitTop() {  //Процедура увеличевает значение верхнего лимита программмного концевика
  changeParamMenu(DecIncrTypes::inc, _data.limitTop,
                  _data.limitBottom - smallestLength, maxVerticalMovementSpindle - largestLength,
                  _speeds, StartLevelSpeed::SPEED_1,
                  _previousMillisSped, _intervals,
                  "TOP > ", "TOP < ", 5, 2);
}

void decreaselimitTop() {  //Процедура уменьшает значение верхнего лимита программмного концевика
  changeParamMenu(DecIncrTypes::dec, _data.limitTop,
                  _data.limitBottom - smallestLength, maxVerticalMovementSpindle - largestLength,
                  _speeds, StartLevelSpeed::SPEED_1,
                  _previousMillisSped, _intervals,
                  "TOP > ", "TOP < ", 5, 2);
}

void setLimitBootom() {  //Процедура копирует значение энкодера в значение нижнего лимита программмного концевика
  if (!stateAutoCycleManual && stateStartFeed && !stateTopSlider) {
    if (_data.linearMove < _data.limitTop) {
      lcdPrintString(_lcd, "ERROR", "BOOTOM < TOP", "", RED, WHITE, 5, 2, 2000, true, false);    
      menuSystem.update();
    } else {
      _lcd.setBacklight(GREEN);
      _data.limitBottom = _data.linearMove;
      menuSystem.update();
      delay(500);
      _lcd.setBacklight(WHITE);
    }
  }

  if (stateGeneralStop) {
    IncDecMode = trigerRS(IncDecMode, true, IncDecMode);

    if ((menuSystem.get_currentScreen() == &bootomScreen) && IncDecMode) {

      _lcd.setBacklight(GREEN);
      menuSystem.set_focusPosition(Position::RIGHT);

    } else {

      _lcd.setBacklight(WHITE);
      menuSystem.set_focusPosition(Position::LEFT);
    }
  }
}

void increaseLimitBootom() {  //Процедура увеличивает значение нижнего лимита программмного концевика
  changeParamMenu(DecIncrTypes::inc, _data.limitBottom,
                  maxVerticalMovementSpindle, _data.limitTop + smallestLength,
                  _speeds, StartLevelSpeed::SPEED_1,
                  _previousMillisSped, _intervals,
                  "BOOTOM > ", "BOOTOM < ", 5, 2);
}

void decrease_limit_bootom() {  //Процедура уменьшает значение нижнего лимита программмного концевика
  changeParamMenu(DecIncrTypes::dec, _data.limitBottom,
                  maxVerticalMovementSpindle, _data.limitTop + smallestLength,
                  _speeds, StartLevelSpeed::SPEED_1,
                  _previousMillisSped, _intervals,
                  "BOOTOM > ", "BOOTOM < ", 5, 2);
}

void modeEditValue() {
  IncDecMode = trigerRS(IncDecMode, true, IncDecMode);

  if ((menuSystem.get_currentScreen() == &diametrScreen) && IncDecMode
      || (menuSystem.get_currentScreen() == &angleScreen) && IncDecMode) {

    _lcd.setBacklight(GREEN);
    menuSystem.set_focusPosition(Position::RIGHT);

  } else {

    _lcd.setBacklight(WHITE);
    menuSystem.set_focusPosition(Position::LEFT);
  }
}

void increaseDiametr() {
  changeParamMenu(DecIncrTypes::inc, _data.cylinderDiametr,
                  permissibleDiameter, smallestDiameter,
                  _speeds, StartLevelSpeed::SPEED_1,
                  _previousMillisSped, _intervals,
                  "Diametr > ", "Diametr < ", 5, 2);
}

void decreaseDiametr() {
  changeParamMenu(DecIncrTypes::dec, _data.cylinderDiametr,
                  permissibleDiameter, smallestDiameter,
                  _speeds, StartLevelSpeed::SPEED_1,
                  _previousMillisSped, _intervals,
                  "Diametr > ", "Diametr < ", 5, 2);
}

void increaseAngle() {
  changeParamMenu(DecIncrTypes::inc, _data.cylinderAngle,
                  maximumScrubbingAngle, minimalScrubbingAngle,
                  _speeds, StartLevelSpeed::SPEED_3,
                  _previousMillisSped, _intervals,
                  "Angle > ", "Angle < ", 5, 3);
}

void decreaseAngle() {
  changeParamMenu(DecIncrTypes::dec, _data.cylinderAngle,
                  maximumScrubbingAngle, minimalScrubbingAngle,
                  _speeds, StartLevelSpeed::SPEED_3,
                  _previousMillisSped, _intervals,
                  "Angle > ", "Angle < ", 5, 3);
}
///////////////////////////Процедуры меню end/////////////////////////////////////

void Menu() {

  while (_lcd.readButtons() > 0 && !startMenu) {
    startMenu = stateMillisDelay(&previousMillisMenu, &intervalMenu);
    lcdPrintString(_lcd, String(second += 1), "", "", WHITE, NOT_CHANGE_COLOR, 0, 0, 1000, true, true);
  }

  if (startMenu) {
    second = 0;
    lcdPrintString(_lcd, "Start Menu", "", "", NOT_CHANGE_COLOR, NOT_CHANGE_COLOR, 0, 0, 1000, true, false);

    if (!stateAutoCycleManual && stateStartFeed && !stateTopSlider) {
      menuSystem.change_menu(limitMenu);
      menuSystem.change_screen(&bootomScreen);
      menuSystem.set_focusedLine(1);
    }
    if (stateGeneralStop) {
      menuSystem.change_menu(mainMenu);
      menuSystem.change_screen(&welcomeScreen);
    }

  } else {
    if (second > 0) {
      second = 0;
    }
  }

  while (startMenu) {

    if (stateTopSlider) {  // Если ползун на концевике парковки. Концевик парковки, ползун в верху исходного состояния
      _data.absoluteAngle = 0;
      _data.anglePrevious = angleSensor.RotationRawToAngle(angleSensor.getRawRotation(true, 64));
    }

    _data.linearMove = getLinearMotion();

    if (buttons = _lcd.readButtons()) {

      if (buttons & BUTTON_UP) {
        if (IncDecMode) {
          delay(10);
          menuSystem.call_function(FunctionTypes::increase);
        } else {
          delay(500);
          if (menuSystem.get_currentScreen() == &topScreen
              || menuSystem.get_currentScreen() == &bootomScreen
              || menuSystem.get_currentScreen() == &diametrScreen
              || menuSystem.get_currentScreen() == &angleScreen
              || menuSystem.get_currentScreen() == &oSecondaryScreen) {
            ///
          } else {
            menuSystem.switch_focus(false);
          }
        }
      }

      if (buttons & BUTTON_DOWN) {
        if (IncDecMode) {
          delay(10);
          menuSystem.call_function(FunctionTypes::decrease);
        } else {
          delay(500);
          if (menuSystem.get_currentScreen() == &topScreen
              || menuSystem.get_currentScreen() == &bootomScreen
              || menuSystem.get_currentScreen() == &diametrScreen
              || menuSystem.get_currentScreen() == &angleScreen
              || menuSystem.get_currentScreen() == &oSecondaryScreen) {
            ///
          } else {
            menuSystem.switch_focus(true);
          }
        }
      }

      if (buttons & BUTTON_LEFT) {
        delay(500);
        if (IncDecMode) {
          ///
        } else {
          menuSystem.previous_screen();

          if (menuSystem.get_currentScreen() == &topScreen
              || menuSystem.get_currentScreen() == &bootomScreen
              || menuSystem.get_currentScreen() == &diametrScreen
              || menuSystem.get_currentScreen() == &angleScreen
              || menuSystem.get_currentScreen() == &oSecondaryScreen) {

            menuSystem.set_focusedLine(1);
            menuSystem.softUpdate();
          }

          if (menuSystem.get_currentScreen() == &settingsScreen) {
            menuSystem.set_focusedLine(0);
            menuSystem.update();
          }
        }
      }

      if (buttons & BUTTON_RIGHT) {
        delay(500);
        if (IncDecMode) {
          ///
        } else {
          menuSystem.next_screen();

          if (menuSystem.get_currentScreen() == &topScreen
              || menuSystem.get_currentScreen() == &bootomScreen
              || menuSystem.get_currentScreen() == &diametrScreen
              || menuSystem.get_currentScreen() == &angleScreen
              || menuSystem.get_currentScreen() == &oSecondaryScreen) {

            menuSystem.set_focusedLine(1);
            menuSystem.softUpdate();
          }

          if (menuSystem.get_currentScreen() == &settingsScreen) {
            menuSystem.set_focusedLine(0);
            menuSystem.update();
          }
        }
      }

      if (buttons & BUTTON_SELECT) {
        delay(500);
        menuSystem.call_function(FunctionTypes::edit);
        menuSystem.update();
      }

      //previousMillisMenu = millis();
      if (!stateAutoCycleManual && stateStartFeed && !stateTopSlider) {
        ////
      } else {
        while (_lcd.readButtons() > 0 && startMenu && !IncDecMode) {
          startMenu = !stateMillisDelay(&previousMillisMenu, &intervalMenu);
          lcdPrintString(_lcd, String(second += 1), "", "", NOT_CHANGE_COLOR, NOT_CHANGE_COLOR, 0, 0, 1000, true, false);
        }
      }

    } else {
      _speeds[0] = false;
      _speeds[1] = false;
      _speeds[2] = false;
      _previousMillisSped = 0;
    }

    if (startMenu) {
      if (second > 0) {
        second = 0;
        menuSystem.update();
      }

      if (stateMillisDelay(&previousMillisMenu, &updateMenu)) {
        if (menuSystem.get_currentScreen() == &topScreen
            || menuSystem.get_currentScreen() == &bootomScreen
            || menuSystem.get_currentScreen() == &diametrScreen
            || menuSystem.get_currentScreen() == &angleScreen) {

          //menuSystem.softUpdate();
          menuSystem.update();
        } else {
          menuSystem.softUpdate();
        }
      }

    } else {
      /////////////////////////////////////////////////////EEPROM SAVE///////////////////////////////////////////////////////
      second = 0;
      saveEeprom(_lcd, _dataBuffer, _data);
      lcdPrintString(_lcd, "Close Menu", "", "", WHITE, NOT_CHANGE_COLOR, 0, 0, 1000, true, true);
    }
  }
}

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
