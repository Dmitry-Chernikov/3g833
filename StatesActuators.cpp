#include "StatesActuators.h"

// Состояние режимов
volatile bool stateEndCycle = false;        // Состояние Конец-Цикла
volatile bool statePush = false;            // Состояние Толчок-Ползун
volatile bool stateStartFeed = false;       // Состояние Подача-Пуск
volatile bool stateSpindle = false;         // Состояние Шпиндель-Старт\Стоп
volatile bool stateAutoCycleManual = false; // Состояние Цикл-Ручной
volatile bool stateTopSlider = false;       // Состояние концевика верхнего положения ползуна
volatile bool stateStartCycle = false;      // Состояние Цикла
volatile bool stateGeneralStop = true;      // Состояние Общий Стоп

bool trigerRS(bool currentState, uint8_t TrigSet, uint8_t TrigReset) { // Триггер с приоритетом сброса
  if (TrigReset) {
    return false;
  }
  if (TrigSet) {
    return true;
  }
  return currentState;
}

bool stateMillisDelay(unsigned long *previousMillis, const unsigned long *Interval) { // unsigned long currentMillis = millis();

  if (*previousMillis == 0) {
    *previousMillis = millis();
  }

  // проверяем не прошел ли нужный интервал, если прошел то
  if ((millis() - *previousMillis) >= *Interval) {
    // обнуляем предыдущее значение millis()
    *previousMillis = 0;
    return HIGH;
  }
  return LOW;
}