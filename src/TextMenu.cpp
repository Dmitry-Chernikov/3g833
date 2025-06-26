#include "TextMenu.h"

unsigned long previousMillisMenu = 0;
bool startMenu = false;
unsigned long intervalMenu = 2000;
unsigned long updateMenu = 500;
uint8_t buttons, second = 0;

unsigned long _previousMillisSped = 0;
bool _speeds[] = {false, false, false};

volatile bool IncDecMode = false;

LiquidLine backLine(11, 1, "/BACK");

LiquidLine welcomeLine1(0, 0, "Properties Menu");
LiquidLine welcomeLine2(1, 1, "Machine 3G833");
LiquidScreen welcomeScreen(welcomeLine1, welcomeLine2);

LiquidLine limitsLine(1, 0, "Limits position");
LiquidLine cylinderLine(1, 1, "Cylinder size");
LiquidScreen settingsScreen(limitsLine, cylinderLine);

LiquidMenu mainMenu(_lcd, welcomeScreen, settingsScreen, 1);

LiquidLine linearMoveLine(0, 0, "Current ", _data.linearMove, "mm");
LiquidLine limitTopLine(1, 1, "Top:", _data.limitTop, "mm");
LiquidLine limitBottomLine(1, 1, "Bottom:", _data.limitBottom, "mm");

LiquidScreen topScreen(linearMoveLine, limitTopLine);
LiquidScreen bottomScreen(linearMoveLine, limitBottomLine);

LiquidLine oSaveLine(0, 0, "Save");
LiquidScreen oSecondaryScreen(oSaveLine, backLine);

LiquidMenu limitMenu(_lcd, bottomScreen, topScreen, oSecondaryScreen, 1);

LiquidLine diameterTitleLine(0, 0, "Diameter");
LiquidLine diameterValueLine(1, 1, "Set ", _data.cylinderDiameter, "mm");
LiquidScreen diameterScreen(diameterTitleLine, diameterValueLine);

LiquidLine angleTitleLine(0, 0, "Grid Angle");
LiquidLine angleValueLine(1, 1, "Set ", _data.cylinderAngle, _symbolDegree);
LiquidScreen angleScreen(angleTitleLine, angleValueLine);

LiquidMenu cylinderMenu(_lcd, diameterScreen, angleScreen, oSecondaryScreen);

LiquidSystem menuSystem(mainMenu, limitMenu, cylinderMenu, 1);

void settingTextMenu() {
  backLine.set_focusPosition(Position::LEFT);
  // backLine.attach_function(1, goBack);
  // backLine.attach_function(2, goBack);
  backLine.attach_function(FunctionTypes::Edit, goBack);

  limitsLine.set_focusPosition(Position::LEFT);
  // limitsLine.attach_function(1, gotoLimitMenu);
  // limitsLine.attach_function(2, gotoLimitMenu);
  limitsLine.attach_function(FunctionTypes::Edit, gotoLimitMenu);

  cylinderLine.set_focusPosition(Position::LEFT);
  // cylinderLine.attach_function(1, gotoCylinderMenu);
  // cylinderLine.attach_function(2, gotoCylinderMenu);
  cylinderLine.attach_function(FunctionTypes::Edit, gotoCylinderMenu);

  // mainMenu.switch_focus(true);

  limitTopLine.set_focusPosition(Position::LEFT);
  limitTopLine.attach_function(FunctionTypes::Increase, increaseLimitTop);
  limitTopLine.attach_function(FunctionTypes::Decrease, decreaseLimitTop);
  limitTopLine.attach_function(FunctionTypes::Edit, setLimitTop);

  limitBottomLine.set_focusPosition(Position::LEFT);
  limitBottomLine.attach_function(FunctionTypes::Increase, increaseLimitBottom);
  limitBottomLine.attach_function(FunctionTypes::Decrease, decreaseLimitBottom);
  limitBottomLine.attach_function(FunctionTypes::Edit, setLimitBottom);

  diameterValueLine.set_focusPosition(Position::LEFT);
  diameterValueLine.attach_function(FunctionTypes::Increase, increaseDiameter);
  diameterValueLine.attach_function(FunctionTypes::Decrease, decreaseDiameter);
  diameterValueLine.attach_function(FunctionTypes::Edit, modeEditValue);

  angleValueLine.set_focusPosition(Position::LEFT);
  angleValueLine.set_decimalPlaces(0); // Количество цифр после запятой в значении линии
  angleValueLine.attach_function(FunctionTypes::Increase, increaseAngle);
  angleValueLine.attach_function(FunctionTypes::Decrease, decreaseAngle);
  angleValueLine.attach_function(FunctionTypes::Edit, modeEditValue);
}

///////////////////////////Процедуры меню begin///////////////////////////////////
// Функция для проверки выхода за пределы
bool isOutOfBounds(float paramManipulation, float maxParam, float minParam) {
  // Serial.println(maxParam - paramManipulation);
  return (maxParam - paramManipulation) < 0 || (paramManipulation - minParam) < 0;
}

// Функция для обновления сообщения об ошибке
void updateErrorMessage(Adafruit_RGBLCDShield lcd, String mesLineOne, String mesLineTwo, uint8_t posLineOne, uint8_t posLineTwo, DecIncrTypes typeOperation, float &paramManipulation, float maxParam,
                        float minParam, String messInc, String messDec) {

  mesLineTwo = (typeOperation == DecIncrTypes::Inc) ? messInc : messDec;
  mesLineTwo += (paramManipulation = (typeOperation == DecIncrTypes::Inc) ? maxParam : minParam);
  lcdPrintString(mesLineOne, mesLineTwo, "", RED, GREEN, posLineOne, posLineTwo, 2000, true, false);
  menuSystem.update();
}

// Функция для проверки достижения порога
bool hasReachedThreshold(float paramManipulation, float maxParam, float minParam, float threshold, DecIncrTypes typeOperation) {
  return (typeOperation == DecIncrTypes::Inc) ? paramManipulation >= maxParam - threshold : paramManipulation <= minParam + threshold;
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

void changeParamMenu(DecIncrTypes typeOperation, float &paramManipulation, float maxParam, float minParam, bool *speeds, StartLevelSpeed startSpeed, unsigned long &previousMillisSped,
                     const unsigned long *intervals, String messInc, String messDec, uint8_t posLineOne, uint8_t posLineTwo) {

  // Определение массивов для инкрементов и интервалов
  const float increments[] = {0.01, 0.10, 1.00, 10.00};
  const float THRESHOLDS[] = {0.002f, 0.004f, 0.006f};
  const float thresholds[] = {(maxParam - minParam) * THRESHOLDS[0], (maxParam - minParam) * THRESHOLDS[1], (maxParam - minParam) * THRESHOLDS[2]};

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
        if (i > 0)
          speeds[i - 1] = true; // Активируем предыдущую скорость, если порог достигнут
        speeds[i] = false;      // Отключаем текущую скорость
        previousMillisSped = 0; // Сбрасываем время
        break;                  // Выход из цикла
      }

      // Изменение параметра
      paramManipulation += (typeOperation == DecIncrTypes::Inc) ? increments[i + 1] : -increments[i + 1];
      if (i != 2) { // Проверяем, чтобы не выйти за пределы массива
        // Serial.println(i);
        speeds[i] = !(speeds[i + 1] = stateMillisDelay(&previousMillisSped, &intervals[i + 1]));
      }
      break; // Выход из цикла
    }

    // Если ни одна скорость не активна
    if (i == startSpeed && allSpeedsInactive(speeds, 3)) {
      paramManipulation += (typeOperation == DecIncrTypes::Inc) ? increments[i] : -increments[i];
      speeds[i] = stateMillisDelay(&previousMillisSped, &intervals[i]);
      break; // Выход из цикла
    }
  }
}

void goBack() { // Функция обратного вызова, которая будет прикреплена к backLine. Выход в основное меню
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

void gotoLimitMenu() { // Процедура вызывает экран меню Лимиты
  menuSystem.change_menu(limitMenu);

  if (menuSystem.get_currentScreen() == &oSecondaryScreen) {
    menuSystem.change_screen(&bottomScreen);
  }

  menuSystem.set_focusedLine(1);
}

void gotoCylinderMenu() { // Процедура вызывает экран меню Цилиндр
  menuSystem.change_menu(cylinderMenu);

  if (menuSystem.get_currentScreen() == &oSecondaryScreen) {
    menuSystem.change_screen(&diameterScreen);
  }

  menuSystem.set_focusedLine(1);
}

void setLimitTop() { // Процедура копирует значение энкодера в значение верхнего лимита программного концевика
  if (!stateAutoCycleManual && stateStartFeed && !stateTopSlider) {
    if (_data.linearMove > _data.limitBottom) {
      lcdPrintString("ERROR", "TOP > BOTTOM", "", RED, WHITE, 5, 2, 2000, true, false);
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
    IncDecMode = triggerRS(IncDecMode, true, IncDecMode);

    if ((menuSystem.get_currentScreen() == &topScreen) && IncDecMode) {

      _lcd.setBacklight(GREEN);
      menuSystem.set_focusPosition(Position::RIGHT);

    } else {

      _lcd.setBacklight(WHITE);
      menuSystem.set_focusPosition(Position::LEFT);
    }
  }
}

void increaseLimitTop() { // Процедура увеличивает значение верхнего лимита программного концевика
  changeParamMenu(DecIncrTypes::Inc, _data.limitTop, _data.limitBottom - smallestLength, maxVerticalMovementSpindle - largestLength, _speeds, StartLevelSpeed::Speed_1, _previousMillisSped, _intervals,
                  "TOP > ", "TOP < ", 5, 2);
}

void decreaseLimitTop() { // Процедура уменьшает значение верхнего лимита программного концевика
  changeParamMenu(DecIncrTypes::Dec, _data.limitTop, _data.limitBottom - smallestLength, maxVerticalMovementSpindle - largestLength, _speeds, StartLevelSpeed::Speed_1, _previousMillisSped, _intervals,
                  "TOP > ", "TOP < ", 5, 2);
}

void setLimitBottom() { // Процедура копирует значение энкодера в значение нижнего лимита программного концевика
  if (!stateAutoCycleManual && stateStartFeed && !stateTopSlider) {
    if (_data.linearMove < _data.limitTop) {
      lcdPrintString("ERROR", "BOTTOM < TOP", "", RED, WHITE, 5, 2, 2000, true, false);
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
    IncDecMode = triggerRS(IncDecMode, true, IncDecMode);

    if ((menuSystem.get_currentScreen() == &bottomScreen) && IncDecMode) {

      _lcd.setBacklight(GREEN);
      menuSystem.set_focusPosition(Position::RIGHT);

    } else {

      _lcd.setBacklight(WHITE);
      menuSystem.set_focusPosition(Position::LEFT);
    }
  }
}

void increaseLimitBottom() { // Процедура увеличивает значение нижнего лимита программного концевика
  changeParamMenu(DecIncrTypes::Inc, _data.limitBottom, maxVerticalMovementSpindle, _data.limitTop + smallestLength, _speeds, StartLevelSpeed::Speed_1, _previousMillisSped, _intervals, "BOOTOM > ",
                  "BOTTOM < ", 5, 2);
}

void decreaseLimitBottom() { // Процедура уменьшает значение нижнего лимита программного концевика
  changeParamMenu(DecIncrTypes::Dec, _data.limitBottom, maxVerticalMovementSpindle, _data.limitTop + smallestLength, _speeds, StartLevelSpeed::Speed_1, _previousMillisSped, _intervals, "BOOTOM > ",
                  "BOTTOM < ", 5, 2);
}

void modeEditValue() {
  IncDecMode = triggerRS(IncDecMode, true, IncDecMode);

  if ((menuSystem.get_currentScreen() == &diameterScreen) && IncDecMode || (menuSystem.get_currentScreen() == &angleScreen) && IncDecMode) {

    _lcd.setBacklight(GREEN);
    menuSystem.set_focusPosition(Position::RIGHT);

  } else {

    _lcd.setBacklight(WHITE);
    menuSystem.set_focusPosition(Position::LEFT);
  }
}

void increaseDiameter() {
  changeParamMenu(DecIncrTypes::Inc, _data.cylinderDiameter, permissibleDiameter, smallestDiameter, _speeds, StartLevelSpeed::Speed_1, _previousMillisSped, _intervals, "Diametr > ", "Diametr < ", 5,
                  2);
}

void decreaseDiameter() {
  changeParamMenu(DecIncrTypes::Dec, _data.cylinderDiameter, permissibleDiameter, smallestDiameter, _speeds, StartLevelSpeed::Speed_1, _previousMillisSped, _intervals, "Diametr > ", "Diametr < ", 5,
                  2);
}

void increaseAngle() {
  changeParamMenu(DecIncrTypes::Inc, _data.cylinderAngle, maximumScrubbingAngle, minimalScrubbingAngle, _speeds, StartLevelSpeed::Speed_3, _previousMillisSped, _intervals, "Angle > ", "Angle < ", 5,
                  3);
}

void decreaseAngle() {
  changeParamMenu(DecIncrTypes::Dec, _data.cylinderAngle, maximumScrubbingAngle, minimalScrubbingAngle, _speeds, StartLevelSpeed::Speed_3, _previousMillisSped, _intervals, "Angle > ", "Angle < ", 5,
                  3);
}
///////////////////////////Процедуры меню end/////////////////////////////////////

void Menu() {

  while (_lcd.readButtons() > 0 && !startMenu) {
    startMenu = stateMillisDelay(&previousMillisMenu, &intervalMenu);
    lcdPrintString(String(second += 1), "", "", WHITE, NOT_CHANGE_COLOR, 0, 0, 1000, true, true);
  }

  if (startMenu) {
    second = 0;
    lcdPrintString("Start Menu", "", "", NOT_CHANGE_COLOR, NOT_CHANGE_COLOR, 0, 0, 1000, true, false);

    if (!stateAutoCycleManual && stateStartFeed && !stateTopSlider) {
      menuSystem.change_menu(limitMenu);
      menuSystem.change_screen(&bottomScreen);
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

    if (stateTopSlider) { // Если ползун на концевике парковки. Концевик парковки, ползун в верху исходного состояния
      _data.absoluteAngle = 0;
      _data.anglePrevious = angleSensor.rotationRawToAngle(angleSensor.getRawRotation(64, true, false));
    }

    _data.linearMove = getLinearMotion();

    if (buttons = _lcd.readButtons()) {

      if (buttons & BUTTON_UP) {
        if (IncDecMode) {
          delay(10);
          menuSystem.call_function(FunctionTypes::Increase);
        } else {
          delay(500);
          if (menuSystem.get_currentScreen() == &topScreen || menuSystem.get_currentScreen() == &bottomScreen || menuSystem.get_currentScreen() == &diameterScreen ||
              menuSystem.get_currentScreen() == &angleScreen || menuSystem.get_currentScreen() == &oSecondaryScreen) {
            ///
          } else {
            menuSystem.switch_focus(false);
          }
        }
      }

      if (buttons & BUTTON_DOWN) {
        if (IncDecMode) {
          delay(10);
          menuSystem.call_function(FunctionTypes::Decrease);
        } else {
          delay(500);
          if (menuSystem.get_currentScreen() == &topScreen || menuSystem.get_currentScreen() == &bottomScreen || menuSystem.get_currentScreen() == &diameterScreen ||
              menuSystem.get_currentScreen() == &angleScreen || menuSystem.get_currentScreen() == &oSecondaryScreen) {
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

          if (menuSystem.get_currentScreen() == &topScreen || menuSystem.get_currentScreen() == &bottomScreen || menuSystem.get_currentScreen() == &diameterScreen ||
              menuSystem.get_currentScreen() == &angleScreen || menuSystem.get_currentScreen() == &oSecondaryScreen) {

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

          if (menuSystem.get_currentScreen() == &topScreen || menuSystem.get_currentScreen() == &bottomScreen || menuSystem.get_currentScreen() == &diameterScreen ||
              menuSystem.get_currentScreen() == &angleScreen || menuSystem.get_currentScreen() == &oSecondaryScreen) {

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
        menuSystem.call_function(FunctionTypes::Edit);
        menuSystem.update();
      }

      // previousMillisMenu = millis();
      if (!stateAutoCycleManual && stateStartFeed && !stateTopSlider) {
        ////
      } else {
        while (_lcd.readButtons() > 0 && startMenu && !IncDecMode) {
          startMenu = !stateMillisDelay(&previousMillisMenu, &intervalMenu);
          lcdPrintString(String(second += 1), "", "", NOT_CHANGE_COLOR, NOT_CHANGE_COLOR, 0, 0, 1000, true, false);
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
        if (menuSystem.get_currentScreen() == &topScreen || menuSystem.get_currentScreen() == &bottomScreen || menuSystem.get_currentScreen() == &diameterScreen ||
            menuSystem.get_currentScreen() == &angleScreen) {

          // menuSystem.softUpdate();
          menuSystem.update();
        } else {
          menuSystem.softUpdate();
        }
      }

    } else {
      /////////////////////////////////////////////////////EEPROM SAVE///////////////////////////////////////////////////////
      second = 0;
      saveEeprom(_lcd, _dataBuffer, _data);
      lcdPrintString("Close Menu", "", "", WHITE, NOT_CHANGE_COLOR, 0, 0, 1000, true, true);
    }
  }
}
