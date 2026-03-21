#pragma once

#include "config.h"
#include <Arduino.h>

/*INPUT Входные сигналы*/
#ifdef ENABLE_KEYPAD
	#define interruptRemote			12  /*1 Вывод прерывания PCINT6 пульта управления */
#endif
	#define buttonEndCycle			22  /*2 Вывод кнопки Конец Цикла */
	#define buttonStartFeed			23  /*3 Вывод кнопки Подача-Пуск, подача электроэнергии на двигатели возвратно-поступательного движения */
	#define buttonSpindleStart		24  /*4 Вывод кнопки Старт Шпиндель */
	#define buttonSpindleStop		25  /*5 Вывод кнопки Шпиндель Стоп */
	#define buttonPush				26  /*6 Вывод кнопки Толчковое опускание хонинговальной головки */
	#define buttonGeneralStop		27  /*7 Вывод кнопки Общий Стоп, отключение питания на все механизмы станка */
	#define switchAutoCycleManual	28  /*8 Вывод переключателя автоматического цикла или ручного управления */
	#define switchTopSlider			29  /*9 Вывод концевика верхнего положения ползуна */
	#define endSwitchTop			30	/*10 Вывод концевика верхнего концевика цикла */
	#define endSwitchBottom			31  /*11 Вывод концевика нижнего концевика цикла */

/*OUTPUT Выходные сигналы*/
	#define electromagnetTop		32  /*1 Вывод электро-муфты ЭТМ0921Н перемещения вверх ползуна */
	#define electromagnetBottom		33  /*2 Вывод электро-муфты ЭТМ0921Н перемещения вниз ползуна */
	#define electromagnetManual		34  /*3 Вывод электро-муфты ЭТМ0721Н ручное управления ползуна */
	#define electromagnetBrake		35  /*4 Вывод электромагнита растормаживания МИС5100М перемещения ползуна */
	#define motorSpindle			36  /*5 Вывод включения мотора вращения шпинделя */
	#define motorStartFeed			37  /*6 Вывод включения мотора возвратно-поступательного движения ползун */
	#define motorSelfCoolant		38  /*7 Вывод включения смазочно-охлаждающей жидкости */
	#define rs485TransceiverReceive 4	/*8 Вывод разрешения работы передачи и приёмника */

/*Инициализация входов и подтягивание входов к положительному потенциалу с помощью внутренних резисторов*/
extern void initSetupInputManipulation();

/*Инициализация выходов и устанавливаем высокое состояние на выходе*/
extern void initSetupOutputExecutiveMechanism();