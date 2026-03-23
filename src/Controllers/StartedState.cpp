#include "StartedState.h"

#include <Arduino.h>

#include "CycleState.h"
#include "ManualState.h"
#include "StoppedState.h"

// ========== StartedState ==========
void StartedState::onEnter() {
	ctx->setFeedMotor(true); /* Включаем мотор возвратно-поступательного движения */
	lastAutoMode = ctx->stateAutoCycleManual;

	// Сброс энкодера если ползун находится на верхнем концевике парковки ползуна
	if (ctx->stateTopSlider) {
		ctx->data.absoluteAngle = 0;
		ctx->data.anglePrevious = getAngle();
	}
}

void StartedState::onExit() {
	// Ничего особенного
}

void StartedState::onUpdate() {
	// Обновляем позицию
	ctx->updatePosition();

	// Обновляем программные концевики
	ctx->updateProgramSwitches();

	// Показываем позицию на LCD
	if (millis() - lastLcdTime > 500) {
		lcdPrintString(ctx->lcd, "POSITION", String(ctx->data.linearMove, 2), "mm",
		YELLOW, NOT_CHANGE_COLOR, 0, 0, 0, true, false);
		lastLcdTime = millis();
	}
}

void StartedState::onButtonGeneralStopPressed() {
	ctx->transitionTo(new StoppedState());
}

void StartedState::onButtonStartFeedReleased() {
}

void StartedState::onButtonPushPressed() {
	ctx->statePush = triggerRS(ctx->statePush, true, false);
}

void StartedState::onButtonPushReleased() {

}

void StartedState::onButtonSpindleStartPressed() {
	ctx->setSpindle(true);
}

void StartedState::onButtonSpindleStopPressed() {
	ctx->setSpindle(false);
}

void StartedState::onSwitchModeChanged(bool autoMode) {
	ctx->stateAutoCycleManual = autoMode;

	if (autoMode && ctx->stateSpindle) {
		// Переходим в цикл если шпиндель включен
		ctx->transitionTo(new CycleState());
	} else if (!autoMode) {
		// Переходим в ручной режим
		ctx->transitionTo(new ManualState());
	}
}

void StartedState::onTopSliderReached(bool reached) {
	ctx->stateTopSlider = reached;
}