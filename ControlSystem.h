#pragma once

#include <Arduino.h>

#include "Display.h"      // Работа с дисплеем Adafruit RGB LCD Shield
#include "Encoder.h"      // Обявление обекта типа AS5048A для работы с энкодером AS5048A
#include "IOPorts.h"      //Описаны все порты ввода/вывода процедуры их настройки
#include "MemoryEeprom.h" // Описывает структуру данных которая сохранияеться в память и процедуры для работы с памятью
#include "StatesActuators.h" //Описаны пременные которые хранят состояния режимов работы станка и исполнительных механизмов
#include "TextMenu.h"        // Создёться текстовое меню на базе LiquidMenu которая ипользует дисплей Adafruit RGB LCD Shield
#include "VariablesProject.h" //Описаны все пречисления используемые в прокте
#include "config.h"           //Определены define для вкл/выкл кода в компеляцию

void handleButtonStates();
void handleMotorStates();
void handleStartFeed();
void handleProgramSwitch();
void setElectromagnetState(bool top, bool bottom);
void handleAutoCycleManual();
void handleAutoCycle();
void handleManualMode();
void handleStop();
void handleCycle();
void handleGeneralStop();