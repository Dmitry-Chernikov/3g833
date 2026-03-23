#pragma once
#include "MachineStates.h"

/* Состояние: Остановлен (stateGeneralStop = true) */
class StoppedState : public MachineStates {
private:
	unsigned long lastMenuTime;

public:
	void onEnter() override;	///< Вхождение во внутреннее состояние
	void onExit() override;		///< Выход из текущего состояние и переход в новое.
	void onUpdate() override;	///< Периодическое обновление состояния

	void onButtonGeneralStopPressed() override;
	void onButtonGeneralStopReleased() override;

	void onButtonStartFeedPressed() override;
	void onButtonStartFeedReleased() override {}

	void onButtonEndCyclePressed() override {}
	void onButtonEndCycleReleased() override {}

	void onButtonPushPressed() override {}
	void onButtonPushReleased() override {}

	void onButtonSpindleStartPressed() override {}
	void onButtonSpindleStopPressed() override {}

	void onSwitchModeChanged(bool autoMode) override {}

	void onTopSliderReached(bool reached) override;

	void onEndSwitchTopTriggered() override {}
	void onEndSwitchBottomTriggered() override {}

	const char* getName() override { return "STOPPED"; }
};