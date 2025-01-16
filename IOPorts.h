#pragma once

/*INPUT*/ //Входные сигналы
#ifdef ENABLE_KYPAD
/*1*/#define interruptRemote       12  // Вывод прерывания PCINT6 пульта управления
#endif
/*2*/#define buttonEndCycle        22  // Вывод кнопки Конец Цикла
/*3*/#define buttonStartFeed       23  // Вывод кнопки Подача-Пуск, подача электроэнергии на двигатели возвратно поступательного движения
/*4*/#define buttonSpindleStart    24  // Вывод кнопки Старт Шпиндель
/*5*/#define buttonSpindleStop     25  // Вывод кнопки Шпиндель Стоп
/*6*/#define buttonPush            26  // Вывод кнопки Толчковое опускание хонинговальной головки
/*7*/#define buttonGeneralStop     27  // Вывод кнопки Общий Стоп, отключение питания на все механизмы станка
/*8*/#define switchAutoCycleManual 28  // Вывод переключателя автоматического цикла или ручного управления
/*9*/#define switchTopSlider       29  // Вывод концевика верхнего положения ползуна
/*10*/#define endSwitchTop         30  // Вывод концевика верхнего концевика цикла
/*11*/#define endSwitchBottom      31  // Вывод концевика нижнего концевика цикла

/*PUTPUT*/ //Выходные сигналы
/*1*/#define electromagnetTop         32  // Вывод электро-муфты ЭТМ0921Н перемещения вверх ползуна
/*2*/#define electromagnetBottom      33  // Вывод электро-муфты ЭТМ0921Н перемещения вниз ползуна
/*3*/#define electromagnetManual      34  // Вывод электро-муфты ЭТМ0721Н ручное управления ползуна
/*4*/#define electromagnetBrake       35  // Вывод электромагнита растормаживания МИС5100М перемещения ползуна
/*5*/#define motorSpindle             36  // Вывод включения мотора вращения шпинделя
/*6*/#define motorStartFeed           37  // Вывод включения мотора возвратно поступательного движения ползун
/*7*/#define motorSelfCoolant         38  // Вывод включения смазочно охлаждающей жидкости
/*8*/#define rs485TransceivReceive    4   // Вывод разрешения работы предачи и приёмника

/////////////Инициализация входов и подтягивание входов к положительному потенциалу с помощью внутренних резисторов/////////////
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

/////////////Инициализация выходов и устанавливаем высокое состояние на выходе/////////////
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