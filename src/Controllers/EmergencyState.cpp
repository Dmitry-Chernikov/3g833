#include "EmergencyState.h"

// ========== EmergencyState ==========
void EmergencyState::onEnter() {
	// ВСЁ ВЫКЛЮЧИТЬ НЕМЕДЛЕННО!
	ctx->setSpindle(false);
	ctx->setFeedMotor(false);
	ctx->moveUp(false);
	ctx->moveDown(false);
	ctx->releaseBrake(false);
	ctx->setManualMode(false);

	ctx->hs321.writeSingleRegister(0x2000, 5);  // Свободный останов

	lcdPrintString(ctx->lcd, "EMERGENCY", "STOP", "", RED, RED, 0, 0, 0, true, false);
}

void EmergencyState::onExit() {
	lcdPrintString(ctx->lcd, "", "", "", WHITE, WHITE, 0, 0, 0, true, true);
	ctx->saveToEEPROM();
}

void EmergencyState::onUpdate() {
	// Ничего не делаем, ждем снятия аварии
}

void EmergencyState::onButtonGeneralStopReleased() {
	// Если кнопка отпущена - возвращаемся в останов
	ctx->transitionTo(new StoppedState());
}