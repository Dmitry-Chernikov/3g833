#include "StatesActuators.h"

// Состояние режимов
volatile bool stateEndCycle = false;			// Состояние Конец-Цикла
volatile bool statePush = false;				// Состояние Толчок-Ползун
volatile bool stateStartFeed = false;			// Состояние Подача-Пуск
volatile bool stateSpindle = false;				// Состояние Шпиндель-Старт\Стоп
volatile bool stateSelfCoolant = false;			// Состояние Смазочно охлаждающей системы
volatile bool stateElectromagnetBrake = false;	// Состояние Электромагнита растормаживания возвратно-поступательного движения
volatile bool stateElectromagnetManual = false;	// Состояние Электромагнитной муфты ручного управления подачей ползуна
volatile bool stateAutoCycleManual = false;		// Состояние Цикл-Ручной
volatile bool stateTopSlider = false;			// Состояние концевика верхнего положения ползуна
volatile bool stateStartCycle = false;			// Состояние Цикла
volatile bool stateGeneralStop = true;			// Состояние Общий Стоп

bool triggerRS(const bool currentState, const uint8_t trigSet, const uint8_t trigReset) {
    // Триггер с приоритетом сброса
    if (trigReset) {
        return false;
    }
    if (trigSet) {
        return true;
    }
    return currentState;
}

bool stateMillisDelay(unsigned long *previousMillis, const unsigned long *interval) {
    // unsigned long currentMillis = millis();

    if (*previousMillis == 0) {
        *previousMillis = millis();
    }

    // проверяем не прошел ли нужный интервал, если прошел то
    if ((millis() - *previousMillis) >= *interval) {
        // обнуляем предыдущее значение millis()
        *previousMillis = 0;
        return HIGH;
    }
    return LOW;
}