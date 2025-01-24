#include <Arduino.h>

#include "config.h" // Определены define для вкл/выкл кода в компеляцию
#include "TechnicalSpecifications3G833.h"

#include "IOPorts.h" // Описаны все порты ввода/вывода процедуры их настройки
#include "StatesActuators.h" // Описаны пременные которые хранят состояния режимов работы станка и исполнительных механизмов
#include "VariablesProject.h" // Описаны все пречисления используемые в прокте
#include "MemoryEeprom.h" // Описывает структуру данных которая сохранияеться в память и процедуры для работы с памятью
#include "Encoder.h" // Обявление обекта типа AS5048A для работы с энкодером AS5048A
#include "Display.h" // Работа с дисплеем Adafruit RGB LCD Shield
#include "TextMenu.h" // Создёться текстовое меню на базе LiquidMenu которая ипользует дисплей Adafruit RGB LCD Shield
#include "ControlSystem.h" // Основной алгоритм работы станка содержит процедуры используемые в loop

//#include <avr/pgmspace.h>
//#include <util/delay.h>

//char input_saved[3];
//char output_saved[3];
//char string_saved[] = " *";
//char string_notSaved[] = "  ";        

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

  initSetupOutpuExecutiveMechanism();

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

  initDisplay();

  clearMemory();

  initMemory();

  settingTextMenu();

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
