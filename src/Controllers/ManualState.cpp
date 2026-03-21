#include "ManualState.h"

#include <Arduino.h>

#include "StartedState.h"
#include "StoppedState.h"


// ========== ManualState ==========
void ManualState::onEnter() {
	ctx->releaseBrake(true);
	ctx->setManualMode(true);
}

void ManualState::onExit() {
	ctx->setManualMode(false);
}

void ManualState::onUpdate() {
	ctx->updatePosition();

	// Ограничение перемещения в ручном режиме
	if (ctx->stateSpindle) {
		float pos = ctx->getLinearMove();
		if (pos < ctx->data.limitTop || pos > ctx->data.limitBottom) {
			ctx->setSpindle(false);
			lcdPrintString(ctx->lcd, "LIMIT", "STOP", "", RED, WHITE, 0, 0, 1000, true, true);
		}
	}
}

void ManualState::onButtonGeneralStopPressed() {
	ctx->transitionTo(new StoppedState());
}

void ManualState::onButtonStartFeedReleased() {
	ctx->transitionTo(new StartedState());
}

void ManualState::onButtonPushPressed() {
	// Толчковый режим вручную
	ctx->releaseBrake(true);
	ctx->moveDown(true);
	delay(100);
	ctx->moveDown(false);
	ctx->statePush = true;
}

void ManualState::onButtonSpindleStartPressed() {
	ctx->setSpindle(true);
}

void ManualState::onButtonSpindleStopPressed() {
	ctx->setSpindle(false);
}

void ManualState::onSwitchModeChanged(bool autoMode) {
	ctx->stateAutoCycleManual = autoMode;
	if (autoMode) {
		ctx->transitionTo(new CycleState());
	}
}

void ManualState::onTopSliderReached(bool reached) {
	ctx->stateTopSlider = reached;
}