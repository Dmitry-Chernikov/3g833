#pragma once

#include <Arduino.h>

// Состояние режимов
extern volatile bool stateEndCycle;        // Состояние Конец-Цикла
extern volatile bool statePush;            // Состояние Толчок-Ползун
extern volatile bool stateStartFeed;       // Состояние Подача-Пуск
extern volatile bool stateSpindle;         // Состояние Шпиндель-Старт\Стоп
extern volatile bool stateAutoCycleManual; // Состояние Цикл-Ручной
extern volatile bool stateTopSlider;       // Состояние концевика верхнего положения ползуна
extern volatile bool stateStartCycle;      // Состояние Цикла
extern volatile bool stateGeneralStop;     // Состояние Общий Стоп

bool trigerRS(bool currentState, uint8_t TrigSet, uint8_t TrigReset);

bool stateMillisDelay(unsigned long *previousMillis, const unsigned long *Interval);