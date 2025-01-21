#include <Arduino.h>

#include <Adafruit_RGBLCDShield.h>
#include <LiquidMenu.h>
#include <EEPROM.h>

#include "config.h"

#include "Display.h"
#include "IOPorts.h"
#include "TechnicalSpecifications3G833.h"
#include "StatesActuators.h"
#include "AllEnumProject.h"
#include "ControlSystem.h"
#include "Encoder.h"

//#include <avr/pgmspace.h>
//#include <util/delay.h>




Adafruit_RGBLCDShield _lcd = Adafruit_RGBLCDShield();



unsigned long previousMillisMenu = 0;
bool startMenu = false;
unsigned long intervalMenu = 2000;
unsigned long updateMenu = 500;
uint8_t buttons, second = 0;

//Переменные для инициализации меню, текст, используемый для Меню индикации для линий сохранения.
volatile bool IncDecMode = false;



//char input_saved[3];
//char output_saved[3];
//char string_saved[] = " *";
//char string_notSaved[] = "  ";

char symbolDegree = (char)223;

unsigned long _previousMillisSped = 0;
const unsigned long _Intervals[] = { 3000, 3000, 3000 };
bool _speeds[] = { false, false, false };

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
LiquidLine angleValueLine(1, 1, "Set ", _data.cylinderAngle, symbolDegree);
LiquidScreen angleScreen(angleTitleLine, angleValueLine);

// И это последнее третье меню.
LiquidMenu cylinderMenu(_lcd, diametrScreen, angleScreen, oSecondaryScreen);

/*
 * Объект LiquidSystem объединяет объекты LiquidMenu для формирования системы меню.
 * Он обеспечивает те же функции, что и LiquidMenu с добавлением add_menu () и change_menu ().
 */
LiquidSystem menuSystem(mainMenu, limitMenu, cylinderMenu, 1);              

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

  mesLineTwo = (typeOperation == DecIncrTypes::Inc) ? messInc : messDec;
  mesLineTwo += (paramManipulation = (typeOperation == DecIncrTypes::Inc) ? maxParam : minParam);
  lcdPrintString(lcd, mesLineOne, mesLineTwo, "", RED, GREEN, posLineOne, posLineTwo, 2000, true, false);  
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

void changeParamMenu(DecIncrTypes typeOperation, float &paramManipulation,
                     float maxParam, float minParam,
                     bool *speeds, StartLevelSpeed startSpeed,
                     unsigned long &previousMillisSped,
                     const unsigned long *Intervals,
                     String messInc, String messDec,
                     uint8_t posLineOne, uint8_t posLineTwo) {

  // Определение массивов для инкрементов и интервалов
  const float Increments[] = { 0.01, 0.10, 1.00, 10.00 };
  const float THRESHOLDS[] = { 0.002f, 0.004f, 0.006f };
  const float Thresholds[] = {
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
      if (hasReachedThreshold(paramManipulation, maxParam, minParam, Thresholds[i], typeOperation)) {
        if (i > 0) speeds[i - 1] = true; // Активируем предыдущую скорость, если порог достигнут
        speeds[i] = false; // Отключаем текущую скорость
        previousMillisSped = 0; // Сбрасываем время
        break; // Выход из цикла
      }

      // Изменение параметра
      paramManipulation += (typeOperation == DecIncrTypes::Inc) ? Increments[i + 1] : -Increments[i + 1];
      if (i != 2) { // Проверяем, чтобы не выйти за пределы массива
        //Serial.println(i);
        speeds[i] = !(speeds[i + 1] = stateMillisDelay(&previousMillisSped, &Intervals[i + 1]));
      }
      break; // Выход из цикла
    }

    // Если ни одна скорость не активна
    if (i == startSpeed && allSpeedsInactive(speeds, 3)) {
      paramManipulation += (typeOperation == DecIncrTypes::Inc) ? Increments[i] : -Increments[i];
      speeds[i] = stateMillisDelay(&previousMillisSped, &Intervals[i]);
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
      _lcd.clear();
      _lcd.setBacklight(RED);
      _lcd.setCursor(5, 0);
      _lcd.print("ERROR");
      _lcd.setCursor(2, 1);
      _lcd.print("TOP > BOOTOM");
      delay(2000);
      _lcd.setBacklight(WHITE);
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
  changeParamMenu(DecIncrTypes::Inc, _data.limitTop,
                  _data.limitBottom - smallestLength, maxVerticalMovementSpindle - largestLength,
                  _speeds, StartLevelSpeed::Speed_1,
                  _previousMillisSped, _Intervals,
                  "TOP > ", "TOP < ", 5, 2);
}

void decreaselimitTop() {  //Процедура уменьшает значение верхнего лимита программмного концевика
  changeParamMenu(DecIncrTypes::Dec, _data.limitTop,
                  _data.limitBottom - smallestLength, maxVerticalMovementSpindle - largestLength,
                  _speeds, StartLevelSpeed::Speed_1,
                  _previousMillisSped, _Intervals,
                  "TOP > ", "TOP < ", 5, 2);
}

void setLimitBootom() {  //Процедура копирует значение энкодера в значение нижнего лимита программмного концевика
  if (!stateAutoCycleManual && stateStartFeed && !stateTopSlider) {
    if (_data.linearMove < _data.limitTop) {
      _lcd.clear();
      _lcd.setBacklight(RED);
      _lcd.setCursor(5, 0);
      _lcd.print("ERROR");
      _lcd.setCursor(2, 1);
      _lcd.print("BOOTOM < TOP");
      delay(2000);
      _lcd.setBacklight(WHITE);
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
  changeParamMenu(DecIncrTypes::Inc, _data.limitBottom,
                  maxVerticalMovementSpindle, _data.limitTop + smallestLength,
                  _speeds, StartLevelSpeed::Speed_1,
                  _previousMillisSped, _Intervals,
                  "BOOTOM > ", "BOOTOM < ", 5, 2);
}

void decrease_limit_bootom() {  //Процедура уменьшает значение нижнего лимита программмного концевика
  changeParamMenu(DecIncrTypes::Dec, _data.limitBottom,
                  maxVerticalMovementSpindle, _data.limitTop + smallestLength,
                  _speeds, StartLevelSpeed::Speed_1,
                  _previousMillisSped, _Intervals,
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
  changeParamMenu(DecIncrTypes::Inc, _data.cylinderDiametr,
                  permissibleDiameter, smallestDiameter,
                  _speeds, StartLevelSpeed::Speed_1,
                  _previousMillisSped, _Intervals,
                  "Diametr > ", "Diametr < ", 5, 2);
}

void decreaseDiametr() {
  changeParamMenu(DecIncrTypes::Dec, _data.cylinderDiametr,
                  permissibleDiameter, smallestDiameter,
                  _speeds, StartLevelSpeed::Speed_1,
                  _previousMillisSped, _Intervals,
                  "Diametr > ", "Diametr < ", 5, 2);
}

void increaseAngle() {
  changeParamMenu(DecIncrTypes::Inc, _data.cylinderAngle,
                  maximumScrubbingAngle, minimalScrubbingAngle,
                  _speeds, StartLevelSpeed::Speed_3,
                  _previousMillisSped, _Intervals,
                  "Angle > ", "Angle < ", 5, 3);
}

void decreaseAngle() {
  changeParamMenu(DecIncrTypes::Dec, _data.cylinderAngle,
                  maximumScrubbingAngle, minimalScrubbingAngle,
                  _speeds, StartLevelSpeed::Speed_3,
                  _previousMillisSped, _Intervals,
                  "Angle > ", "Angle < ", 5, 3);
}
///////////////////////////Процедуры меню end/////////////////////////////////////



#ifdef ENABLE_KYPAD
void pciSetup(byte pin) {
  //*Pin Change Interrupt Прерывание по изменению вывода*//
  // PCICR Регистр управления PCINT прерываниями, имеются три группы PCI0..2 в которые входят выводы по 8 штук
  // PCIFR Регистр флагов сработавших прерываний PCINT, показывает в какой группе сработало прерывание от вывода
  // PCMSKx Регистр маскировки прерываний каждого из портов для групп PCI0..2, активирует прерывание по выводу
  // 1 Задать обработчик для соответствующего прерывания PCINT, используя макрос ISR.
  // 2 Разрешить генерацию прерываний интересующим выводом микроконтроллера (регистр группы PCMSKx).
  // 3 Разрешить обработку прерывания PCINT, которое генерирует интересующий вывод (регистр PCICR).
  // 4 Установить бит I, разрешающий обработку прерываний глобально (регистр SREG).
  //PCMSK0 |= 1 << 6;
  *digitalPinToPCMSK(pin) |= bit(digitalPinToPCMSKbit(pin));  // Разрешаем PCINT для указанного пина
  //PCIFR |= 0 << 0
  PCIFR |= bit(digitalPinToPCICRbit(pin));  // Очищаем признак запроса прерывания для соответствующей группы пинов
  //PCICR |= 1 << 0;
  PCICR |= bit(digitalPinToPCICRbit(pin));  // Разрешаем PCINT для соответствующей группы пинов
  SREG |= 1 << SREG_I;                      // бит 7 Разрешить прерывания микроконтроллера
}

void readKeypad() {
  handleButtonStates();
  handleMotorStates();
}
#endif

bool stateMillisDelay(unsigned long* previousMillis, const unsigned long* Interval) {
  //unsigned long currentMillis = millis();

  if (*previousMillis == 0) {
    *previousMillis = millis();
  }

  //проверяем не прошел ли нужный интервал, если прошел то
  if ((millis() - *previousMillis) >= *Interval) {
    // обнуляем предыдущее значение millis()
    *previousMillis = 0;
    return HIGH;
  }
  return LOW;
}




/*****SERIAL*****/
// #define TYPES_READ_CMD 3   //Считывание параметров ведомого
// #define TYPES_WRITE_CMD 6  //Запись параметров ведомого
// #define TYPES_READ_CMD 10  //Непрерывная запись набора параметров

// const int ADRES_DATA_PARAM = 0x000D;
// const int ADRES_DATA_STATE = 0x0070;
// const int ADRES_FAULTY_DESCRIPTION = 0x0080;
/*****SERIAL*****/

void setup() {
  cli();

  #ifdef ENABLE_KYPAD
    pinMode(interruptRemote, INPUT_PULLUP);  // Подтянем пины источники PCINT к питанию
    pciSetup(interruptRemote);               // И разрешим на них прерывания T6
  #endif

  initSetupInputManipulation();

  /*****SERIAL*****/
  Serial.begin(9600);
  //Serial1.begin(9600);  // Использовать Serial1 (TX1 >> D18 , RX1 >> D19)
  //pinMode(rs485TransceivReceive, OUTPUT);
  //digitalWrite(rs485TransceivReceive, false);

  #ifdef ENABLE_KYPAD
    readKeypad();
  #endif

  /////////////Инициализация энкодера/////////////
  initEncoder();
  //angleSensor.init();
  // data.absoluteAngle = 0;
  // data.anglePrevious = angleSensor.RotationRawToAngle(angleSensor.getRawRotation(true, 64));

  _lcd.begin(16, 2);
  _lcd.setBacklight(WHITE);

  #ifdef CLEAR_EEPROM
    lcdPrintString(_lcd, "CLEAR EEPROM", "", "", NOT_CHANGE_COLOR, NOT_CHANGE_COLOR, 0, 0, 0, true, false);

    uint8_t indexLine = 0;
    uint16_t compareParam = EEPROM.length() / 16;

    for (int i = 0; i < EEPROM.length(); i++) {
      EEPROM.write(i, 0);
      if (i = compareParam) {
        lcdPrintString(_lcd, "", "0", "", NOT_CHANGE_COLOR, NOT_CHANGE_COLOR, 0, indexLine, 0, false, false);
        compareParam = compareParam + (EEPROM.length() / 16);
        indexLine = indexLine + 1;
      }
    }

    lcdPrintString(_lcd, "CLEAR EEPROM OK", "", "", NOT_CHANGE_COLOR, NOT_CHANGE_COLOR, 0, 0, 1000, true, false);  
  #endif

  #ifdef ENABLE_EEPROM
    EEPROM.get(0, _data);

    if (_data.initData != '*') {
      _data.initData = '*';

      _data.linearMove = 0;
      _data.anglePrevious = 0;
      _data.absoluteAngle = 0;

      _data.limitTop = 10;
      _data.limitBottom = 50;
      _data.cylinderDiametr = 80;
      _data.cylinderAngle = 60;

      _data.stateElectromagnetTop = true;
      _data.stateElectromagnetBottom = true;
      _data.stateIntermediate = true;

      EEPROM.put(0, _data);

      lcdPrintString(_lcd, "INIT EEPROM OK", String(_data.initData), "", NOT_CHANGE_COLOR, NOT_CHANGE_COLOR, 0, 0, 1000, true, true);
    }
  #endif

  backLine.set_focusPosition(Position::LEFT);
  //backLine.attach_function(1, goBack);
  //backLine.attach_function(2, goBack);
  backLine.attach_function(FunctionTypes::Edit, goBack);

  limitsLine.set_focusPosition(Position::LEFT);
  //limitsLine.attach_function(1, gotoLimitMenu);
  //limitsLine.attach_function(2, gotoLimitMenu);
  limitsLine.attach_function(FunctionTypes::Edit, gotoLimitMenu);

  cylinderLine.set_focusPosition(Position::LEFT);
  //cylinderLine.attach_function(1, gotoCylinderMenu);
  //cylinderLine.attach_function(2, gotoCylinderMenu);
  cylinderLine.attach_function(FunctionTypes::Edit, gotoCylinderMenu);

  //mainMenu.switch_focus(true);

  limitTopLine.set_focusPosition(Position::LEFT);
  limitTopLine.attach_function(FunctionTypes::Increase, IncreaselimitTop);
  limitTopLine.attach_function(FunctionTypes::Decrease, decreaselimitTop);
  limitTopLine.attach_function(FunctionTypes::Edit, setlimitTop);

  limitBootomLine.set_focusPosition(Position::LEFT);
  limitBootomLine.attach_function(FunctionTypes::Increase, increaseLimitBootom);
  limitBootomLine.attach_function(FunctionTypes::Decrease, decrease_limit_bootom);
  limitBootomLine.attach_function(FunctionTypes::Edit, setLimitBootom);

  diametrValueLine.set_focusPosition(Position::LEFT);
  diametrValueLine.attach_function(FunctionTypes::Increase, increaseDiametr);
  diametrValueLine.attach_function(FunctionTypes::Decrease, decreaseDiametr);
  diametrValueLine.attach_function(FunctionTypes::Edit, modeEditValue);

  angleValueLine.set_focusPosition(Position::LEFT);
  angleValueLine.set_decimalPlaces(0); //Количество цифр после запятой в значении линии
  angleValueLine.attach_function(FunctionTypes::Increase, increaseAngle);
  angleValueLine.attach_function(FunctionTypes::Decrease, decreaseAngle);
  angleValueLine.attach_function(FunctionTypes::Edit, modeEditValue);

  //strncpy(input_saved, string_saved, sizeof(string_saved));
  //strncpy(output_saved, string_saved, sizeof(string_saved));

  sei();
}

void loop() {

  /*****SERIAL*****/
  // if (Serial1.available()) {
  //   Serial.write(Serial1.read());
  // }

  // if (stateMillisDelay(&previousMillisMenu, &intervalMenu)) {
  //   //SEND//
  //   digitalWrite(rs485TransceivReceive, true);  // переводим модуль в режим передачи данных
  //   delay(10);
  //   Serial1.write(" Data");
  //   Serial1.write(0x0a);

  //   // TO Recieved //
  //   delay(10);
  //   digitalWrite(rs485TransceivReceive, false);  // переводим модуль в режим приёма данных
  // }
  /*****SERIAL*****/

  handleButtonStates();
  handleMotorStates();

  /////////////////////////////////////////////////////ЛОГИКА СОСТОЯНИЯ///////////////////////////////////////////////////////
  if (stateStartFeed) {  // Кнопку подача-пуск нажали. Запускаем мотор возвратно поступательного движения

    handleStartFeed();
    handleProgramSwitch();

    handleAutoCycle();

    handleManualMode();

    #ifdef ENABLE_PROGRAM_SWITCH
      if (!_data.stateIntermediate && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
        lcdPrintString(_lcd, "IN FIELD ACTION", String(_data.linearMove, 2), "mm", YELLOW, NOT_CHANGE_COLOR, 0, 0, 0, true, false);      
      }

      if (_data.stateIntermediate && !_data.stateElectromagnetBottom && stateMillisDelay(&previousMillisMenu, &updateMenu)) {        
        lcdPrintString(_lcd, "LIMIT TOP PROG", String(_data.linearMove, 2), "mm", WHITE, NOT_CHANGE_COLOR, 0, 0, 0, true, false);      
      }

      if (_data.stateIntermediate && !_data.stateElectromagnetTop && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
        lcdPrintString(_lcd, "LIMIT BOOTOM PROG", String(_data.linearMove, 2), "mm", WHITE, NOT_CHANGE_COLOR, 0, 0, 0, true, false);      
      }
    #endif

    #ifdef ENABLE_SWITCH
      if (!digitalRead(endSwitchTop) && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
        lcdPrintString(_lcd, "LIMIT TOP MECHAN", String(_data.linearMove, 2), "mm", YELLOW, NOT_CHANGE_COLOR, 0, 0, 0, true, false);      
      }

      if (!digitalRead(endSwitchBottom) && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
        lcdPrintString(_lcd, "LIMIT BOOTOM MECHAN", String(_data.linearMove, 2), "mm", GREEN, NOT_CHANGE_COLOR, 0, 0, 0, true, false);      
      }
    #endif
  }

  if (!stateStartFeed) {  // Кнопку Общий стоп нажали

    handleStop();

    /////////////////////////////////////////////////////EEPROM SAVE///////////////////////////////////////////////////////
    saveEeprom(_lcd, _dataBuffer, _data);

    /////////////////////////////////////////////////////LCD DISPLAY BUTTONS READ///////////////////////////////////////////////////////
    Menu();
  }

  /////////////////////////////////////////////////////ЦИКЛ///////////////////////////////////////////////////////
  while (stateStartCycle) {  // Включён режим Цикл
    
    handleCycle();

    #ifdef ENABLE_PROGRAM_SWITCH
      if (!_data.stateIntermediate && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
        lcdPrintString(_lcd, "IN FIELD ACTION", String(_data.linearMove, 2), "mm", GREEN, NOT_CHANGE_COLOR, 0, 0, 0, true, false);
      }

      if (_data.stateIntermediate && !_data.stateElectromagnetBottom && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
        lcdPrintString(_lcd, "LIMIT TOP PROG", String(_data.linearMove, 2), "mm", YELLOW, NOT_CHANGE_COLOR, 0, 0, 0, true, false);
      }

      if (_data.stateIntermediate && !_data.stateElectromagnetTop && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
        lcdPrintString(_lcd, "LIMIT BOOTOM PROG", String(_data.linearMove, 2), "mm", YELLOW, NOT_CHANGE_COLOR, 0, 0, 0, true, false);
      }
    #endif

    #ifdef ENABLE_SWITCH
      if (!digitalRead(endSwitchTop) && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
        lcdPrintString(_lcd, "LIMIT TOP MECHAN", String(_data.linearMove, 2), "mm", YELLOW, NOT_CHANGE_COLOR, 0, 0, 0, true, false);
      }

      if (!digitalRead(endSwitchBottom) && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
        lcdPrintString(_lcd, "LIMIT BOOTOM MECHAN", String(_data.linearMove, 2), "mm", GREEN, NOT_CHANGE_COLOR, 0, 0, 0, true, false);
      }
    #endif
    
  }
}

#if defined(ENABLE_KYPAD)
ISR(PCINT0_vect) {  // Обработчик запросов прерывания от пинов PCINT0..PCINT7

  cli();         // сбрасываем флаг прерывания (Запретить прерывания)
  readKeypad();  // вызов процедуры опроса клавиатуры
  sei();         // устанавливаем флаг прерывания (Разрешить прерывания)
}
#endif


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
          menuSystem.call_function(FunctionTypes::Increase);
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
          menuSystem.call_function(FunctionTypes::Decrease);
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
        menuSystem.call_function(FunctionTypes::Edit);
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



template< typename LCD, typename B, typename D >
void saveEeprom(LCD lcd, B &dataBuffer, D &data) {
  EEPROM.get(0, dataBuffer);
  if (data != dataBuffer) {
    EEPROM.put(0, data);  // Сохранение изменений структуры data в EEPROM
    lcdPrintString(lcd, "SAVE EEPROM OK", String(data.initData), "", WHITE, NOT_CHANGE_COLOR, 0, 0, 1000, true, true);
  }
}
