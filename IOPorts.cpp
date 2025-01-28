#include "IOPorts.h"

/////////////Инициализация входов и подтягивание входов к положительному
///потенциалу с помощью внутренних резисторов/////////////
void initSetupInputManipulation() {
  pinMode(buttonEndCycle, INPUT_PULLUP);
  pinMode(buttonStartFeed, INPUT_PULLUP);
  pinMode(buttonSpindleStart, INPUT_PULLUP);
  pinMode(buttonSpindleStop, INPUT_PULLUP);
  pinMode(buttonPush, INPUT_PULLUP);
  pinMode(buttonGeneralStop, INPUT_PULLUP);
  pinMode(switchAutoCycleManual, INPUT_PULLUP);
  pinMode(switchTopSlider, INPUT_PULLUP);
  pinMode(endSwitchTop, INPUT_PULLUP);
  pinMode(endSwitchBottom, INPUT_PULLUP);
}

/////////////Инициализация выходов и устанавливаем высокое состояние на
///выходе/////////////
void initSetupOutpuExecutiveMechanism() {
  pinMode(electromagnetTop, OUTPUT);
  digitalWrite(electromagnetTop, true);

  pinMode(electromagnetBottom, OUTPUT);
  digitalWrite(electromagnetBottom, true);

  pinMode(electromagnetManual, OUTPUT);
  digitalWrite(electromagnetManual, true);

  pinMode(electromagnetBrake, OUTPUT);
  digitalWrite(electromagnetBrake, true);

  pinMode(motorSpindle, OUTPUT);
  digitalWrite(motorSpindle, true);

  pinMode(motorStartFeed, OUTPUT);
  digitalWrite(motorStartFeed, true);

  pinMode(motorSelfCoolant, OUTPUT);
  digitalWrite(motorSelfCoolant, true);
}
