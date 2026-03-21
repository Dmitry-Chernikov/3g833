#pragma once

#include "MachineStates.h"

// Состояние: Автоматический цикл
class CycleState : public MachineStates {
private:
	bool movingDown;
	unsigned long lastMoveTime;

public:
	void onEnter() override;
	void onExit() override;
	void onUpdate() override;

	void onButtonGeneralStopPressed() override;
	void onButtonGeneralStopReleased() override {}
	void onButtonStartFeedPressed() override {}
	void onButtonStartFeedReleased() override {}
	void onButtonEndCyclePressed() override;
	void onButtonPushPressed() override {}
	void onButtonSpindleStartPressed() override;
	void onButtonSpindleStopPressed() override;
	void onSwitchModeChanged(bool autoMode) override;
	void onTopSliderReached(bool reached) override;
	void onEndSwitchTopTriggered() override;
	void onEndSwitchBottomTriggered() override;

	const char* getName() override { return "CYCLE"; }
};