#pragma once

#include "MachineContext.h"

// Базовый класс состояния
class MachineStates {
protected:
    MachineContext* ctx;

public:
    MachineStates() : ctx(nullptr) {} /* Конструктор по умолчанию для создания пустого объекта с нулевым контекстом */
    virtual ~MachineStates() {} /* Дестркутор */


    void setContext(MachineContext* context) { /* Присваиваем контекст переданного в метод состояния */
	    ctx = context;
    }

    /* Обработчики События */
    virtual void onEnter() = 0;   	/* Вхождение во внутреннее состояние */
    virtual void onExit() = 0;		/* Выход из текущего состояние и переход в новое. */
    virtual void onUpdate() = 0;    /* Периодическое обновление состояния */

    virtual void onButtonGeneralStopPressed() = 0;			/* Нажатие кнопки Общий Стоп */
    virtual void onButtonGeneralStopReleased() = 0;			/* Отжатие кнопки Общий Стоп */

    virtual void onButtonStartFeedPressed() = 0;			/* Нажата кнопки Подача-Пуск */
    virtual void onButtonStartFeedReleased() = 0;			/* Отжатие кнопки Подача-Пуск */

	virtual void onButtonEndCyclePressed() = 0;				/* Нажатие кнопки Конец Цикла */
	virtual void onButtonEndCycleReleased() = 0;			/* Отжатие кнопки Конец Цикла */

    virtual void onButtonPushPressed() = 0;					/* Нажатие кнопки Толчковая */
	virtual void onButtonPushReleased() = 0;				/* Отжатие кнопки Толчковая */

    virtual void onButtonSpindleStartPressed() = 0;			/* Нажали кнопку Старт Шпиндель */
    virtual void onButtonSpindleStopPressed() = 0;			/* Отжатие кнопки Старт Шпиндель */

    virtual void onSwitchModeChanged(bool autoMode) = 0;	/* Изменение состояния переключателя режима работы управления, (true - Автоматически Цикл, false - Ручной Ввод Хоны) */

    virtual void onTopSliderReached(bool reached) = 0;		/* Включение концевика верхнего положения ползуна (true - достигли, false - отошёл) */

    virtual void onEndSwitchTopTriggered() = 0;				/* Путевой переключатель перешёл в состояния верхнего положения ползуна */
    virtual void onEndSwitchBottomTriggered() = 0;			/* Путевой переключатель перешёл в состояния нижнего положения ползуна */

    virtual const char* getName() = 0;						/* Получение имени класса состояния */
};





