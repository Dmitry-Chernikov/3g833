#pragma once

#include "Display.h"
#include "config.h"
#include <Arduino.h>
#include <EEPROM.h>

#if defined ENABLE_EEPROM || defined CLEAR_EEPROM
struct Data {
  char initData;

  float linearMove;    // Длина линейного перемещения от концевика парковки от
                       // энкодера
  float anglePrevious; // Угол предыдущий от экодера
  float absoluteAngle; // Обсалютный угол или инкрементн и декремент угла

  float limitTop;        // Верхняя позиция цикла
  float limitBottom;     // Нижняя позиция цикла
  float cylinderDiameter; // Диаметр обрабатываемого цилиндра
  float cylinderAngle;   // Желаемый угол сетки в цилиндре

  bool stateElectromagnetTop;    // Сохранение состояния муфты движения вверх, если
                                 // электричество отключили
  bool stateElectromagnetBottom; // Сохранение состояния муфты движения вниз,
                                 // если электричество отключили
  bool stateIntermediate;        // Сохранение состояния интервала в режиме цикла, если
                                 // электричество отключили

  bool operator!=(const Data &other) const {
    return initData != other.initData || abs(linearMove - other.linearMove) > 0.1 || 
                                          abs(anglePrevious - other.anglePrevious) > 0.1 || 
                                          abs(absoluteAngle - other.absoluteAngle) > 0.1 ||
                                          abs(limitTop - other.limitTop) > 0.1 || 
                                          abs(limitBottom - other.limitBottom) > 0.1 || 
                                          abs(cylinderDiameter - other.cylinderDiameter) > 0.1 ||
                                          abs(cylinderAngle - other.cylinderAngle) > 0.1 ||

           stateElectromagnetTop != other.stateElectromagnetTop || stateElectromagnetBottom != other.stateElectromagnetBottom || stateIntermediate != other.stateIntermediate;
  }
};

extern Data _data;
extern Data _dataBuffer; // временная переменная для проверки данных в EEPROM с data, чтобы не писать в EEPROM часто
#endif

void initMemory();
void clearMemory();

template <typename LCD, typename B, typename D> void saveEeprom(LCD lcd, B &dataBuffer, D &data) {
  EEPROM.get(0, dataBuffer);
  if (data != dataBuffer) {
    EEPROM.put(0, data); // Сохранение изменений структуры data в EEPROM
    lcdPrintString(lcd, "SAVE EEPROM OK", String(data.initData), "", WHITE, NOT_CHANGE_COLOR, 0, 0, 1000, true, true);
  }
}
