#include <Arduino.h>

/** @todo Необходимо добавить в проект функцию для настройки векторного режима и управления скоростью по Modbus HS321:
 * 1 для двигателей вращательного движения
 * 2 для двигателей возвратно поступательного движения
 *
 * Основные шаги настройки векторного режима:
 * 1. Выбор режима управления (F0.01)
 * - Установите F0.01 = 1 для выбора векторного разомкнутого цикла (Sensorless Vector Control).
 * - Установи F0.02 = 2 (Управление по связи, управление по Modbus).
 * - Установи F0.03 = 8 (Источник частоты — связь по Modbus). Теперь задавать частоту будете через Modbus-регистр 1000H.
 * 2000H, Запись, Команды управления, 1–7 (см. таблицу ниже)
 *   0001 = Пуск вперёд
 *   0002 = Пуск в обратном направлении
 *   0003 = Толчок вперёд
 *   0004 = Толчок назад
 *   0005 = Свободный останов (coast stop)
 *   0006 = Замедленный останов (ramp stop)
 *   0007 = Сброс ошибки
 * 1000H, Запись, Задание частоты, -10000…+10000 = -100.00%…+100.00%
 * 3000H, Чтение, Состояние привода, 1–3 (1=FWD (Forward running), 2=REV (Revers running), 3=STOP)
 * 8000H, Чтение, Код ошибки, 0=нет ошибки, другие — код ошибки
 * Формат данных:
 *   Частота: 5000 = 50.00 Гц (делитель 100)
 *   Проценты: 10000 = 100.00%
 *   Ток: 70 = 7.0 А (делитель 10)
 *
 * 2. Настройка параметров двигателя (Группа F9)
 *   Вал двигателя освобождаем от нагрузки для холостого режима.
 * - Установка F9.00 = 0.0 кВт, Номинальная мощность двигателя.
 * - Установка F9.01 = 380 В, Номинальное напряжение двигателя.
 * - Установка F9.02 = 0 А , Номинальный ток двигателя.
 * - Установка F9.03 = 1500 об/мин, Номинальная скорость вращения двигателя.
 * - Установка F9.04 = 50.00 Гц, Частота вращения двигателя.
 * - Установка F9.05 = 1 , автоматическая статическая идентификации дополнительных параметров двигателя.
 *   Нажми кнопку RUN
 *   Жди 10–30 секунд — привод измерит R, L, ток холостого хода.
 *   После завершения F9.05 автоматически сбросится в 0.
 *   Частотный преобразователь запускается автоматически в холостом режиме.
 *
 *   3. Настройка регуляторов скорости (F2.00 – F2.05) ПИ-регуляторы скорости для низких и высоких частот
 * - F2.00 Коэффициент П (Kp) низкой скорости 1-100 (20-40), ст-25 Быстрота реакции на ошибку скорости
 * - F2.01 Коэффициент И (Ki) низкой скорости 1.00-10.00 (0.5-2.0), ст-1.00 Устранение остаточной ошибки
 * - F2.02 Коэффициент П (Kp) высокой скорости 1-100 (10-30), cn-15 Для высоких оборотов поменьше Kp
 * - F2.03 Коэффициент И (Ki) высокой скорости 1.00-10.00 (1.0-3.0), ст-0.50 Меньше интеграла на высоких оборотах
 * - F2.04 Частота переключения на низкоскоростной регулятор 5-30 Гц (10 Гц), ст-10.00 Гц Переключение между регуляторами
 * - F2.05 Частота переключения на высокоскоростной регулятор 20-100 Гц (30.00 Гц), ст-30.00 Гц Всё выше 30 Гц — «высокоскоростной» регулятор
 *
 *   4. Настройка ограничения момента (F2.19 – F2.20)
 * - F2.19: Ограничение момента при регулировании скорости (150% по умолчанию).
 * - F2.20: Максимальный момент в зоне ослабления поля (100% по умолчанию).
 *
 *   5. Настройка компенсации скольжения (F2.14, F2.28)
 * - F2.14: Коэффициент компенсации скольжения (100% по умолчанию).
 * - F2.28: Коэффициент компенсации наблюдения за потоком (100% по умолчанию).
 *
 *   6. Настройка фильтров и времени (F2.23, F2.29)
 * - F2.23: Постоянная времени фильтра контура скорости (25 мс по умолчанию).
 * - F2.29: Коэффициент фильтра наблюдения за потоком (300 по умолчанию).
**/

/**
 * @brief Процедура настройки частотного преобразователя HS321 в векторном режиме с автоматической адаптацией под двигатель
 *
 * Выполняет, полную настройку векторного управления по Modbus RTU:
 * - Выбор режима Sensorless Vector Control
 * - Настройка источника задания и команд
 * - Параметризация двигателя (F9.00–F9.04)
 * - Запуск автоидентификации параметров двигателя (F9.05 = 1)
 * - Настройка ПИ-регуляторов скорости (низко/высокоскоростные зоны)
 * - Ограничение момента, компенсация скольжения, фильтры
 *
 * @param driver Объект драйвера HS321 (предполагается наличие методов writeSingleParameter, readSingleParameter)
 * @return true, если настройка прошла успешно; false — при ошибке записи/чтения или таймауте автоадаптации
 */
/*bool configureVectorModeWithAutoTuning(HS321& driver) {
    // === 1. Режим управления и интерфейс ===
    if (!driver.writeSingleParameter(0x0F01, 1)) return false;  // F0.01 = 1: Sensorless Vector Control
    if (!driver.writeSingleParameter(0x0F02, 2)) return false;  // F0.02 = 2: Управление по Modbus
    if (!driver.writeSingleParameter(0x0F03, 8)) return false;  // F0.03 = 8: Источник частоты — Modbus (регистр 1000H)

    // === 2. Параметры двигателя (пример для стандартного АД 380В, 50Гц, 1500 об/мин) ===
    if (!driver.writeSingleParameter(0x0F90, 0))     return false;   // F9.00 = 0.0 кВт (автоопределение)
    if (!driver.writeSingleParameter(0x0F91, 380))   return false;   // F9.01 = 380 В
    if (!driver.writeSingleParameter(0x0F92, 0))     return false;   // F9.02 = 0 А (автоопределение тока)
    if (!driver.writeSingleParameter(0x0F93, 1500))  return false;   // F9.03 = 1500 об/мин
    if (!driver.writeSingleParameter(0x0F94, 5000))  return false;   // F9.04 = 50.00 Гц (в сотых долях)

    // === 3. Включение автоидентификации (статическая настройка) ===
    if (!driver.writeSingleParameter(0x0F95, 1)) return false;  // F9.05 = 1: запуск автонастройки

    // Ожидание завершения автоадаптации (F9.05 должен сброситься в 0)
    uint32_t timeout = 40000;  // 40 секунд максимум
    uint32_t start = millis();
    uint16_t f905_value;
    while (timeout > (millis() - start)) {
        if (driver.readSingleParameter(0x0F95, &f905_value) && f905_value == 0) {
            break;  // Успешно завершено
        }
        delay(100);
    }
    if (timeout <= (millis() - start)) {
        return false;  // Таймаут автоадаптации
    }

    // === 4. Настройка ПИ-регуляторов скорости ===
    if (!driver.writeSingleParameter(0x0F20, 25))    return false;   // F2.00: Kp низкой скорости = 25
    if (!driver.writeSingleParameter(0x0F21, 100))   return false;   // F2.01: Ki низкой скорости = 1.00 (в сотых)
    if (!driver.writeSingleParameter(0x0F22, 15))    return false;   // F2.02: Kp высокой скорости = 15
    if (!driver.writeSingleParameter(0x0F23, 50))    return false;   // F2.03: Ki высокой скорости = 0.50
    if (!driver.writeSingleParameter(0x0F24, 1000))  return false;   // F2.04: Переключение на низкую зону = 10.00 Гц
    if (!driver.writeSingleParameter(0x0F25, 3000))  return false;   // F2.05: Переключение на высокую зону = 30.00 Гц

    // === 5. Ограничение момента ===
    if (!driver.writeSingleParameter(0x0F219, 150)) return false;   // F2.19: Ограничение момента = 150%
    if (!driver.writeSingleParameter(0x0F220, 100)) return false;   // F2.20: Макс. момент в ослаблении поля = 100%

    // === 6. Компенсация и фильтры ===
    if (!driver.writeSingleParameter(0x0F214, 100)) return false;   // F2.14: Компенсация скольжения = 100%
    if (!driver.writeSingleParameter(0x0F228, 100)) return false;   // F2.28: Компенсация потока = 100%
    if (!driver.writeSingleParameter(0x0F223, 25))  return false;   // F2.23: Фильтр скорости = 25 мс
    if (!driver.writeSingleParameter(0x0F229, 300)) return false;   // F2.29: Фильтр наблюдения = 300

    return true;  // Все настройки выполнены успешно
}*/

#include "TechnicalSpecifications3G833.h"
using namespace TechnicalSpecifications3G833;
#include "config.h" //Определены define для вкл/выкл кода в компиляцию

#include "ControlSystem.h"    // Основной алгоритм работы станка содержит процедуры используемые в loop
#include "Display.h"          // Работа с дисплеем Adafruit RGB LCD Shield
#include "Encoder.h"          // Объявление объекта типа AS5048A для работы с энкодером AS5048A
#include "IOPorts.h"
#include "MemoryEeprom.h"     // Описывает структуру данных которая сохраняется в память и процедуры для работы с памятью

#include "StatesActuators.h"  // Описаны переменные которые хранят состояния режимов работы станка и исполнительных механизмов

#include "TextMenu.h"         // Создаётся текстовое меню на базе LiquidMenu которая использует дисплей Adafruit RGB LCD Shield

#include <HS321.h>
//#include <ModbusMaster.h>

// #include <avr/pgmspace.h>
// #include <util/delay.h>

// char input_saved[3];
// char output_saved[3];
// char string_saved[] = " *";
// char string_notSaved[] = "  ";

#ifdef ENABLE_KEYPAD
void pciSetup(const byte pin) {
	//*Pin Change Interrupt Прерывание по изменению вывода*//
	// PCICR Регистр управления PCINT прерываниями, имеются три группы PCI0..2 в
	// которые входят выводы по 8 штук PCIFR Регистр флагов сработавших прерываний
	// PCINT, показывает в какой группе сработало прерывание от вывода PCMSKx
	// Регистр маскировки прерываний каждого из портов для групп PCI0..2,
	// активирует прерывание по выводу. 1 Задать обработчик для соответствующего
	// прерывания PCINT, используя макрос ISR. 2 Разрешить генерацию прерываний
	// интересующим выводом микроконтроллера (регистр группы PCMSKx). 3 Разрешить
	// обработку прерывания PCINT, которое генерирует интересующий вывод (регистр
	// PCICR). 4 Установить бит I, разрешающий обработку прерываний глобально
	// (регистр SREG).
	// PCMSK0 |= 1 << 6;
	*digitalPinToPCMSK(pin) |= bit(digitalPinToPCMSKbit(pin)); // Разрешаем PCINT для указанного пина
	// PCIFR |= 0 << 0
	PCIFR |= bit(digitalPinToPCICRbit(pin)); // Очищаем признак запроса прерывания
	// для соответствующей группы пинов
	// PCICR |= 1 << 0;
	PCICR |= bit(digitalPinToPCICRbit(pin)); // Разрешаем PCINT для соответствующей группы пинов
	SREG |= 1 << SREG_I; // бит 7 Разрешить прерывания микроконтроллера
}

void readKeypad() {
	handleButtonStates();
	handleMotorStates();
}
#endif

/*****hs321*****/
HS321 hs321(0x0001, Serial1, &Serial, 9600, rs485TransceiverReceive);
uint16_t responseData;
/*****hs321*****/


void setup() {
	cli();

#ifdef ENABLE_KEYPAD
	pinMode(interruptRemote, INPUT_PULLUP); // Подтянем пины источники PCINT к питанию
	pciSetup(interruptRemote); // И разрешим на них прерывания T6
#endif

	initSetupInputManipulation();

	initSetupOutputExecutiveMechanism();

	hs321.begin();


#ifdef ENABLE_KEYPAD
	readKeypad();
#endif

	/////////////Инициализация энкодера/////////////
	initEncoder();

	initDisplay();

	clearMemory();

	initMemory();

	settingTextMenu();

	// strncpy(input_saved, string_saved, sizeof(string_saved));
	// strncpy(output_saved, string_saved, sizeof(string_saved));

	sei();
}

void loop() {
	constexpr uint16_t value[2] = {1, 2};
	/*****SUSWE321*****/
	if (hs321.writeParametersInGroups(GROUP_F0, 1, value, 2)) {
		lcdPrintString(_lcd, "Frequency", "OK", "", YELLOW, NOT_CHANGE_COLOR, 0, 0, 0, true, false);
	}
	/*****SUSWE321*****/


	handleButtonStates();
	handleMotorStates();

	/////////////////////////////////////////////////////ЛОГИКА СОСТОЯНИЯ///////////////////////////////////////////////////////
	if (stateStartFeed) {
		// Кнопку подача-пуск нажали. Запускаем мотор возвратно-поступательного движения
		handleStartFeed();

		handleProgramSwitch();

		handleAutoCycle();

		handleManualMode();

#ifdef ENABLE_PROGRAM_SWITCH
		if (!_data.stateIntermediate && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
			lcdPrintString(_lcd, "IN FIELD ACTION", String(_data.linearMove, 2), "mm", YELLOW, NOT_CHANGE_COLOR, 0, 0,
			               0, true, false);
		}

		if (_data.stateIntermediate && !_data.stateElectromagnetBottom && stateMillisDelay(
			    &previousMillisMenu, &updateMenu)) {
			lcdPrintString(_lcd, "LIMIT TOP PROG", String(_data.linearMove, 2), "mm", WHITE, NOT_CHANGE_COLOR, 0, 0, 0,
			               true, false);
		}

		if (_data.stateIntermediate && !_data.stateElectromagnetTop && stateMillisDelay(
			    &previousMillisMenu, &updateMenu)) {
			lcdPrintString(_lcd, "LIMIT BOTTOM PROG", String(_data.linearMove, 2), "mm", WHITE, NOT_CHANGE_COLOR, 0, 0,
			               0, true, false);
		}
#endif

#ifdef ENABLE_SWITCH
		if (!digitalRead(endSwitchTop) && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
			lcdPrintString(_lcd, "LIMIT TOP MECHAN", String(_data.linearMove, 2), "mm", YELLOW, NOT_CHANGE_COLOR, 0, 0,
			               0, true, false);
		}

		if (!digitalRead(endSwitchBottom) && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
			lcdPrintString(_lcd, "LIMIT BOTTOM MECHAN", String(_data.linearMove, 2), "mm", GREEN, NOT_CHANGE_COLOR, 0,
			               0, 0, true, false);
		}
#endif
	}

	if (!stateStartFeed) {
		// Кнопку Общий стоп нажали

		handleStop();

		/////////////////////////////////////////////////////EEPROM SAVE///////////////////////////////////////////////////////
		saveEeprom(_lcd, _dataBuffer, _data);

		/////////////////////////////////////////////////////LCD DISPLAY BUTTONS READ///////////////////////////////////////////////////////
		Menu();
	}

	/////////////////////////////////////////////////////ЦИКЛ///////////////////////////////////////////////////////
	while (stateStartCycle) {
		// Включён режим Цикл
		handleCycle();

#ifdef ENABLE_PROGRAM_SWITCH
		if (!_data.stateIntermediate && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
			lcdPrintString(_lcd, "IN FIELD ACTION", String(_data.linearMove, 2), "mm", GREEN, NOT_CHANGE_COLOR, 0, 0, 0,
			               true, false);
		}

		if (_data.stateIntermediate && !_data.stateElectromagnetBottom && stateMillisDelay(
			    &previousMillisMenu, &updateMenu)) {
			lcdPrintString(_lcd, "LIMIT TOP PROG", String(_data.linearMove, 2), "mm", YELLOW, NOT_CHANGE_COLOR, 0, 0, 0,
			               true, false);
		}

		if (_data.stateIntermediate && !_data.stateElectromagnetTop && stateMillisDelay(
			    &previousMillisMenu, &updateMenu)) {
			lcdPrintString(_lcd, "LIMIT BOTTOM PROG", String(_data.linearMove, 2), "mm", YELLOW, NOT_CHANGE_COLOR, 0, 0,
			               0, true, false);
		}
#endif

#ifdef ENABLE_SWITCH
		if (!digitalRead(endSwitchTop) && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
			lcdPrintString(_lcd, "LIMIT TOP MECHAN", String(_data.linearMove, 2), "mm", YELLOW, NOT_CHANGE_COLOR, 0, 0,
			               0, true, false);
		}

		if (!digitalRead(endSwitchBottom) && stateMillisDelay(&previousMillisMenu, &updateMenu)) {
			lcdPrintString(_lcd, "LIMIT BOTTOM MECHAN", String(_data.linearMove, 2), "mm", GREEN, NOT_CHANGE_COLOR, 0,
			               0, 0, true, false);
		}
#endif
	}
}

#if defined(ENABLE_KEYPAD)
ISR(PCINT0_vect) {
	// Обработчик запросов прерывания от пинов PCINT0..PCINT7

	cli(); // сбрасываем флаг прерывания (Запретить прерывания)
	readKeypad(); // вызов процедуры опроса клавиатуры
	sei(); // устанавливаем флаг прерывания (Разрешить прерывания)
}
#endif
