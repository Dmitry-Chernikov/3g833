#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "Display.h"

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

void initMemory();
void clearMemory();

template< typename LCD, typename B, typename D >
void saveEeprom(LCD lcd, B &dataBuffer, D &data) {
  EEPROM.get(0, dataBuffer);
  if (data != dataBuffer) {
    EEPROM.put(0, data);  // Сохранение изменений структуры data в EEPROM
    lcdPrintString(lcd, "SAVE EEPROM OK", String(data.initData), "", WHITE, NOT_CHANGE_COLOR, 0, 0, 1000, true, true);
  }
}
