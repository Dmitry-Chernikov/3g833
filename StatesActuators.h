#pragma once

//Состояние режимов
volatile bool stateEndCycle = false;         // Состояние Конец-Цикла
volatile bool statePush = false;             // Состояние Толчок-Ползун
volatile bool stateStartFeed = false;        // Состояние Подача-Пуск
volatile bool stateSpindle = false;          // Состояние Шпиндель-Старт\Стоп
volatile bool stateAutoCycleManual = false;  // Состояние Цикл-Ручной
volatile bool stateTopSlider = false;        // Состояние концевика верхнего положения ползуна
volatile bool stateStartCycle = false;       // Состояние Цикла
volatile bool stateGeneralStop = true;       // Состояние Общий Стоп
