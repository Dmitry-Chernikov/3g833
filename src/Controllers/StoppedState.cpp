#include "StoppedState.h"

#include <Arduino.h>
#include "StartedState.h"
#include "TextMenu.h"

// ========== StoppedState ==========
void StoppedState::onEnter() {
	/* Полная остановка всех исполнительных механизмов */
	ctx->setSpindle(false);			/* Отключаем мотор шпинделя */
	ctx->setSelfCoolant(false);		/* Отключаем мотор системы подачи смазочно-охлаждающей жидкости */
	ctx->setFeedMotor(false);		/* Отключаем мотор возвратно-поступательного движения */
	ctx->moveUp(false);			/* Отключаем электромагнитную муфту подъема ползуна */
	ctx->moveDown(false);		/* Отключаем электромагнитную муфту спускания ползуна */
	ctx->setManualMode(false);	/* Отключаем электромагнитную муфту ручного управления перемещения ползуна */
	ctx->releaseBrake(false);			/* Отключаем электромагнит растормаживания, ленточный тормоз блокирует перемещение ползуна */

	ctx->saveToEEPROM();  /* Сохраняем все настройки */
}

void StoppedState::onExit() {}

void StoppedState::onUpdate() {
	Menu();  // При долгом нажатии кнопки дисплея открывается меню настройки станка
}

void StoppedState::onButtonGeneralStopPressed() {}

void StoppedState::onButtonGeneralStopReleased() {
	// Кнопка общей стоп отпущена - можно переходить в другое состояние
}

void StoppedState::onButtonStartFeedPressed() {
	ctx->transitionTo(new StartedState());
}

void StoppedState::onTopSliderReached(bool reached) {
	ctx->stateTopSlider = reached;
}

