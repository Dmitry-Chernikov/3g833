#include <Arduino.h>

#include "TechnicalSpecifications3G833.h"
using namespace TechnicalSpecifications3G833;
#include "config.h" //Определены define для вкл/выкл кода в компиляцию

#include "ControlSystem.h"    // Основной алгоритм работы станка содержит процедуры используемые в loop
#include "Display.h"          // Работа с дисплеем Adafruit RGB LCD Shield
#include "Encoder.h"          // Объявление объекта типа AS5048A для работы с энкодером AS5048A
#include "IOPorts.h"          // Описаны все порты ввода/вывода процедуры их настройки
#include "MemoryEeprom.h"     // Описывает структуру данных которая сохраняется в память и процедуры для работы с памятью

#include "StatesActuators.h"  // Описаны переменные которые хранят состояния режимов работы станка и исполнительных механизмов

#include "TextMenu.h"         // Создаётся текстовое меню на базе LiquidMenu которая использует дисплей Adafruit RGB LCD Shield

#include "VariablesProject.h" // Описаны все причисления используемые в проекте

#include "SUSWE320/SUSWE321.h"
//#include <ModbusMaster.h>

// #include <avr/pgmspace.h>
// #include <util/delay.h>

// char input_saved[3];
// char output_saved[3];
// char string_saved[] = " *";
// char string_notSaved[] = "  ";

#ifdef ENABLE_KEYPAD
void pciSetup(byte pin) {
    //*Pin Change Interrupt Прерывание по изменению вывода*//
    // PCICR Регистр управления PCINT прерываниями, имеются три группы PCI0..2 в
    // которые входят выводы по 8 штук PCIFR Регистр флагов сработавших прерываний
    // PCINT, показывает в какой группе сработало прерывание от вывода PCMSKx
    // Регистр маскировки прерываний каждого из портов для групп PCI0..2,
    // активирует прерывание по выводу 1 Задать обработчик для соответствующего
    // прерывания PCINT, используя макрос ISR. 2 Разрешить генерацию прерываний
    // интересующим выводом микроконтроллера (регистр группы PCMSKx). 3 Разрешить
    // обработку прерывания PCINT, которое генерирует интересующий вывод (регистр
    // PCICR). 4 Установить бит I, разрешающий обработку прерываний глобально
    // (регистр SREG).
    // PCMSK0 |= 1 << 6;
    *digitalPinToPCMSK(pin) |= bit(digitalPinToPCMSKbit(pin)); // Разрешаем PCINT для указанного пина
    // PCIFR |= 0 << 0
    PCIFR |= bit(digitalPinToPCICRbit(pin)); // Очищаем признак запроса прерывания
    // для соответствующей группы пинов
    // PCICR |= 1 << 0;
    PCICR |= bit(digitalPinToPCICRbit(pin)); // Разрешаем PCINT для соответствующей группы пинов
    SREG |= 1 << SREG_I; // бит 7 Разрешить прерывания микроконтроллера
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

// const int ADDRESS_DATA_PARAM = 0x000D;
// const int ADDRESS_DATA_STATE = 0x0070;
// const int ADDRESS_FAULTY_DESCRIPTION = 0x0080;
/*****SERIAL*****/

/*****SUSWE321*****/
SUSWE321 suswe321(0x0001, &Serial1, &Serial, 9600, rs485TransceiverReceive);
uint16_t responseData;
/*****SUSWE321*****/


void setup() {
    cli();

#ifdef ENABLE_KEYPAD
    pinMode(interruptRemote, INPUT_PULLUP); // Подтянем пины источники PCINT к питанию
    pciSetup(interruptRemote); // И разрешим на них прерывания T6
#endif

    initSetupInputManipulation();

    initSetupOutputExecutiveMechanism();

    suswe321.begin();
    pinMode(rs485TransceiverReceive, OUTPUT);
    digitalWrite(rs485TransceiverReceive, RS485Receive); // переводим модуль в режим приёма данных

#ifdef ENABLE_KEYPAD
    readKeypad();
#endif

    /////////////Инициализация энкодера/////////////
    initEncoder();

    initDisplay();

    clearMemory();

    initMemory();

    settingTextMenu();

    // strncpy(input_saved, string_saved, sizeof(string_saved));
    // strncpy(output_saved, string_saved, sizeof(string_saved));

    sei();
}

void testConnection() {
    Serial.println("\n=== CONNECTION TEST ===");

    // Тест 1: Простой пинг
    Serial.println("1. Sending test command...");
    uint16_t value;

    // Добавляем больше задержек
    if (suswe321.readSingleParameter(GROUP_d, &value)) {
        Serial.println("*** SUCCESS: Device responded! ***");
        Serial.print("Value: 0x");
        Serial.println(value, HEX);
    } else {
        Serial.println("*** FAILED: No response from device ***");

        // Проверка напряжения на линиях
        Serial.println("Check:");
        Serial.println("- RS485 A/B lines connection");
        Serial.println("- Common GND");
        Serial.println("- Device power");
        Serial.println("- MAX485 power (5V)");
        Serial.println("- DE/RE pin connection");
    }

    Serial.println("=== TEST COMPLETE ===\n");
}

void loop() {

    /*****SUSWE321*****/
    if (suswe321.checkCommunicationSettings()) {
        //Serial.println(String(responseData));

        lcdPrintString(_lcd, "Frequency", "OK", "", YELLOW, NOT_CHANGE_COLOR, 0, 0, 0, true, false);
    }

    //Serial.println(suswe321.readFaultDescription(0x0001, &responseData)); // Чтение описания неисправностей
    //delay(1000);
    /*****SUSWE321*****/


    handleButtonStates();
    handleMotorStates();

    /////////////////////////////////////////////////////ЛОГИКА СОСТОЯНИЯ///////////////////////////////////////////////////////
    if (stateStartFeed) {
        // Кнопку подача-пуск нажали. Запускаем мотор возвратно-поступательного движения

        handleStartFeed();
        handleProgramSwitch();

        handleAutoCycle();

        handleManualMode();

#ifdef ENABLE_PROGRAM_SWITCH
        if (!_data.stateIntermediate && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
            lcdPrintString(_lcd, "IN FIELD ACTION", String(_data.linearMove, 2), "mm", YELLOW, NOT_CHANGE_COLOR, 0, 0,
                           0, true, false);
        }

        if (_data.stateIntermediate && !_data.stateElectromagnetBottom && stateMillisDelay(
                &previousMillisMenu, &updateMenu)) {
            lcdPrintString(_lcd, "LIMIT TOP PROG", String(_data.linearMove, 2), "mm", WHITE, NOT_CHANGE_COLOR, 0, 0, 0,
                           true, false);
        }

        if (_data.stateIntermediate && !_data.stateElectromagnetTop && stateMillisDelay(
                &previousMillisMenu, &updateMenu)) {
            lcdPrintString(_lcd, "LIMIT BOTTOM PROG", String(_data.linearMove, 2), "mm", WHITE, NOT_CHANGE_COLOR, 0, 0,
                           0, true, false);
        }
#endif

#ifdef ENABLE_SWITCH
        if (!digitalRead(endSwitchTop) && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
            lcdPrintString(_lcd, "LIMIT TOP MECHAN", String(_data.linearMove, 2), "mm", YELLOW, NOT_CHANGE_COLOR, 0, 0,
                           0, true, false);
        }

        if (!digitalRead(endSwitchBottom) && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
            lcdPrintString(_lcd, "LIMIT BOTTOM MECHAN", String(_data.linearMove, 2), "mm", GREEN, NOT_CHANGE_COLOR, 0,
                           0, 0, true, false);
        }
#endif
    }

    if (!stateStartFeed) {
        // Кнопку Общий стоп нажали

        handleStop();

        /////////////////////////////////////////////////////EEPROM SAVE///////////////////////////////////////////////////////
        saveEeprom(_lcd, _dataBuffer, _data);

        /////////////////////////////////////////////////////LCD DISPLAY BUTTONS READ///////////////////////////////////////////////////////
        Menu();
    }

    /////////////////////////////////////////////////////ЦИКЛ///////////////////////////////////////////////////////
    while (stateStartCycle) {
        // Включён режим Цикл

        handleCycle();

#ifdef ENABLE_PROGRAM_SWITCH
        if (!_data.stateIntermediate && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
            lcdPrintString(_lcd, "IN FIELD ACTION", String(_data.linearMove, 2), "mm", GREEN, NOT_CHANGE_COLOR, 0, 0, 0,
                           true, false);
        }

        if (_data.stateIntermediate && !_data.stateElectromagnetBottom && stateMillisDelay(
                &previousMillisMenu, &updateMenu)) {
            lcdPrintString(_lcd, "LIMIT TOP PROG", String(_data.linearMove, 2), "mm", YELLOW, NOT_CHANGE_COLOR, 0, 0, 0,
                           true, false);
        }

        if (_data.stateIntermediate && !_data.stateElectromagnetTop && stateMillisDelay(
                &previousMillisMenu, &updateMenu)) {
            lcdPrintString(_lcd, "LIMIT BOTTOM PROG", String(_data.linearMove, 2), "mm", YELLOW, NOT_CHANGE_COLOR, 0, 0,
                           0, true, false);
        }
#endif

#ifdef ENABLE_SWITCH
        if (!digitalRead(endSwitchTop) && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
            lcdPrintString(_lcd, "LIMIT TOP MECHAN", String(_data.linearMove, 2), "mm", YELLOW, NOT_CHANGE_COLOR, 0, 0,
                           0, true, false);
        }

        if (!digitalRead(endSwitchBottom) && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
            lcdPrintString(_lcd, "LIMIT BOTTOM MECHAN", String(_data.linearMove, 2), "mm", GREEN, NOT_CHANGE_COLOR, 0,
                           0, 0, true, false);
        }
#endif
    }
}

#if defined(ENABLE_KEYPAD)
ISR(PCINT0_vect) {
    // Обработчик запросов прерывания от пинов PCINT0..PCINT7

    cli(); // сбрасываем флаг прерывания (Запретить прерывания)
    readKeypad(); // вызов процедуры опроса клавиатуры
    sei(); // устанавливаем флаг прерывания (Разрешить прерывания)
}
#endif
