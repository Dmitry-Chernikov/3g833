#pragma once

#include <Arduino.h>
#include "MemoryEeprom.h"
#include "StatesActuators.h"
#include "IOPorts.h"
#include "Display.h"
#include "Encoder.h"
#include "HS321.h"
#include "config.h"

// Предварительное объявление состояний для избежания циклических зависимостей
class MachineStates;

class MachineContext {
private:
    MachineStates* currentState_;

    // Время для задержек
    unsigned long lastDebounceTime_;
    unsigned long lastLcdUpdate_;
    unsigned long lastEepromSave_;

    // Для прерываний (volatile!)
    static MachineContext* instance_;  // Синглтон для прерывания
    volatile bool buttonsChanged_;
    volatile uint8_t buttonStates_;

public:
    // Данные из EEPROM (прямая ссылка на глобальные переменные)
    Data& data;
    Data& dataBuffer;

    /* Флаги состояний (теперь будут обновляться автоматически при инициализации контекста машины состояния с помощью конструктора глобальными переменными StatesActuators) */
    volatile bool& stateEndCycle;				/* Состояние Конец-Цикла */
    volatile bool& statePush;					/* Состояние Толчок-Ползун */
    volatile bool& stateStartFeed;				/* Состояние Подача-Пуск */
    volatile bool& stateSpindle;				/* Состояние Шпиндель-Старт\Стоп */
	volatile bool& stateSelfCoolant;			/* Состояние Смазочно охлаждающей системы */
	volatile bool& stateElectromagnetBrake;		/* Состояние Электромагнита растормаживания возвратно-поступательного движения */
	volatile bool& stateElectromagnetManual;	/* Состояние Электромагнитной муфты ручного управления подачей ползуна */
    volatile bool& stateAutoCycleManual;		/* Состояние АвтоЦикл\Ручной режим */
    volatile bool& stateTopSlider;				/* Состояние концевика верхнего положения ползуна */
    volatile bool& stateStartCycle;				/* Состояние Цикла*/
    volatile bool& stateGeneralStop;			/* Состояние Общий Стоп */

    // Объекты железа
    Adafruit_RGBLCDShield& lcd;
    HS321& hs321;

    // Конструктор
    MachineContext(Data& dataRef, Data& bufferRef, Adafruit_RGBLCDShield& lcdRef, HS321& hs321Ref);

    // Деструктор
    ~MachineContext();

    // Получение экземпляра для прерывания
    static MachineContext* getInstance() {
	    return instance_;
    }

    // Переход в новое состояние
    void transitionTo(MachineStates* newState);

    // Получение текущего состояния
    MachineStates* getCurrentState() const {
	    return currentState_;
    }

    // Основной цикл обновления
    void update();

    // Обработчики событий (вызываются из состояний или прерываний)
	void onButtonGeneralStopPressed() const;
    void onButtonGeneralStopReleased() const;
    void onButtonStartFeedPressed() const;
    void onButtonStartFeedReleased() const;
    void onButtonEndCyclePressed() const;
    void onButtonPushPressed() const;
    void onButtonSpindleStartPressed() const;
    void onButtonSpindleStopPressed() const;
    void onSwitchModeChanged(bool autoMode) const;
    void onTopSliderReached(bool reached) const;
    void onEndSwitchTopTriggered() const;
    void onEndSwitchBottomTriggered() const;

    // Метод для прерывания
    void onButtonChange(uint8_t pin, bool state);

    // Обработка накопленных изменений из прерывания
    void processButtonChanges();

    /* Управление железом */
	void setFeedMotor(bool on) const;
    void setSpindle(bool on) const;
	void setSelfCoolant(bool on) const;
	void releaseBrake(bool release) const;
    void moveUp(bool enable) const;
    void moveDown(bool enable) const;
	void setManualMode(bool enable) const;

    // Обновление энкодера и позиции
    void updatePosition() const;

    // Сохранение в EEPROM
    void saveToEEPROM() const;

    // Получение линейного перемещения
    float getLinearMove() const {
	    return data.linearMove;
    }

    // Проверка программных концевиков
    void updateProgramSwitches() const;
};
