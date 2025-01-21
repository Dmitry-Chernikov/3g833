#pragma once

#include <Arduino.h> // или другой нужный заголовок
#include "config.h"
#include "IOPorts.h"
#include "StatesActuators.h"
#include "AllEnumProject.h"
#include "Encoder.h"
#include "Display.h"

bool trigerRS(bool currentState, uint8_t TrigSet, uint8_t TrigReset);
// Объявление функций
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