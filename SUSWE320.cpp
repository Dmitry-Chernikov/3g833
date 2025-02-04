#include "SUSWE320.h"

// Процедура добавления параметра в группу
void ParameterGroup::addParametr(const Parameter &param) {
  if (parameterCount < maxParameters) {
    parameters[parameterCount++] = param;
  }else{
    Serial.println("Достигнуто максимальное количество параметров!");
  }
}

// Функция для получения мощности модели
float ParametersSUSWE320::getPower(Model model) { 
  return modelPowers[static_cast<int>(model)]; 
}

// Функция возвращает указатель на информацию об ошибке по её коду
const FaultInfo* ParametersSUSWE320::getFaultInfo(int code) {
    // Проверяем, что код находится в допустимом диапазоне
    if (0 <= code - 1 && code - 1 <= faultCount) {
        // Возвращаем указатель на элемент массива faultTable, соответствующий коду ошибки
        return &faultTable[code - 1];
    } else {
        // Если код не найден, возвращаем nullptr
        return nullptr; 
    }    
}

// Конструктор ParametersSUSWE320
ParametersSUSWE320::ParametersSUSWE320(Model model)
    : model(model), allParameters{ParameterGroup("F0 - Основные рабочие параметры", 21),
                                  ParameterGroup("F1 - Параметры управления V/F", 15),
                                  ParameterGroup("F2 - Параметры управления вектором", 34),
                                  ParameterGroup("F3 - Вспомогательные рабочие параметры", 9),
                                  ParameterGroup("F4 - Вспомогательные рабочие параметры 2", 9),
                                  ParameterGroup("F5 - Параметры цифрового ввода/вывода", 21),
                                  ParameterGroup("F6 - Функции аналогового входа и выхода", 15),
                                  ParameterGroup("F7 - Параметры выполнения программы (ПЛК)", 26),
                                  ParameterGroup("F8 - Параметры ПИД", 56),
                                  ParameterGroup("F9 - Параметры двигателя", 12),
                                  ParameterGroup("FA - Параметры защиты", 27),
                                  ParameterGroup("Fb - Отображение и специальные параметры", 23),
                                  ParameterGroup("FC - Параметры связи", 6),
                                  ParameterGroup("FP - Заводские параметры", 1),
                                  ParameterGroup("d - Параметры мониторинга", 20)} {
  // Добавляем параметры в группу F0
  Parameter param1;
  param1.name = "F0.00";
  param1.factoryDefault.floatValue = getPower(model); // Устанавливаем значение по умолчанию как 5.5 кВт
  param1.unit = "кВт";
  param1.minSetting.floatValue = 0.0;
  param1.maxSetting.floatValue = 99.9;
  param1.description = "Текущая мощность привода AC";
  allParameters[GROUP_F0].addParametr(param1);

  Parameter param2;
  param2.name = "F0.01";
  param2.factoryDefault.intValue = 0; // Устанавливаем значение по умолчанию как 0
  param2.unit = "";
  param2.minSetting.intValue = 0;
  param2.maxSetting.intValue = 1;
  param2.description = "0: Управление V/F, 1: Открытый вектор";
  allParameters[GROUP_F0].addParametr(param2);

  Parameter param3;
  param3.name = "F0.02";
  param3.factoryDefault.intValue = 0; // Устанавливаем значение по умолчанию как 0
  param3.unit = "";
  param3.minSetting.intValue = 0;
  param3.maxSetting.intValue = 2;
  param3.description = "0: Команда с панели, 1: Команда с терминала, 2: Команда по связи";
  allParameters[GROUP_F0].addParametr(param3);

  Parameter param4;
  param4.name = "F0.03";
  param4.factoryDefault.intValue = 4; // Устанавливаем значение по умолчанию как 4
  param4.unit = "";
  param4.minSetting.intValue = 0;
  param4.maxSetting.intValue = 8;
  param4.description = "0: Цифровая установка, 1: Цифровая установка с памятью, 2: AI1, 3: AI2, 4: AI3, 5: Мультискоростная команда, 6: Простой ПЛК, 7: PID, 8: Связь";
  allParameters[GROUP_F0].addParametr(param4);

  // Добавляем параметры в группу F1
  Parameter param5;
  param5.name = "F1.00";
  param5.factoryDefault.intValue = 0; // Устанавливаем значение по умолчанию как 0
  param5.unit = "";
  param5.minSetting.intValue = 0;
  param5.maxSetting.intValue = 4;
  param5.description = "0: Линейная кривая, 1: Квадратная кривая, 2: Кривая 1.5 мощности, 3: Кривая 1.2 мощности, 4: Многоточечная VF кривая";
  allParameters[GROUP_F1].addParametr(param5);

  Parameter param6;
  param6.name = "F1.01";
  param6.factoryDefault.floatValue = 3.0; // Устанавливаем значение по умолчанию как 3.0
  param6.unit = "%";
  param6.minSetting.floatValue = 0.0;
  param6.maxSetting.floatValue = 30.0;
  param6.description = "Ручное увеличение момента, это значение установлено в процентах относительно номинального напряжения двигателя.";
  allParameters[GROUP_F1].addParametr(param6);

  Parameter param7;
  param7.name = "F1.02";
  param7.factoryDefault.floatValue = 15.0; // Устанавливаем значение по умолчанию как 15.0
  param7.unit = "Hz";
  param7.minSetting.floatValue = 0.0;
  param7.maxSetting.floatValue = 50.0;
  param7.description = "Частота отсечения для ручного увеличения момента";
  allParameters[GROUP_F1].addParametr(param7);
}