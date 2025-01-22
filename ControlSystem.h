#pragma once

#include <Arduino.h>

#include "config.h"
#include "IOPorts.h"
#include "StatesActuators.h"
#include "VariablesProject.h"
#include "MemoryEeprom.h"
#include "Encoder.h"
#include "Display.h"
#include "TextMenu.h"

void handleButtonStates();
void handleMotorStates();
void handleStartFeed();
void handleProgramSwitch();
void setElectromagnetState(bool top, bool bottom);
void handleAutoCycleManual();
void handleAutoCycle();
void handleManualMode();
void handleStop();
void handleCycle();
void handleGeneralStop();