#include "SUSWE320.h"

void ParameterGroup::addParametr(const Parameter& param){
  if(parameterCount < MAX_PARAMETERS){
    parameters[parameterCount++] = param; 
  }
}

// Конструктор ParametersSUSWE320
ParametersSUSWE320::ParametersSUSWE320() 
    : allParameters{
        ParameterGroup("F0 - Основные рабочие параметры"),
        ParameterGroup("F1 - Параметры управления V/F"),
        ParameterGroup("F2 - Параметры управления вектором"),
        ParameterGroup("F3 - Вспомогательные рабочие параметры"),
        ParameterGroup("F4 - Вспомогательные рабочие параметры 2"),
        ParameterGroup("F5 - Параметры цифрового ввода/вывода"),
        ParameterGroup("F6 - Функции аналогового входа и выхода"),
        ParameterGroup("F7 - Параметры выполнения программы (ПЛК)"),
        ParameterGroup("F8 - Параметры ПИД"),
        ParameterGroup("F9 - Параметры двигателя"),
        ParameterGroup("FA - Параметры защиты"),
        ParameterGroup("Fb - Отображение и специальные параметры"),
        ParameterGroup("FC - Параметры связи"),
        ParameterGroup("FP - Заводские параметры"),
        ParameterGroup("d - Параметры мониторинга")
    } 
  {
    // Добавляем параметры в группу F0
    Parameter param1;
    param1.name = "F0.00";
    param1.factoryDefault.floatValue = POWER_5_5KW; // Устанавливаем значение по умолчанию как 5.5 кВт
    param1.unit = "кВт";
    param1.minSetting = 0.0;
    param1.maxSetting = 99.9;
    param1.description = "Текущая мощность привода AC";
    allParameters[GROUP_F0].addParametr(param1);

    Parameter param2;
    param2.name = "F0.01";
    param2.factoryDefault.intValue = 0; // Устанавливаем значение по умолчанию как 0
    param2.unit = "";
    param2.minSetting = 0;
    param2.maxSetting = 1;
    param2.description = "0: Управление V/F, 1: Открытый вектор";
    allParameters[GROUP_F0].addParametr(param2);

    Parameter param3;
    param3.name = "F0.02";
    param3.factoryDefault.intValue = 0; // Устанавливаем значение по умолчанию как 0
    param3.unit = "";
    param3.minSetting = 0;
    param3.maxSetting = 2;
    param3.description = "0: Команда с панели, 1: Команда с терминала, 2: Команда по связи";
    allParameters[GROUP_F0].addParametr(param3);

    Parameter param4;
    param4.name = "F0.03";
    param4.factoryDefault.intValue = 4; // Устанавливаем значение по умолчанию как 4
    param4.unit = "";
    param4.minSetting = 0;
    param4.maxSetting = 8;
    param4.description = "0: Цифровая установка, 1: Цифровая установка с памятью, 2: AI1, 3: AI2, 4: AI3, 5: Мультискоростная команда, 6: Простой ПЛК, 7: PID, 8: Связь";
    allParameters[GROUP_F0].addParametr(param4);

    // Добавляем параметры в группу F1
    Parameter param5;
    param5.name = "F1.00";
    param5.factoryDefault.intValue = 0; // Устанавливаем значение по умолчанию как 0
    param5.unit = "";
    param5.minSetting = 0;
    param5.maxSetting = 4;
    param5.description = "0: Линейная кривая, 1: Квадратная кривая, 2: Кривая 1.5 мощности, 3: Кривая 1.2 мощности, 4: Многоточечная VF кривая";
    allParameters[GROUP_F1].addParametr(param5);

    Parameter param6;
    param6.name = "F1.01";
    param6.factoryDefault.floatValue = 3.0; // Устанавливаем значение по умолчанию как 3.0
    param6.unit = "%";
    param6.minSetting = 0.0;
    param6.maxSetting = 30.0;
    param6.description = "Ручное увеличение момента, это значение установлено в процентах относительно номинального напряжения двигателя.";
    allParameters[GROUP_F1].addParametr(param6);

    Parameter param7;
    param7.name = "F1.02";
    param7.factoryDefault.floatValue = 15.0; // Устанавливаем значение по умолчанию как 15.0
    param7.unit = "Hz";
    param7.minSetting = 0.0;
    param7.maxSetting = 50.0;
    param7.description = "Частота отсечения для ручного увеличения момента";
    allParameters[GROUP_F1].addParametr(param7);
  }