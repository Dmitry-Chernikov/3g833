#include "MachineContext.h"
#include <util/atomic.h>

#include "MachineStates.h"

// Инициализация статического экземпляра
MachineContext* MachineContext::instance_ = nullptr;

MachineContext::MachineContext(Data& dataRef, Data& bufferRef,
                               Adafruit_RGBLCDShield& lcdRef, HS321& hs321Ref)
    : currentState_(nullptr)
    , lastDebounceTime_(0)
    , lastLcdUpdate_(0)
    , lastEepromSave_(0)
    , buttonsChanged_(false)
    , buttonStates_(0)
    , data(dataRef)
    , dataBuffer(bufferRef)
    , stateEndCycle(::stateEndCycle)
    , statePush(::statePush)
    , stateStartFeed(::stateStartFeed)
    , stateSpindle(::stateSpindle)
	, stateSelfCoolant(::stateSelfCoolant)
	, stateElectromagnetBrake(::stateElectromagnetBrake)
	, stateElectromagnetManual(::stateElectromagnetManual)
    , stateAutoCycleManual(::stateAutoCycleManual)
    , stateTopSlider(::stateTopSlider)
    , stateStartCycle(::stateStartCycle)
    , stateGeneralStop(::stateGeneralStop)
    , lcd(lcdRef)
    , hs321(hs321Ref)
{
    instance_ = this;
}

MachineContext::~MachineContext() {
    if (currentState_) {
        delete currentState_;
    }
    saveToEEPROM();
    instance_ = nullptr;
}

// Методы переходя между состояниями
void MachineContext::transitionTo(MachineStates* newState) {
	/* Выход из текущего состояния */
    if (currentState_) {
        currentState_->onExit();	// 1. Сохранить данные, выключить моторы
        delete currentState_;		// 2. Удалить старое состояние
    }
	/* Вход в новое состояние */
    currentState_ = newState;			// 3. Установить новое состояние
    currentState_->setContext(this);	// 4. Дать ссылку на контекст
    currentState_->onEnter();			// 5. Инициализация нового состояния
    
    Serial.print("Состояние изменено на: ");
    Serial.println(currentState_->getName());
}

void MachineContext::update() {
    // Обработка изменений из прерывания
    processButtonChanges();
    
    // Чтение концевиков
    bool topSlider = !digitalRead(switchTopSlider);
    if (topSlider != stateTopSlider) {
        onTopSliderReached(topSlider);
    }
    
    // Чтение механических концевиков
    if (!digitalRead(endSwitchTop)) {
        onEndSwitchTopTriggered();
    }
    if (!digitalRead(endSwitchBottom)) {
        onEndSwitchBottomTriggered();
    }
    
    // Чтение переключателя режимов
    bool autoMode = digitalRead(switchAutoCycleManual);
    if (autoMode != stateAutoCycleManual) {
        onSwitchModeChanged(autoMode);
    }
    
    // Обновление позиции
    updatePosition();
    
    // Обновление текущего состояния
    if (currentState_) {
        currentState_->onUpdate();
    }
    
    // Периодическое сохранение в EEPROM
    if (millis() - lastEepromSave_ > 5000) {
        saveToEEPROM();
        lastEepromSave_ = millis();
    }
}

void MachineContext::onButtonGeneralStopPressed() const {
	currentState_->onButtonGeneralStopPressed();
}

void MachineContext::onButtonGeneralStopReleased() const {
	currentState_->onButtonGeneralStopReleased();
}

void MachineContext::onButtonStartFeedPressed() const {
	currentState_->onButtonStartFeedPressed();
}

void MachineContext::onButtonStartFeedReleased() const {
	currentState_ ->onButtonStartFeedReleased();
}

void MachineContext::onButtonEndCyclePressed() const {
	currentState_->onButtonEndCyclePressed();
}

void MachineContext::onButtonPushPressed() const {
	currentState_->onButtonPushPressed();
}

void MachineContext::onButtonSpindleStartPressed() const {
	currentState_->onButtonSpindleStartPressed();
}

void MachineContext::onButtonSpindleStopPressed() const {
	currentState_->onButtonSpindleStopPressed();
}

void MachineContext::onSwitchModeChanged(const bool autoMode) const {
	currentState_->onSwitchModeChanged(autoMode);
}

void MachineContext::onTopSliderReached(const bool reached) const {
	currentState_->onTopSliderReached(reached);
}

void MachineContext::onEndSwitchTopTriggered() const {
	currentState_->onEndSwitchTopTriggered();
}

void MachineContext::onEndSwitchBottomTriggered() const {
	currentState_->onEndSwitchBottomTriggered();
}

// Приватные методы для обработки кнопок и состояний концевика и т.д.
void MachineContext::processButtonChanges() {
	// Проверяем флаг ДО чтения
	if (!buttonsChanged_) return;
    
    uint8_t states;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        states = buttonStates_; // Копируем во временную переменную и сбрасываем флаг
    }

    static uint8_t lastStableStates = 0xFF; // Все кнопки не нажаты по умолчанию
    static unsigned long lastDebounce = 0; // Время последнего обновления анти-дребезга
	static uint8_t lastRawStates = 0xFF;

	// Состояние изменено с прошлого раза?
    if (states != lastStableStates) {
    	lastRawStates = states;
        lastDebounce = millis(); // Записали время изменения события изменения состояния кнопок
    	return; // Ждем стабилизации
    }

	// Проверяем стабильность
    if ((millis() - lastDebounce) > 50) { // Период ожидания анти-дребезга вышел
        if (states != lastStableStates) { // Сравниваем состояние после периода ожидания анти-дребезга
            const uint8_t changed = states ^ lastStableStates; // Сохраняем битовую маску всех отличающихся позиций
            
            // Карта пинов на биты (настрой под свою распиновку)
            if (changed & (1 << 0)) {  // buttonGeneralStop
                if (!(states & (1 << 0))) onButtonGeneralStopPressed();
                else onButtonGeneralStopReleased();
            }
            if (changed & (1 << 1)) {  // buttonStartFeed
                if (!(states & (1 << 1))) onButtonStartFeedPressed();
                else onButtonStartFeedReleased();
            }
            if (changed & (1 << 2)) {  // buttonEndCycle
                if (!(states & (1 << 2))) onButtonEndCyclePressed();
            }
            if (changed & (1 << 3)) {  // buttonPush
                if (!(states & (1 << 3))) onButtonPushPressed();
            }
            if (changed & (1 << 4)) {  // buttonSpindleStart
                if (!(states & (1 << 4))) onButtonSpindleStartPressed();
            }
            if (changed & (1 << 5)) {  // buttonSpindleStop
                if (!(states & (1 << 5))) onButtonSpindleStopPressed();
            }
            
            lastStableStates = states;
        }
    	// Сбрасываем флаг
    	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        	buttonsChanged_ = false;
        }
    }
}

void MachineContext::onButtonChange(const uint8_t pin, const bool state) {
    const uint8_t bitMask = 1 << pin; // Маска нужного бита
	// Кнопка была нажата или отжата?
    if (state) {
        buttonStates_ |= bitMask; // Если нажата, то ставим бит в единицу
    } else {
        buttonStates_ &= ~bitMask; // В противном случае убираем его
    }
    buttonsChanged_ = true;
}

/* Управление железом */

// Управление мотором возвратно-поступательного движения
void MachineContext::setFeedMotor(const bool on) const {
	digitalWrite(motorStartFeed, !on);
	stateStartFeed = on;
}

// Управление мотором шпинделя
void MachineContext::setSpindle(const bool on) const {
    digitalWrite(motorSpindle, !on);
    stateSpindle = on;
}

// Управление мотором помпы СОЖ
void MachineContext::setSelfCoolant(const bool on) const {
	digitalWrite(motorSelfCoolant, !on);
	stateSelfCoolant = on;
}

// Управление электромагнитом растормаживание
void MachineContext::releaseBrake(const bool release) const {
	digitalWrite(electromagnetBrake, !release);
	stateElectromagnetBrake = release;
}

// Управление электромагнитом движения вверх
void MachineContext::moveUp(const bool enable) const {
    digitalWrite(electromagnetTop, !enable);
	// Сохраняем состояние электромагнита в data для сохранения в eeprom, на случай отключения питания, для восстановления
    data.stateElectromagnetTop = enable;
}

// Управление электромагнитом движения вниз
void MachineContext::moveDown(const bool enable) const {
    digitalWrite(electromagnetBottom, !enable);
	// Сохраняем состояние электромагнита в data для сохранения в eeprom, на случай отключения питания, для восстановления
    data.stateElectromagnetBottom = enable;
}

// Управление электромагнитом ручной подачи возвратно-поступательного движения
void MachineContext::setManualMode(const bool enable) const {
    digitalWrite(electromagnetManual, !enable);
	stateElectromagnetManual = enable;
}

// Чтение значение энкодера, возвращает значение линейное положения ползуна в мм
void MachineContext::updatePosition() const {
    data.linearMove = getLinearMotion();
}

void MachineContext::updateProgramSwitches() const {
#ifdef ENABLE_PROGRAM_SWITCH
    if (data.linearMove <= data.limitTop) {
        data.stateElectromagnetTop = true; // Включить состояние вверх программного переключателя
        data.stateElectromagnetBottom = false; // Выключить состояние вниз программного переключателя
        data.stateIntermediate = true; // Ползун вышел из промежуточного состояния
    } else if (data.linearMove >= data.limitBottom) {
        data.stateElectromagnetTop = false; // Выключить состояние вверх программного переключателя
        data.stateElectromagnetBottom = true; // Включить состояние вниз программного переключателя
        data.stateIntermediate = true; // Ползун вышел из промежуточного состояния
    } else {
    	// Ползун в промежуточном состоянии
        data.stateIntermediate = false;
    }
#endif
}

void MachineContext::saveToEEPROM() const {
    ::saveEeprom(lcd, dataBuffer, data);
}
