// ControlSystem.cpp


#include "ControlSystem.h"

bool trigerRS(bool currentState, uint8_t TrigSet, uint8_t TrigReset) {  //Триггер с приоритетом сброса
  if (TrigReset) { return false; }
  if (TrigSet) { return true; }
  return currentState;
}

// Определение функций
void handleButtonStates() {
    // stateGeneralStop = trigerRS(stateGeneralStop, !digitalRead(buttonGeneralStop), !digitalRead(buttonStartFeed)); // Общий стоп
    // stateStartFeed = trigerRS(stateStartFeed, !digitalRead(buttonStartFeed), !digitalRead(buttonGeneralStop)); // Подача-пуск
    // stateAutoCycleManual = trigerRS(stateAutoCycleManual, digitalRead(switchAutoCycleManual), !digitalRead(switchAutoCycleManual)); // Переключатель режимов
    // stateTopSlider = trigerRS(stateTopSlider, digitalRead(switchTopSlider), !digitalRead(switchTopSlider)); // Концевик парковки
    // stateEndCycle = trigerRS(stateEndCycle, !digitalRead(buttonEndCycle), stateGeneralStop); // Конец цикла

    stateGeneralStop = trigerRS(stateGeneralStop, !digitalRead(buttonGeneralStop), !digitalRead(buttonStartFeed));                   // Общий стоп
    stateStartFeed = trigerRS(stateStartFeed, !digitalRead(buttonStartFeed), !digitalRead(buttonGeneralStop));                       // Подача-пуск
    digitalWrite(motorStartFeed, !stateStartFeed);                                                                                   // Запускаем мотор возвратно поступательного движения
    stateAutoCycleManual = trigerRS(stateAutoCycleManual, digitalRead(switchAutoCycleManual), !digitalRead(switchAutoCycleManual));  // Переключатель режимов: "Ввод хоны", "Ручной"
    stateTopSlider = trigerRS(stateTopSlider, digitalRead(switchTopSlider), !digitalRead(switchTopSlider));                          // Концевик парковки ползуна в верху исходного состояния
    stateEndCycle = trigerRS(stateEndCycle, !digitalRead(buttonEndCycle), stateGeneralStop);                                         // Конец цикла
}

void handleMotorStates() {
    digitalWrite(motorStartFeed, !stateStartFeed); // Запускаем мотор возвратно поступательного движения
}

void handleStartFeed() {
    if (stateTopSlider) {
        _data.absoluteAngle = 0;
        _data.anglePrevious = angleSensor.RotationRawToAngle(angleSensor.getRawRotation(true, 64));
    }

    _data.linearMove = getLinearMotion(); // Получаем данные с энкодера
    handleMechanicalSwitch();
    handleAutoCycleManual();
}

/**
  Имитация механического путевого переключателя,
  data.linearMove хранит текущее положение ползуна, контролируемое энкодером
  data.limitTop и data.limitBottom лимиты крайних положений ползуна
  data.stateElectromagnetTop и data.stateElectromagnetBottom хранят состояние переключателя
  data структура сохраняется в EEPROM для восстановления состояния программного путевого переключателя
  после экстренного нажатия кнопки ОБЩИЙ СТОП и выключения питания,
  !!! Не реализовано контроль питания, что бы успеть сохранить data в EEPROM.
*/
void handleMechanicalSwitch() {
#ifdef ENABLE_PROGRAM_SWITCH
    if (_data.linearMove <= _data.limitTop) {
        setElectromagnetState(true, false);
    } else if (_data.linearMove >= _data.limitBottom) {
        setElectromagnetState(false, true);
    } else {
        _data.stateIntermediate = false; // Ползун в промежуточном состоянии
    }
#endif
}


void setElectromagnetState(bool top, bool bottom) {
    _data.stateElectromagnetTop = top; // True|False включить|выключить состояние вверх программного переключателя
    _data.stateElectromagnetBottom = bottom; // True|False включить|выключить состояние вниз программного переключателя
    _data.stateIntermediate = true; // ползун вышел из промежуточного состояния
}

// void handleAutoCycleManual() {
//     if (stateAutoCycleManual) {
//         handleAutoCycle();
//     } else {
//         handleManualMode();
//     }
// }

// void handleAutoCycle() {
//     // Логика автоматического цикла
//     // ...
// }

// void handleManualMode() {
//     // Логика ручного режима
//     // ...
// }

// void handleStop() {
//     stateSpindle = trigerRS(stateSpindle, !digitalRead(buttonSpindleStart), !digitalRead(buttonSpindleStop) || stateEndCycle || stateGeneralStop);
//     digitalWrite(motorSpindle, !stateSpindle);
//     digitalWrite(motorSelfCoolant, !stateSpindle);
//     // Дополнительные действия при остановке
// }

// void handleCycle() {
//     // Логика цикла
//     // ...
//     if (stateGeneralStop) {
//         handleGeneralStop();
//     }
// }

// void handleGeneralStop() {
//     // Логика общего стопа
//     // ...
//     saveEeprom(_lcd, _dataBuffer, _data);
// }