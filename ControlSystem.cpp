#include "ControlSystem.h"

// Определение функций
void handleButtonStates() {
  stateGeneralStop = trigerRS(stateGeneralStop, !digitalRead(buttonGeneralStop), !digitalRead(buttonStartFeed));                  // Общий стоп
  stateStartFeed = trigerRS(stateStartFeed, !digitalRead(buttonStartFeed), !digitalRead(buttonGeneralStop));                      // Подача-пуск
  stateAutoCycleManual = trigerRS(stateAutoCycleManual, digitalRead(switchAutoCycleManual), !digitalRead(switchAutoCycleManual)); // Переключатель режимов: "Ввод хоны", "Ручной"
  stateTopSlider = trigerRS(stateTopSlider, digitalRead(switchTopSlider), !digitalRead(switchTopSlider)); // Концевик парковки ползуна в верху исходного состояния
  stateEndCycle = trigerRS(stateEndCycle, !digitalRead(buttonEndCycle), stateGeneralStop);                // Конец цикла
}

void handleMotorStates() {
  digitalWrite(motorStartFeed, !stateStartFeed); // Запускаем мотор возвратно поступательного движения
}

void handleStartFeed() {
  if (stateTopSlider) { // Если ползун на концевике парковки. Концевик парковки, ползун в верху исходного состояния
    _data.absoluteAngle = 0;
    _data.anglePrevious = getAngle();
  }

  _data.linearMove = getLinearMotion(); // Получаем данные с энкодера
  handleProgramSwitch();
  handleAutoCycleManual();
}

/**
  Имитация программного переключателя имитрирующего работу механического
  путевого переключателя, data.linearMove хранит текущее положение ползуна,
  контролируемое энкодером data.limitTop и data.limitBottom лимиты крайних
  положений ползуна data.stateElectromagnetTop и data.stateElectromagnetBottom
  хранят состояние переключателя data структура сохраняется в EEPROM для
  восстановления состояния программного путевого переключателя после экстренного
  нажатия кнопки ОБЩИЙ СТОП и выключения питания,
  !!! Не реализовано контроль питания, что бы успеть сохранить data в EEPROM.
*/
void handleProgramSwitch() {
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
  _data.stateElectromagnetTop = top;       // True|False включить|выключить состояние вверх программного переключателя
  _data.stateElectromagnetBottom = bottom; // True|False включить|выключить состояние вниз программного переключателя
  _data.stateIntermediate = true;          // ползун вышел из промежуточного состояния
}

void handleAutoCycleManual() {
  if (stateAutoCycleManual) { // Переключатель включен в режим Цикл
    handleAutoCycle();
  } else { // Переключатель включен в режим Ручной
    handleManualMode();
  }
}

void handleAutoCycle() {
  if (!digitalRead(electromagnetManual)) {   // Если порт electromagnetManual включён
    digitalWrite(electromagnetBrake, true);  // отключаем электромагнит растормаживания
    digitalWrite(electromagnetManual, true); // отключаем электромагнит ручной подачи
  }

  if (stateTopSlider) { // Если ползун на концевике парковки

    statePush = trigerRS(statePush, !digitalRead(buttonPush),
                         digitalRead(buttonPush) || stateGeneralStop); // Толчковый ввод хоны

    if (stateEndCycle) {
      digitalWrite(electromagnetTop, true);   // выключить электромагнит движения вверх
      digitalWrite(electromagnetBrake, true); // выключить электромагнит растормаживания
      stateEndCycle = false;                  // Конец цикла, сбрасываем так как ползун стоит на концевике парковки
    }

#ifdef ENABLE_PROGRAM_SWITCH
    if (statePush && !_data.stateElectromagnetBottom) { // Если кнопка Толковая нажата и программный переключатель включён вверх

      digitalWrite(electromagnetBrake, false);                           // включаем электромагнит растормаживания
      digitalWrite(electromagnetBottom, _data.stateElectromagnetBottom); // включаем электромагнит движения вниз
    }
    if (!statePush && !_data.stateElectromagnetBottom) {

      digitalWrite(electromagnetBrake, true);  // выключить электромагнит растормаживания
      digitalWrite(electromagnetBottom, true); // выключить электромагнит движения вниз
    }
#endif

#ifdef ENABLE_SWITCH
    if (statePush && !digitalRead(endSwitchTop)) { // Если кнопка Толковая нажата и переключатель включён вверх

      digitalWrite(electromagnetBrake, false);  // включаем электромагнит растормаживания
      digitalWrite(electromagnetBottom, false); // включаем электромагнит движения вниз
    }
    if (!statePush && !digitalRead(endSwitchTop)) {

      digitalWrite(electromagnetBrake, true);  // выключить электромагнит растормаживания
      digitalWrite(electromagnetBottom, true); // выключить электромагнит движения вниз
    }
#endif
  }

  if (!stateTopSlider) { // Если ползун сошёл с концевика парковки

    if (stateEndCycle) { // Если кнопку Конец Цикла нажали

      stateSpindle = trigerRS(stateSpindle, !digitalRead(buttonSpindleStart),
                              !digitalRead(buttonSpindleStop) || stateEndCycle || stateGeneralStop); // Шпиндель Стоп или Старт

      digitalWrite(motorSpindle, !stateSpindle);     // выключаем мотор шпинделя
      digitalWrite(motorSelfCoolant, !stateSpindle); // выключаем мотор помпы СОЖ
      digitalWrite(electromagnetBottom, true);       // выключить электромагнит движения вниз

      digitalWrite(electromagnetBrake, false);                             // включаем электромагнит растормаживание
      digitalWrite(electromagnetTop, _data.stateElectromagnetTop = false); // включить электромагнит движения вверх
    }

    if (!stateEndCycle) { // Если кнопку Конец Цикла не нажали

      if (stateSpindle) { // Если в ручном режиме ввели в цилиндр и запустили шпиндель и перевели переключатель в Цикл или просто Включён
        lcdPrintString(_lcd, "", "", "", WHITE, NOT_CHANGE_COLOR, 0, 0, 0, true, false);
        digitalWrite(electromagnetBrake, false); // включаем электромагнит растормаживания
        stateStartCycle = true;                  // вход в цикл
      }

#ifdef ENABLE_PROGRAM_SWITCH
      stateSpindle = trigerRS(stateSpindle, !digitalRead(buttonSpindleStart),
                              !digitalRead(buttonSpindleStop) || statePush || stateEndCycle || stateGeneralStop || _data.stateElectromagnetTop); // Шпиндель Стоп или Старт
#endif

#ifdef ENABLE_SWITCH
      stateSpindle = trigerRS(stateSpindle, !digitalRead(buttonSpindleStart),
                              !digitalRead(buttonSpindleStop) || statePush || stateEndCycle || stateGeneralStop || digitalRead(endSwitchBottom)); // Шпиндель Стоп или Старт
#endif

      if (!stateSpindle) { // Шпиндель выключен

        statePush = trigerRS(statePush, !digitalRead(buttonPush),
                             digitalRead(buttonPush) || stateSpindle || stateEndCycle || stateGeneralStop); // Толчковый ввод хоны

#ifdef ENABLE_PROGRAM_SWITCH
        if (statePush && !_data.stateElectromagnetTop || statePush && !_data.stateElectromagnetBottom) { // Если кнопка Толковая нажата и переключатель путевой включён вверх или вниз

          digitalWrite(electromagnetBrake, false);                           // включаем электромагнит растормаживания
          digitalWrite(electromagnetTop, _data.stateElectromagnetTop);       // выключаем электромагнит движения вверх
          digitalWrite(electromagnetBottom, _data.stateElectromagnetBottom); // включаем электромагнит вижения вниз
        }
        if ((!statePush && !_data.stateElectromagnetTop) || (!statePush && !_data.stateElectromagnetBottom)) { // Если кнопка Толковая не нажата и переключатель путевой включён вниз или верх

          digitalWrite(electromagnetBrake, true);  // включаем электромагнит растормаживания
          digitalWrite(electromagnetTop, true);    // выключаем электромагнит движения вверх
          digitalWrite(electromagnetBottom, true); // выключаем электромагнит движения вниз
        }
#endif

#ifdef ENABLE_SWITCH
        if (statePush && !digitalRead(endSwitchTop)) { // Если кнопка Толковая нажата и переключатель путевой включён вверх

          digitalWrite(electromagnetBrake, false);  // включаем электромагнит растормаживания
          digitalWrite(electromagnetTop, true);     // выключаем электромагнит движения вверх
          digitalWrite(electromagnetBottom, false); // включаем электромагнит движения вниз
        }
        if (statePush && !digitalRead(endSwitchBottom)) { // Если кнопка Толковая нажата и переключатель путевой включён вверх

          digitalWrite(electromagnetBrake, false); // включаем электромагнит растормаживания
          digitalWrite(electromagnetTop, false);   // включаем электромагнит движения вверх
          digitalWrite(electromagnetBottom, true);  // выключаем электромагнит движения вниз
        }
        if ((!statePush && !digitalRead(endSwitchTop)) || (!statePush && !digitalRead(endSwitchBottom))) { // Если кнопка Толковая не нажата и переключатель путевой включён вниз или вверх

          digitalWrite(electromagnetBrake, true);  // выключаем электромагнит растормаживания
          digitalWrite(electromagnetTop, true);    // выключаем электромагнит движения вверх
          digitalWrite(electromagnetBottom, true); // выключаем электромагнит движения вниз
        }
#endif
      }
    }
  }
}

void handleManualMode() {
  digitalWrite(electromagnetBrake, false);  // включаем электромагнит растормаживания
  digitalWrite(electromagnetManual, false); // включаем электромагнит ручной подачи

  if (!stateTopSlider) { // Если ползун сошёл с концевика парковки

    stateSpindle = trigerRS(stateSpindle, !digitalRead(buttonSpindleStart),
                            !digitalRead(buttonSpindleStop) || stateEndCycle || stateGeneralStop); // Шпиндель Стоп или Стоп

#ifdef ENABLE_PROGRAM_SWITCH
    if (stateSpindle && ((_data.limitTop - 5 /*мм*/) < _data.linearMove) && (_data.linearMove < (_data.limitBottom + 5 /*мм*/))) {
      digitalWrite(motorSpindle, !stateSpindle);     // включение выключение мотора шпинделя
      digitalWrite(motorSelfCoolant, !stateSpindle); // включение выключение мотора помпы СОЖ
    } else {
      stateSpindle = false;
    }
#endif

#ifdef ENABLE_SWITCH
    if (stateSpindle) {
      digitalWrite(motorSpindle, !stateSpindle);     // включение выключение мотора шпинделя
      digitalWrite(motorSelfCoolant, !stateSpindle); // включение выключение мотора помпы СОЖ
    }
#endif

    if (!stateSpindle) {
      digitalWrite(motorSpindle, !stateSpindle);     // включение выключение мотора шпинделя
      digitalWrite(motorSelfCoolant, !stateSpindle); // включение выключение мотора помпы СОЖ
      /**
        LCD DISPLAY AND BUTTONS READLIMIT TOP PROG
      */
      Menu();
    }
  }
}

void handleStop() {
  stateSpindle = trigerRS(stateSpindle, !digitalRead(buttonSpindleStart),
                          !digitalRead(buttonSpindleStop) || stateEndCycle || stateGeneralStop); // Шпиндель Стоп или Старт

  digitalWrite(motorSpindle, !stateSpindle);     // выключение мотора шпинделя
  digitalWrite(motorSelfCoolant, !stateSpindle); // выключаем мотор помпы СОЖ
  digitalWrite(electromagnetTop, true);          // выключить электромагнит движения вверх
  digitalWrite(electromagnetBottom, true);       // выключить электромагнит движения вниз
  digitalWrite(electromagnetManual, true);       // выключаем электромагнит ручной подачи
  digitalWrite(electromagnetBrake, true);        // выключаем электромагнит растормаживания
}

void handleCycle() {

  handleProgramSwitch();

  _data.linearMove = getLinearMotion(); // Получаем данные с энкодера

  handleButtonStates();
  handleMotorStates();

  if (stateAutoCycleManual) { // Переключатель включен в режим Цикл

    digitalWrite(electromagnetManual, true); // выключить электромагнит ручного управления если был на ручном

    stateSpindle = trigerRS(stateSpindle, !digitalRead(buttonSpindleStart),
                            !digitalRead(buttonSpindleStop) || stateEndCycle || stateGeneralStop); // Шпиндель Стоп или Старт

    if (stateEndCycle) { // Если кнопку Конец Цикла нажали

      digitalWrite(motorSpindle, !stateSpindle);     // выключаем мотор шпинделя
      digitalWrite(motorSelfCoolant, !stateSpindle); // выключаем мотор помпы СОЖ
      digitalWrite(electromagnetBottom, true);       // выключить электромагнит движения вниз
      digitalWrite(electromagnetTop, false);         // включить электромагнит движения вверх

      if (stateTopSlider) { // Ползун поднялся в верх, исходное состояние конец цикла

        digitalWrite(electromagnetTop, true);   // выключить электромагнит движения вверх
        digitalWrite(electromagnetBrake, true); // выключаем электромагнит растормаживания
        stateEndCycle = false;                  // сбрасываем состояние выхода из цикла
        stateStartCycle = false;                // выходим из цикла
        // break;
      }
    }

    if (!stateEndCycle && stateSpindle) { // Если кнопку Конец Цикла не нажали, Проверяем состояние шпинделя Если включен

      digitalWrite(motorSpindle, !stateSpindle);     // включить мотор шпинделя
      digitalWrite(motorSelfCoolant, !stateSpindle); // включаем мотор помпы СОЖ

#if defined(ENABLE_PROGRAM_SWITCH) && !defined(ENABLE_SWITCH)
      if (!data.stateElectromagnetBottom) { // Если переключатель включен в положение верх

        digitalWrite(electromagnetTop, data.stateElectromagnetTop); // выключить электромагнит движения вверх
        delay(100);
        digitalWrite(electromagnetBottom, data.stateElectromagnetBottom); // включить электромагнит движения вниз
      }

      if (!data.stateElectromagnetTop) { // Если переключатель включен в положение низ

        digitalWrite(electromagnetBottom, data.stateElectromagnetBottom); // выключить электромагнит движения вниз
        delay(100);
        digitalWrite(electromagnetTop, data.stateElectromagnetTop); // включить электромагнит движения вверх
      }
#endif

#if defined(ENABLE_PROGRAM_SWITCH) && defined(ENABLE_SWITCH)
      if (!_data.stateElectromagnetBottom || !digitalRead(endSwitchTop)) { // Если переключатель включен в положение верх

        digitalWrite(electromagnetTop, true); // выключить электромагнит движения вверх
        delay(100);
        digitalWrite(electromagnetBottom, false); // включить электромагнит движения вниз
      }
      if (!_data.stateElectromagnetTop || !digitalRead(endSwitchBottom)) { // Если переключатель включен в положение низ

        digitalWrite(electromagnetBottom, true); // выключить электромагнит движения вниз
        delay(100);
        digitalWrite(electromagnetTop, false); // включить электромагнит движения вверх
      }
#endif

#if defined(ENABLE_SWITCH) && !defined(ENABLE_PROGRAM_SWITCH)
      if (!digitalRead(endSwitchTop)) { // Если переключатель включен в положение верх

        digitalWrite(electromagnetTop, true); // выключить электромагнит движения вверх
        delay(100);
        digitalWrite(electromagnetBottom, false); // включить электромагнит движения вниз
      }
      if (!digitalRead(endSwitchBottom)) { // Если переключатель включен в положение низ

        digitalWrite(electromagnetBottom, true); // выключить электромагнит движения вниз
        delay(100);
        digitalWrite(electromagnetTop, false); // включить электромагнит движения вверх
      }
#endif
    }

    if (!stateEndCycle && !stateSpindle) { // Если кнопку Конец Цикла не нажали, Проверяем состояние шпинделя Если выключен

      digitalWrite(motorSpindle, !stateSpindle);     // отключаем мотор шпинделя
      digitalWrite(motorSelfCoolant, !stateSpindle); // отключаем мотор помпы СОЖ

#ifdef ENABLE_PROGRAM_SWITCH
      if (!_data.stateElectromagnetBottom) { // Если переключатель включен в положение верх

        digitalWrite(electromagnetTop, _data.stateElectromagnetTop); // выключить электромагнит движения вверх
        delay(100);
        digitalWrite(electromagnetBottom, _data.stateElectromagnetBottom); // включить электромагнит движения вниз
      }

      if (!_data.stateElectromagnetTop) { // Если переключатель включен в положение низ

        digitalWrite(electromagnetBottom, _data.stateElectromagnetBottom); // выключить электромагнит движения вниз
        delay(100);
        digitalWrite(electromagnetTop, _data.stateElectromagnetTop); // включить электромагнит движения вверх
      }
#endif

#ifdef ENABLE_SWITCH
      if (!digitalRead(endSwitchTop)) { // Если переключатель включен в положение верх

        digitalWrite(electromagnetTop, true); // выключить электромагнит движения вверх
        delay(100);
        digitalWrite(electromagnetBottom, false); // включить электромагнит движения вниз
      }

      if (!digitalRead(endSwitchBottom)) { // Если переключатель включен в положение низ

        digitalWrite(electromagnetBottom, true); // выключить электромагнит движения вниз
        delay(100);
        digitalWrite(electromagnetTop, false); // включить электромагнит движения вверх
      }
#endif
    }
  }

  if (!stateAutoCycleManual) { // Переключатель включен в режим Ручной

    stateSpindle = trigerRS(stateSpindle, !digitalRead(buttonSpindleStart),
                            !digitalRead(buttonSpindleStop) || stateEndCycle || stateGeneralStop); // Шпиндель Стоп или Старт

    digitalWrite(electromagnetTop, true);    // выключить электромагнит движения вверх
    digitalWrite(electromagnetBottom, true); // выключить электромагнит движения вниз

    digitalWrite(electromagnetManual, false); // включить электромагнит ручного управления

    digitalWrite(motorSpindle, !stateSpindle);     // включить мотор шпинделя или отключить в зависимости от stateSpindle
    digitalWrite(motorSelfCoolant, !stateSpindle); // включить мотор помпы СОЖ или отключить в зависимости от stateSpindle

    if (!stateSpindle) { // Если шпиндель выключен, есть возможность запустить меню настройки

      /////////////////////////////////////////////////////LCD DISPLAY BUTTONS READ///////////////////////////////////////////////////////
      Menu(); // Запуск меню настроек по удержанию кнопки в течение 3 секунд
    }
  }

  if (stateGeneralStop) {
    // Логика общего стопа
    handleGeneralStop();
  }
}

void handleGeneralStop() {
  // Нажата кнопка Общий стоп
  digitalWrite(motorSpindle, !stateSpindle);     // отключаем мотор шпинделя
  digitalWrite(motorSelfCoolant, !stateSpindle); // отключаем мотор помпы СОЖ
  digitalWrite(motorStartFeed, !stateStartFeed); // отключаем мотор возвратно поступательного движения

  digitalWrite(electromagnetTop, true); // выключить электромагнит движения вверх

  digitalWrite(electromagnetBottom, true); // выключить электромагнит движения вниз
  digitalWrite(electromagnetBrake, true);  // выключить электромагнит растормаживания
  digitalWrite(electromagnetManual, true); // выключить электромагнит ручного управления

  stateStartCycle = false; // выходим из цикла

  /////////////////////////////////////////////////////EEPROM SAVE///////////////////////////////////////////////////////
  saveEeprom(_lcd, _dataBuffer, _data);
}