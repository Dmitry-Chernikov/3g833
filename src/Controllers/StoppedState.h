#pragma once
#include "MachineStates.h"

/* Состояние: Остановлен (stateGeneralStop = true) */
class StoppedState : public MachineStates {
private:
	unsigned long lastMenuTime;

public:
	void onEnter() override;
	void onExit() override;
	void onUpdate() override;

	void onButtonGeneralStopPressed() override {}
	void onButtonGeneralStopReleased() override;
	void onButtonStartFeedPressed() override;
	void onButtonStartFeedReleased() override {}
	void onButtonEndCyclePressed() override {}
	void onButtonPushPressed() override {}
	void onButtonSpindleStartPressed() override {}
	void onButtonSpindleStopPressed() override {}
	void onSwitchModeChanged(bool autoMode) override {}
	void onTopSliderReached(bool reached) override;
	void onEndSwitchTopTriggered() override {}
	void onEndSwitchBottomTriggered() override {}

	const char* getName() override { return "STOPPED"; }
};