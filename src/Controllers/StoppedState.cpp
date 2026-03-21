#include "StoppedState.h"

#include <Arduino.h>

#include "StartedState.h"
#include "TextMenu.h"

// ========== StoppedState ==========
void StoppedState::onEnter() {
	// Выключаем всё
	ctx->setSpindle(false);
	ctx->setFeedMotor(false);
	ctx->moveUp(false);
	ctx->moveDown(false);
	ctx->releaseBrake(false);
	ctx->setManualMode(false);

	lastMenuTime = millis();
}

void StoppedState::onExit() {
	ctx->saveToEEPROM();
}

void StoppedState::onUpdate() {
	// Показываем меню
	if (millis() - lastMenuTime > 100) {
		Menu();  // Твоя существующая функция меню
		lastMenuTime = millis();
	}
}

void StoppedState::onButtonGeneralStopReleased() {
	// Кнопка общей стоп отпущена - можно переходить в другое состояние
}

void StoppedState::onButtonStartFeedPressed() {
	ctx->transitionTo(new StartedState());
}

void StoppedState::onTopSliderReached(bool reached) {
	ctx->stateTopSlider = reached;
}

