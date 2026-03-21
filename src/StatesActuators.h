#pragma once

#include <Arduino.h>

// Состояние режимов
extern volatile bool stateEndCycle;				// Состояние Конец-Цикла
extern volatile bool statePush;					// Состояние Толчок-Ползун
extern volatile bool stateStartFeed;			// Состояние Подача-Пуск
extern volatile bool stateSpindle;				// Состояние Шпиндель-Старт\Стоп
extern volatile bool stateSelfCoolant;			// Состояние Смазочно охлаждающей системы
extern volatile bool stateElectromagnetBrake;	// Состояние Электромагнита растормаживания возвратно-поступательного движения
extern volatile bool stateElectromagnetManual;	// Состояние Электромагнитной муфты ручного управления подачей ползуна
extern volatile bool stateAutoCycleManual;		// Состояние Цикл-Ручной
extern volatile bool stateTopSlider;			// Состояние концевика верхнего положения ползуна
extern volatile bool stateStartCycle;			// Состояние Цикла
extern volatile bool stateGeneralStop;			// Состояние Общий Стоп

bool triggerRS(bool currentState, uint8_t trigSet, uint8_t trigReset);

bool stateMillisDelay(unsigned long *previousMillis, const unsigned long *interval);