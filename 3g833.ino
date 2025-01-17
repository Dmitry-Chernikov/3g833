#include <Arduino.h>
#include <AS5048A.h>
#include <Adafruit_RGBLCDShield.h>
#include <LiquidMenu.h>
#include <EEPROM.h>

#define ENABLE_KYPAD           // Это для моей клавиатуры кода включаю
#define ENABLE_SWITCH          // Включение кода механического переключателя
#define ENABLE_PROGRAM_SWITCH  // Включение кода программного переключателя
#define ENABLE_EEPROM          // Включить код поддержки использования EEPROM и переменно структуры Data и переменной data
//#define CLEAR_EEPROM // Запуск кода очистки всей памяти EEPROM для отладки

#include "ColorDisplay.h"
#include "IOPorts.h"
#include "TechnicalSpecifications3G833.h"



//#include <avr/pgmspace.h>
//#include <util/delay.h>


AS5048A angleSensor(SS);  //выход на Arduino SS = PIN_SPI_SS (53), MOSI = PIN_SPI_MOSI (51), MISO = PIN_SPI_MISO (50), SCK = PIN_SPI_SCK (52)

Adafruit_RGBLCDShield _lcd = Adafruit_RGBLCDShield();

//Состояние режимов
volatile bool stateEndCycle = false;         // Состояние Конец-Цикла
volatile bool statePush = false;             // Состояние Толчок-Ползун
volatile bool stateStartFeed = false;        // Состояние Подача-Пуск
volatile bool stateSpindle = false;          // Состояние Шпиндель-Старт\Стоп
volatile bool stateAutoCycleManual = false;  // Состояние Цикл-Ручной
volatile bool stateTopSlider = false;        // Состояние концевика верхнего положения ползуна
volatile bool stateStartCycle = false;       // Состояние Цикла
volatile bool stateGeneralStop = true;       // Состояние Общий Стоп

//Переменные энкодера
const uint8_t normalModule = 3;      //Модуль нормальны
const uint8_t numberGearTeeth = 17;  //Число зубьев колеса или число заходов червяка

unsigned long previousMillisMenu = 0;
bool startMenu = false;
unsigned long intervalMenu = 2000;
unsigned long updateMenu = 500;
uint8_t buttons, second = 0;

//Переменные для инициализации меню, текст, используемый для Меню индикации для линий сохранения.
volatile bool IncDecMode = false;

enum FunctionTypes : uint8_t {
  increase = 1,
  decrease = 2,
  edit = 3
};

enum DecIncrTypes : uint8_t {
  inc = 1,
  dec = 2
};

enum StartLevelSpeed : uint8_t {
    SPEED_1 = 0,  // Первая скорость
    SPEED_2,      // Вторая скорость
    SPEED_3       // Третья скорость
};

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

//char input_saved[3];
//char output_saved[3];
//char string_saved[] = " *";
//char string_notSaved[] = "  ";

char symbolDegree = (char)223;

unsigned long _previousMillisSped = 0;
const unsigned long _intervals[] = { 3000, 3000, 3000 };
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

///////////////////////////Prototype function///////////////////////////
void lcdPrintString(Adafruit_RGBLCDShield lcd, String msg = "", String dataChar = "", String dataCharPostfix = "", uint8_t color = -1, uint8_t posLineOne = 0, uint8_t posLineTwo = 0, unsigned long msgDelay = 0, bool clearBeforeRendering = false, bool clearAfterRendering = false);

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
  lcdPrintString(lcd, mesLineOne, mesLineTwo, "", RED, posLineOne, posLineTwo, 2000, true, false);
  lcd.setBacklight(GREEN);
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

bool trigerRS(bool currentState, uint8_t TrigSet, uint8_t TrigReset) {  //Триггер с приоритетом сброса
  if (TrigReset) { return false; }
  if (TrigSet) { return true; }
  return currentState;
}

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
  stateGeneralStop = trigerRS(stateGeneralStop, !digitalRead(buttonGeneralStop), !digitalRead(buttonStartFeed));                   // Общий стоп
  stateStartFeed = trigerRS(stateStartFeed, !digitalRead(buttonStartFeed), !digitalRead(buttonGeneralStop));                       // Подача-пуск
  digitalWrite(motorStartFeed, !stateStartFeed);                                                                                   // Запускаем мотор возвратно поступательного движения
  stateAutoCycleManual = trigerRS(stateAutoCycleManual, digitalRead(switchAutoCycleManual), !digitalRead(switchAutoCycleManual));  // Переключатель режимов: "Ввод хоны", "Ручной"
  stateTopSlider = trigerRS(stateTopSlider, digitalRead(switchTopSlider), !digitalRead(switchTopSlider));                          // Концевик парковки ползуна в верху исходного состояния
  stateEndCycle = trigerRS(stateEndCycle, !digitalRead(buttonEndCycle), stateGeneralStop);                                         // Конец цикла
}
#endif

bool stateMillisDelay(unsigned long *previousMillis, const unsigned long *interval) {
  //unsigned long currentMillis = millis();

  if (*previousMillis == 0) {
    *previousMillis = millis();
  }

  //проверяем не прошел ли нужный интервал, если прошел то
  if ((millis() - *previousMillis) >= *interval) {
    // обнуляем предыдущее значение millis()
    *previousMillis = 0;
    return HIGH;
  }
  return LOW;
}

float getLinearMotion() {
  return angleSensor.LinearDisplacementRack(angleSensor.AbsoluteAngleRotation(&_data.absoluteAngle, angleSensor.RotationRawToAngle(angleSensor.getRawRotation(true, 64)), &_data.anglePrevious), normalModule, numberGearTeeth);

  // _lcd.clear();
  // _lcd.setCursor(0, 0);
  // _lcd.print(getLinearMotion(), 4);
  // _lcd.print(" mm");

  // _lcd.setCursor(0, 1);
  // //_lcd.print(val, DEC);

  // //_lcd.print(angleSensor.RotationRawToRadian(angleSensor.getRawRotation(true)), DEC);
  // _lcd.print(int(absoluteAngle), DEC);  //_lcd.print(millis()/1000);
  // _lcd.print(char(223));

  // _lcd.print(int(angleSensor.GetAngularMinutes(absoluteAngle)), DEC);  //_lcd.print(millis()/1000);
  // _lcd.print(char(34));

  // _lcd.print(int(angleSensor.GetAngularSeconds(absoluteAngle)), DEC);
  // _lcd.print(char(39));
  // _lcd.print("  ");
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
  angleSensor.init();
  // data.absoluteAngle = 0;
  // data.anglePrevious = angleSensor.RotationRawToAngle(angleSensor.getRawRotation(true, 64));

  _lcd.begin(16, 2);
  _lcd.setBacklight(WHITE);

#ifdef CLEAR_EEPROM
  lcdPrintString(_lcd, "CLEAR EEPROM", "", "", NOT_CHANGE_COLOR, 0, 0, 0, true, false);

  uint8_t indexLine = 0;
  uint16_t compareParam = EEPROM.length() / 16;

  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0);   
    if (i = compareParam) {     
      lcdPrintString(_lcd, "", "0", "", NOT_CHANGE_COLOR, 0, indexLine, 0, false, false);
      compareParam = compareParam + (EEPROM.length() / 16);
      indexLine = indexLine + 1;
    }
  }

  lcdPrintString(_lcd, "CLEAR EEPROM OK", "", "", NOT_CHANGE_COLOR, 0, 0, 1000, true, false);  
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

    lcdPrintString(_lcd, "INIT EEPROM OK", String(_data.initData), "", NOT_CHANGE_COLOR, 0, 0, 1000, true, true);
  }
#endif

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


  stateGeneralStop = trigerRS(stateGeneralStop, !digitalRead(buttonGeneralStop), !digitalRead(buttonStartFeed));                   // Общий стоп
  stateStartFeed = trigerRS(stateStartFeed, !digitalRead(buttonStartFeed), !digitalRead(buttonGeneralStop));                       // Подача-пуск
  digitalWrite(motorStartFeed, !stateStartFeed);                                                                                   // Запускаем мотор возвратно поступательного движения
  stateAutoCycleManual = trigerRS(stateAutoCycleManual, digitalRead(switchAutoCycleManual), !digitalRead(switchAutoCycleManual));  // Переключатель режимов: "Ввод хоны", "Ручной"
  stateTopSlider = trigerRS(stateTopSlider, digitalRead(switchTopSlider), !digitalRead(switchTopSlider));                          // Концевик парковки ползуна в верху исходного состояния
  stateEndCycle = trigerRS(stateEndCycle, !digitalRead(buttonEndCycle), stateGeneralStop);                                         // Конец цикла

  /////////////////////////////////////////////////////ЛОГИКА СОСТОЯНИЯ///////////////////////////////////////////////////////
  if (stateStartFeed) {  // Кнопку подача-пуск нажали. Запускаем мотор возвратно поступательного движения

    if (stateTopSlider) {  // Если ползун на концевике парковки. Концевик парковки, ползун в верху исходного состояния
      _data.absoluteAngle = 0;
      _data.anglePrevious = angleSensor.RotationRawToAngle(angleSensor.getRawRotation(true, 64));
    }

    _data.linearMove = getLinearMotion();  // Получаем данные с энкодера

/**
      Имитация механического путевого переключателя,
      data.linearMove хранит текущее положение ползуна, контролируемое энкодером
      data.limitTop и data.limitBottom лимиты крайних положений ползуна
      data.stateElectromagnetTop и data.stateElectromagnetBottom хранят состояние переключателя
      data структура сохраняется в EEPROM для восстановления состояния программного путевого переключателя
      после экстренного нажатия кнопки ОБЩИЙ СТОП и выключения питания,
      !!! Не реализовано контроль питания, что бы успеть сохранить data в EEPROM.
    */
#ifdef ENABLE_PROGRAM_SWITCH
    if (_data.linearMove <= _data.limitTop) {  // Если переключатель включен в положение верх

      _data.stateElectromagnetTop = true;      // включить стояние вверх программного переключателя
      _data.stateIntermediate = true;          // ползун вышел из промежуточного состояния
      _data.stateElectromagnetBottom = false;  // выключить стояние вниз программного переключателя
    }
    if (_data.limitTop < _data.linearMove && _data.linearMove < _data.limitBottom) {

      _data.stateIntermediate = false;  // ползун находиться в промежуточном состоянии, между лимитами
    }
    if (_data.linearMove >= _data.limitBottom) {  // Если переключатель включен в положение низ

      _data.stateElectromagnetTop = false;    // выключить стояние вверх программного переключателя
      _data.stateIntermediate = true;         // ползун вышел из промежуточного состояния
      _data.stateElectromagnetBottom = true;  // включить стояние вниз программного переключателя
    }
#endif

    if (stateAutoCycleManual) {  // Переключатель включен в режим Цикл

      if (!digitalRead(electromagnetManual)) {    // Если порт electromagnetManual включён
        digitalWrite(electromagnetBrake, true);   // отключаем электромагнит растормаживания
        digitalWrite(electromagnetManual, true);  // отключаем электромагнит ручной подачи
      }

      if (stateTopSlider) {  // Если ползун на концевике парковки

        statePush = trigerRS(statePush,
                             !digitalRead(buttonPush),
                             digitalRead(buttonPush) || stateGeneralStop);  // Толчковый ввод хоны

        if (stateEndCycle) {
          digitalWrite(electromagnetTop, true);    // выключить электромагнит движения вверх
          digitalWrite(electromagnetBrake, true);  // выключить электромагнит растормаживания
          stateEndCycle = false;                   // Конец цикла, сбрасываем так как ползун стоит на концевике парковки
        }


#ifdef ENABLE_PROGRAM_SWITCH
        if (statePush && !_data.stateElectromagnetBottom) {  // Если кнопка Толковая нажата и программный переключатель включён вверх

          digitalWrite(electromagnetBrake, false);                           // включаем электромагнит растормаживания
          digitalWrite(electromagnetBottom, _data.stateElectromagnetBottom);  // включаем электромагнит движения вниз
        }
        if (!statePush && !_data.stateElectromagnetBottom) {

          digitalWrite(electromagnetBrake, true);   // выключить электромагнит растормаживания
          digitalWrite(electromagnetBottom, true);  // выключить электромагнит движения вниз
        }
#endif

#ifdef ENABLE_SWITCH
        if (statePush && !digitalRead(endSwitchTop)) {  // Если кнопка Толковая нажата и переключатель включён вверх

          digitalWrite(electromagnetBrake, false);   // включаем электромагнит растормаживания
          digitalWrite(electromagnetBottom, false);  // включаем электромагнит движения вниз
        }
        if (!statePush && !digitalRead(endSwitchTop)) {

          digitalWrite(electromagnetBrake, true);   // выключить электромагнит растормаживания
          digitalWrite(electromagnetBottom, true);  // выключить электромагнит движения вниз
        }
#endif
      }

      if (!stateTopSlider) {  // Если ползун сошёл с концевика парковки

        if (stateEndCycle) {  // Если кнопку Конец Цикла нажали

          stateSpindle = trigerRS(stateSpindle,
                                  !digitalRead(buttonSpindleStart),
                                  !digitalRead(buttonSpindleStop) || stateEndCycle || stateGeneralStop);  // Шпиндель Стоп или Старт

          digitalWrite(motorSpindle, !stateSpindle);      // выключаем мотор шпинделя
          digitalWrite(motorSelfCoolant, !stateSpindle);  // выключаем мотор помпы СОЖ
          digitalWrite(electromagnetBottom, true);        // выключить электромагнит движения вниз

          digitalWrite(electromagnetBrake, false);                             // включаем электромагнит растормаживание
          digitalWrite(electromagnetTop, _data.stateElectromagnetTop = false);  // включить электромагнит движения вверх
        }

        if (!stateEndCycle) {  // Если кнопку Конец Цикла не нажали

          if (stateSpindle) {  // Если в ручном режиме ввели в цилиндр и запустили шпиндель и перевели переключатель в Цикл или просто Включён              
            lcdPrintString(_lcd, "", "", "", WHITE, 0, 0, 0, true, false);
            digitalWrite(electromagnetBrake, false);  // включаем электромагнит растормаживания
            stateStartCycle = true;                   // вход в цикл
          }

#ifdef ENABLE_PROGRAM_SWITCH
          stateSpindle = trigerRS(stateSpindle,
                                  !digitalRead(buttonSpindleStart),
                                  !digitalRead(buttonSpindleStop) || statePush || stateEndCycle || stateGeneralStop || _data.stateElectromagnetTop);  // Шпиндель Стоп или Старт
#endif

#ifdef ENABLE_SWITCH
          stateSpindle = trigerRS(stateSpindle,
                                  !digitalRead(buttonSpindleStart),
                                  !digitalRead(buttonSpindleStop) || statePush || stateEndCycle || stateGeneralStop || digitalRead(endSwitchBottom));  // Шпиндель Стоп или Старт
#endif

          if (!stateSpindle) {  // Шпиндель выключен

            statePush = trigerRS(statePush,
                                 !digitalRead(buttonPush),
                                 digitalRead(buttonPush) || stateSpindle || stateEndCycle || stateGeneralStop);  // Толчковый ввод хоны

#ifdef ENABLE_PROGRAM_SWITCH
            if (statePush && !_data.stateElectromagnetTop || statePush && !_data.stateElectromagnetBottom) {  // Если кнопка Толковая нажата и переключатель путевой включён вверх или вниз

              digitalWrite(electromagnetBrake, false);                           // включаем электромагнит растормаживания
              digitalWrite(electromagnetTop, _data.stateElectromagnetTop);        // выключаем электромагнит движения вверх
              digitalWrite(electromagnetBottom, _data.stateElectromagnetBottom);  // включаем электромагнит движения вниз
            }
            if ((!statePush && !_data.stateElectromagnetTop) || (!statePush && !_data.stateElectromagnetBottom)) {  // Если кнопка Толковая не нажата и переключатель путевой включён вниз или вверх

              digitalWrite(electromagnetBrake, true);   // включаем электромагнит растормаживания
              digitalWrite(electromagnetTop, true);     // выключаем электромагнит движения вверх
              digitalWrite(electromagnetBottom, true);  // выключаем электромагнит движения вниз
            }
#endif

#ifdef ENABLE_SWITCH
            if (statePush && !digitalRead(endSwitchTop)) {  // Если кнопка Толковая нажата и переключатель путевой включён вверх

              digitalWrite(electromagnetBrake, false);   // включаем электромагнит растормаживания
              digitalWrite(electromagnetTop, true);      // выключаем электромагнит движения вверх
              digitalWrite(electromagnetBottom, false);  // включаем электромагнит движения вниз
            }
            if (statePush && !digitalRead(endSwitchBottom)) {  // Если кнопка Толковая нажата и переключатель путевой включён вверх

              digitalWrite(electromagnetBrake, false);  // включаем электромагнит растормаживания
              digitalWrite(electromagnetTop, false);    // включаем электромагнит движения вверх
              digitalWrite(electromagnetBottom, true);  // выключаем электромагнит движения вниз
            }
            if ((!statePush && !digitalRead(endSwitchTop)) || (!statePush && !digitalRead(endSwitchBottom))) {  // Если кнопка Толковая не нажата и переключатель путевой включён вниз или вверх

              digitalWrite(electromagnetBrake, true);   // выключаем электромагнит растормаживания
              digitalWrite(electromagnetTop, true);     // выключаем электромагнит движения вверх
              digitalWrite(electromagnetBottom, true);  // выключаем электромагнит движения вниз
            }
#endif
          }
        }
      }
    }

    if (!stateAutoCycleManual) {  // Переключатель включен в режим Ручной

      digitalWrite(electromagnetBrake, false);   // включаем электромагнит растормаживания
      digitalWrite(electromagnetManual, false);  // включаем электромагнит ручной подачи

      if (!stateTopSlider) {  // Если ползун сошёл с концевика парковки

        stateSpindle = trigerRS(stateSpindle,
                                !digitalRead(buttonSpindleStart),
                                !digitalRead(buttonSpindleStop) || stateEndCycle || stateGeneralStop);  // Шпиндель Стоп или Стоп

#ifdef ENABLE_PROGRAM_SWITCH
        if (stateSpindle && ((_data.limitTop - 5 /*мм*/) < _data.linearMove) && (_data.linearMove < (_data.limitBottom + 5 /*мм*/))) {
          digitalWrite(motorSpindle, !stateSpindle);      // включение выключение мотора шпинделя
          digitalWrite(motorSelfCoolant, !stateSpindle);  // включение выключение мотора помпы СОЖ
        } else {
          stateSpindle = false;
        }
#endif

#ifdef ENABLE_SWITCH
        if (stateSpindle) {
          digitalWrite(motorSpindle, !stateSpindle);      // включение выключение мотора шпинделя
          digitalWrite(motorSelfCoolant, !stateSpindle);  // включение выключение мотора помпы СОЖ
        }
#endif

        if (!stateSpindle) {
          digitalWrite(motorSpindle, !stateSpindle);      // включение выключение мотора шпинделя
          digitalWrite(motorSelfCoolant, !stateSpindle);  // включение выключение мотора помпы СОЖ
          /**
            LCD DISPLAY AND BUTTONS READ
          */
          Menu();
        }
      }
    }

#ifdef ENABLE_PROGRAM_SWITCH
    if (!_data.stateIntermediate && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
      lcdPrintString(_lcd, "IN FIELD ACTION", String(_data.linearMove, 2), "mm", YELLOW, 0, 0, 0, true, false);      
    }

    if (_data.stateIntermediate && !_data.stateElectromagnetBottom && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
      lcdPrintString(_lcd, "LIMIT TOP PROG", String(_data.linearMove, 2), "mm", WHITE, 0, 0, 0, true, false);      
    }

    if (_data.stateIntermediate && !_data.stateElectromagnetTop && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
      lcdPrintString(_lcd, "LIMIT BOOTOM PROG", String(_data.linearMove, 2), "mm", WHITE, 0, 0, 0, true, false);      
    }
#endif

#ifdef ENABLE_SWITCH
    if (!digitalRead(endSwitchTop) && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
      lcdPrintString(_lcd, "LIMIT TOP MECHAN", String(_data.linearMove, 2), "mm", YELLOW, 0, 0, 0, true, false);      
    }

    if (!digitalRead(endSwitchBottom) && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
      lcdPrintString(_lcd, "LIMIT BOOTOM MECHAN", String(_data.linearMove, 2), "mm", GREEN, 0, 0, 0, true, false);      
    }
#endif
  }

  if (!stateStartFeed) {  // Кнопку Общий стоп нажали

    stateSpindle = trigerRS(stateSpindle,
                            !digitalRead(buttonSpindleStart),
                            !digitalRead(buttonSpindleStop) || stateEndCycle || stateGeneralStop);  // Шпиндель Стоп или Старт

    digitalWrite(motorSpindle, !stateSpindle);      // выключение мотора шпинделя
    digitalWrite(motorSelfCoolant, !stateSpindle);  // выключаем мотор помпы СОЖ
    digitalWrite(electromagnetTop, true);           // выключить электромагнит движения вверх
    digitalWrite(electromagnetBottom, true);        // выключить электромагнит движения вниз
    digitalWrite(electromagnetManual, true);        // выключаем электромагнит ручной подачи
    digitalWrite(electromagnetBrake, true);         // выключаем электромагнит растормаживания

    /////////////////////////////////////////////////////EEPROM SAVE///////////////////////////////////////////////////////
    saveEeprom(_lcd, _dataBuffer, _data);

    /////////////////////////////////////////////////////LCD DISPLAY BUTTONS READ///////////////////////////////////////////////////////
    Menu();
  }

  /////////////////////////////////////////////////////ЦИКЛ///////////////////////////////////////////////////////
  while (stateStartCycle) {  // Включён режим Цикл

#ifdef ENABLE_PROGRAM_SWITCH
    if (_data.linearMove <= _data.limitTop) {  // Если переключатель включен в положение верх

      _data.stateElectromagnetTop = true;      // включить стояние вверх программного переключателя
      _data.stateIntermediate = true;          // ползун вышел из промежуточного состояния
      _data.stateElectromagnetBottom = false;  // выключить стояние вниз программного переключателя
    }
    if (_data.limitTop < _data.linearMove && _data.linearMove < _data.limitBottom) {

      _data.stateIntermediate = false;  // ползун находиться в промежуточном состоянии, между лимитами
    }
    if (_data.linearMove >= _data.limitBottom) {  // Если переключатель включен в положение низ

      _data.stateElectromagnetTop = false;    // выключить стояние вверх программного переключателя
      _data.stateIntermediate = true;         // ползун вышел из промежуточного состояния
      _data.stateElectromagnetBottom = true;  // включить стояние вниз программного переключателя
    }
#endif

    _data.linearMove = getLinearMotion();  // Получаем данные с энкодера

    stateGeneralStop = trigerRS(stateGeneralStop, !digitalRead(buttonGeneralStop), !digitalRead(buttonStartFeed));                   // Общий стоп
    stateStartFeed = trigerRS(stateStartFeed, !digitalRead(buttonStartFeed), !digitalRead(buttonGeneralStop));                       // Подача-пуск
    digitalWrite(motorStartFeed, !stateStartFeed);                                                                                   // Запускаем мотор возвратно поступательного движения
    stateAutoCycleManual = trigerRS(stateAutoCycleManual, digitalRead(switchAutoCycleManual), !digitalRead(switchAutoCycleManual));  // Переключатель режимов: "Ввод хоны", "Ручной"
    stateTopSlider = trigerRS(stateTopSlider, digitalRead(switchTopSlider), !digitalRead(switchTopSlider));                          // Концевик парковки ползуна в верху исходного состояния
    stateEndCycle = trigerRS(stateEndCycle, !digitalRead(buttonEndCycle), stateGeneralStop);                                         // Конец цикла

    if (stateAutoCycleManual) {  // Переключатель включен в режим Цикл

      digitalWrite(electromagnetManual, true);  // выключить электромагнит ручного управления если был на ручном

      stateSpindle = trigerRS(stateSpindle,
                              !digitalRead(buttonSpindleStart),
                              !digitalRead(buttonSpindleStop) || stateEndCycle || stateGeneralStop);  // Шпиндель Стоп или Старт

      if (stateEndCycle) {  // Если кнопку Конец Цикла нажали

        digitalWrite(motorSpindle, !stateSpindle);      // выключаем мотор шпинделя
        digitalWrite(motorSelfCoolant, !stateSpindle);  // выключаем мотор помпы СОЖ
        digitalWrite(electromagnetBottom, true);        // выключить электромагнит движения вниз
        digitalWrite(electromagnetTop, false);          // включить электромагнит движения вверх

        if (stateTopSlider) {  // Ползун поднялся в верх, исходное состояние конец цикла

          digitalWrite(electromagnetTop, true);    // выключить электромагнит движения вверх
          digitalWrite(electromagnetBrake, true);  // выключаем электромагнит растормаживания
          stateEndCycle = false;                   // сбрасываем состояние выхода из цикла
          stateStartCycle = false;                 // выходим из цикла
          //break;
        }
      }

      if (!stateEndCycle && stateSpindle) {  //Если кнопку Конец Цикла не нажали, Проверяем состояние шпинделя Eсли включен

        digitalWrite(motorSpindle, !stateSpindle);      // включить мотор шпинделя
        digitalWrite(motorSelfCoolant, !stateSpindle);  // включаем мотор помпы СОЖ

#if defined(ENABLE_PROGRAM_SWITCH) && !defined(ENABLE_SWITCH)
        if (!data.stateElectromagnetBottom) {  // Если переключатель включен в положение верх

          digitalWrite(electromagnetTop, data.stateElectromagnetTop);  // выключить электромагнит движения вверх
          delay(100);
          digitalWrite(electromagnetBottom, data.stateElectromagnetBottom);  // включить электромагнит движения вниз
        }

        if (!data.stateElectromagnetTop) {  // Если переключатель включен в положение низ

          digitalWrite(electromagnetBottom, data.stateElectromagnetBottom);  // выключить электромагнит движения вниз
          delay(100);
          digitalWrite(electromagnetTop, data.stateElectromagnetTop);  // включить электромагнит движения вверх
        }
#endif

#if defined(ENABLE_PROGRAM_SWITCH) && defined(ENABLE_SWITCH)
        if (!_data.stateElectromagnetBottom || !digitalRead(endSwitchTop)) {  // Если переключатель включен в положение верх

          digitalWrite(electromagnetTop, true);  // выключить электромагнит движения вверх
          delay(100);
          digitalWrite(electromagnetBottom, false);  // включить электромагнит движения вниз
        }
        if (!_data.stateElectromagnetTop || !digitalRead(endSwitchBottom)) {  // Если переключатель включен в положение низ

          digitalWrite(electromagnetBottom, true);  // выключить электромагнит движения вниз
          delay(100);
          digitalWrite(electromagnetTop, false);  // включить электромагнит движения вверх
        }
#endif

#if defined(ENABLE_SWITCH) && !defined(ENABLE_PROGRAM_SWITCH)
        if (!digitalRead(endSwitchTop)) {  // Если переключатель включен в положение верх

          digitalWrite(electromagnetTop, true);  // выключить электромагнит движения вверх
          delay(100);
          digitalWrite(electromagnetBottom, false);  // включить электромагнит движения вниз
        }
        if (!digitalRead(endSwitchBottom)) {  // Если переключатель включен в положение низ

          digitalWrite(electromagnetBottom, true);  // выключить электромагнит движения вниз
          delay(100);
          digitalWrite(electromagnetTop, false);  // включить электромагнит движения вверх
        }
#endif
      }

      if (!stateEndCycle && !stateSpindle) {  //Если кнопку Конец Цикла не нажали, Проверяем состояние шпинделя Если выключен

        digitalWrite(motorSpindle, !stateSpindle);      // отключаем мотор шпинделя
        digitalWrite(motorSelfCoolant, !stateSpindle);  // отключаем мотор помпы СОЖ

#ifdef ENABLE_PROGRAM_SWITCH
        if (!_data.stateElectromagnetBottom) {  // Если переключатель включен в положение верх

          digitalWrite(electromagnetTop, _data.stateElectromagnetTop);  // выключить электромагнит движения вверх
          delay(100);
          digitalWrite(electromagnetBottom, _data.stateElectromagnetBottom);  // включить электромагнит движения вниз
        }

        if (!_data.stateElectromagnetTop) {  // Если переключатель включен в положение низ

          digitalWrite(electromagnetBottom, _data.stateElectromagnetBottom);  // выключить электромагнит движения вниз
          delay(100);
          digitalWrite(electromagnetTop, _data.stateElectromagnetTop);  // включить электромагнит движения вверх
        }
#endif

#ifdef ENABLE_SWITCH
        if (!digitalRead(endSwitchTop)) {  // Если переключатель включен в положение верх

          digitalWrite(electromagnetTop, true);  // выключить электромагнит движения вверх
          delay(100);
          digitalWrite(electromagnetBottom, false);  // включить электромагнит движения вниз
        }

        if (!digitalRead(endSwitchBottom)) {  // Если переключатель включен в положение низ

          digitalWrite(electromagnetBottom, true);  // выключить электромагнит движения вниз
          delay(100);
          digitalWrite(electromagnetTop, false);  // включить электромагнит движения вверх
        }
#endif
      }
    }

    if (!stateAutoCycleManual) {  // Переключатель включен в режим Ручной

      stateSpindle = trigerRS(stateSpindle,
                              !digitalRead(buttonSpindleStart),
                              !digitalRead(buttonSpindleStop) || stateEndCycle || stateGeneralStop);  // Шпиндель Стоп или Старт

      digitalWrite(electromagnetTop, true);     // выключить электромагнит движения вверх
      digitalWrite(electromagnetBottom, true);  // выключить электромагнит движения вниз

      digitalWrite(electromagnetManual, false);  // включить электромагнит ручного управления

      digitalWrite(motorSpindle, !stateSpindle);      // включить мотор шпинделя или отключить в зависимости от stateSpindle
      digitalWrite(motorSelfCoolant, !stateSpindle);  // включить мотор помпы СОЖ или отключить в зависимости от stateSpindle

      if (!stateSpindle) {  // Если шпиндель выключен, есть возможность запустить меню настройки
        /////////////////////////////////////////////////////LCD DISPLAY BUTTONS READ///////////////////////////////////////////////////////
        Menu();  // Запуск меню настроек по удержанию кнопки в течение 3 секунд
      }
    }

#ifdef ENABLE_PROGRAM_SWITCH
    if (!_data.stateIntermediate && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
      lcdPrintString(_lcd, "IN FIELD ACTION", String(_data.linearMove, 2), "mm", GREEN, 0, 0, 0, true, false);
    }

    if (_data.stateIntermediate && !_data.stateElectromagnetBottom && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
      lcdPrintString(_lcd, "LIMIT TOP PROG", String(_data.linearMove, 2), "mm", YELLOW, 0, 0, 0, true, false);
    }

    if (_data.stateIntermediate && !_data.stateElectromagnetTop && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
      lcdPrintString(_lcd, "LIMIT BOOTOM PROG", String(_data.linearMove, 2), "mm", YELLOW, 0, 0, 0, true, false);
    }
#endif

#ifdef ENABLE_SWITCH
    if (!digitalRead(endSwitchTop) && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
      lcdPrintString(_lcd, "LIMIT TOP MECHAN", String(_data.linearMove, 2), "mm", YELLOW, 0, 0, 0, true, false);
    }

    if (!digitalRead(endSwitchBottom) && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
      lcdPrintString(_lcd, "LIMIT BOOTOM MECHAN", String(_data.linearMove, 2), "mm", GREEN, 0, 0, 0, true, false);
    }
#endif

    if (stateGeneralStop) {                           // Нажата кнопка Общий стоп
      digitalWrite(motorSpindle, !stateSpindle);      // отключаем мотор шпинделя
      digitalWrite(motorSelfCoolant, !stateSpindle);  // отключаем мотор помпы СОЖ
      digitalWrite(motorStartFeed, !stateStartFeed);  // отключаем мотор возвратно поступательного движения

      digitalWrite(electromagnetTop, true);     // выключить электромагнит движения вверх
      digitalWrite(electromagnetBottom, true);  // выключить электромагнит движения вниз
      digitalWrite(electromagnetBrake, true);   // выключить электромагнит растормаживания
      digitalWrite(electromagnetManual, true);  // выключить электромагнит ручного управления

      stateStartCycle = false;  // выходим из цикла

      /////////////////////////////////////////////////////EEPROM SAVE///////////////////////////////////////////////////////
      saveEeprom(_lcd, _dataBuffer, _data);
    }
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
    lcdPrintString(_lcd, String(second += 1), "", "", WHITE, 0, 0, 1000, true, true);
  }

  if (startMenu) {
    second = 0;
    lcdPrintString(_lcd, "Start Menu", "", "", NOT_CHANGE_COLOR, 0, 0, 1000, true, false);

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
          lcdPrintString(_lcd, String(second += 1), "", "", NOT_CHANGE_COLOR, 0, 0, 1000, true, false);
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
      lcdPrintString(_lcd, "Close Menu", "", "", WHITE, 0, 0, 1000, true, true);
    }
  }
}

void lcdPrintString(Adafruit_RGBLCDShield lcd, String msg, String msgData, String msgAfterData, uint8_t color, uint8_t posLineOne, uint8_t posLineTwo, unsigned long msgDelay, bool clearBeforeRendering, bool clearAfterRendering ) {
  if (clearBeforeRendering) lcd.clear();
  if (color != -1) lcd.setBacklight(color);
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
  if (clearAfterRendering) lcd.clear();  
}


template< typename LCD, typename B, typename D >
void saveEeprom(LCD lcd, B &dataBuffer, D &data) {
  EEPROM.get(0, dataBuffer);
  if (data != dataBuffer) {
    EEPROM.put(0, data);  // Сохранение изменений структуры data в EEPROM
    lcdPrintString(lcd, "SAVE EEPROM OK", String(data.initData), "", WHITE, 0, 0, 1000, true, true);
  }
}
