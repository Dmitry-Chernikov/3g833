#include "CycleState.h"

#include <Arduino.h>

#include "EmergencyState.h"
#include "ManualState.h"
#include "StartedState.h"

// ========== CycleState ==========
void CycleState::onEnter() {
    ctx->setManualMode(false);
    ctx->releaseBrake(true);
    movingDown = true;

    // Включаем шпиндель если выключен
    if (!ctx->stateSpindle) {
        ctx->setSpindle(true);
    }

    // Настройка частотника через Modbus
    ctx->hs321.setFrequencySetpoint( 5000); // 50.00 Гц
    ctx->hs321.writeControlCommand(FORWARD_RUN_COMMAND);    // Пуск вперед
}

void CycleState::onExit() {
    ctx->hs321.writeControlCommand(DECELERATE_STOP_COMMAND);    // Останов
    ctx->moveUp(false);
    ctx->moveDown(false);
}

void CycleState::onUpdate() {
    ctx->updatePosition();
    ctx->updateProgramSwitches();

    // Управление движением
    if (movingDown) {
        ctx->moveUp(false);
        ctx->moveDown(true);

        // Достигли низа?
        if (ctx->data.linearMove >= ctx->data.limitBottom) {
            movingDown = false;
            delay(100);
        }
    } else {
        ctx->moveDown(false);
        ctx->moveUp(true);

        // Достигли верха?
        if (ctx->data.linearMove <= ctx->data.limitTop) {
            movingDown = true;
            delay(100);
        }
    }
}

void CycleState::onButtonGeneralStopPressed() {
    ctx->transitionTo(new EmergencyState());
}

void CycleState::onButtonEndCyclePressed() {
    ctx->stateEndCycle = true;
    movingDown = false;  // Начинаем подъем
    ctx->moveDown(false);
    ctx->moveUp(true);
}

void CycleState::onButtonSpindleStartPressed() {
    // Уже включен
}

void CycleState::onButtonSpindleStopPressed() {
    ctx->setSpindle(false);
}

void CycleState::onSwitchModeChanged(bool autoMode) {
    ctx->stateAutoCycleManual = autoMode;
    if (!autoMode) {
        ctx->transitionTo(new ManualState());
    }
}

void CycleState::onTopSliderReached(bool reached) {
    ctx->stateTopSlider = reached;

    if (reached && ctx->stateEndCycle) {
        // Завершаем цикл
        ctx->moveUp(false);
        ctx->releaseBrake(false);
        ctx->stateEndCycle = false;
        ctx->stateStartCycle = false;
        ctx->transitionTo(new StartedState());
    }
}

void CycleState::onEndSwitchTopTriggered() {
    movingDown = true;  // Меняем направление
    ctx->moveDown(true);
    ctx->moveUp(false);
}

void CycleState::onEndSwitchBottomTriggered() {
    movingDown = false;  // Меняем направление
    ctx->moveUp(true);
    ctx->moveDown(false);
}