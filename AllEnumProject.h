#pragma once

#include <Arduino.h>
#include "config.h"

enum FunctionTypes : uint8_t {
  Increase = 1,
  Decrease = 2,
  Edit = 3
};

enum DecIncrTypes : uint8_t {
  Inc = 1,
  Dec = 2
};

enum StartLevelSpeed : uint8_t {
    Speed_1 = 0,  // Первая скорость
    Speed_2,      // Вторая скорость
    Speed_3       // Третья скорость
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

extern Data _data;
extern Data _dataBuffer;  // временная переменная для проверки данных в EEPROM с data, чтобы не писать в EEPROM часто
#endif
