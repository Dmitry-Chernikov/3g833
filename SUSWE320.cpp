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
    if (0 <= code - 1 && code - 1 < faultCount) {
        // Возвращаем указатель на элемент массива faultTable, соответствующий коду ошибки
        return &faultTable[code - 1];
    } else {
        // Если код не найден, возвращаем nullptr
        return nullptr; 
    }    
}

Parameter ParametersSUSWE320::createParameter(const char* name, float defaultValue, const char* unit, float min, float max, const char* description) {
    Parameter param;
    param.name = name;
    param.factoryDefault.floatValue = defaultValue; // Инициализация floatValue
    param.unit = unit;
    param.minSetting.floatValue = min; // Инициализация minSetting
    param.maxSetting.floatValue = max; // Инициализация maxSetting
    param.description = description;
    param.type = ParameterType::FLOAT;
    return param;
}

Parameter ParametersSUSWE320::createParameter(const char* name, int defaultValue, const char* unit, int min, int max, const char* description) {
    Parameter param;
    param.name = name;
    param.factoryDefault.intValue = defaultValue; // Инициализация floatValue
    param.unit = unit;
    param.minSetting.intValue = min; // Инициализация minSetting
    param.maxSetting.intValue = max; // Инициализация maxSetting
    param.description = description;
    param.type = ParameterType::INT;
    return param;
}

Parameter ParametersSUSWE320::createParameter(const char* name, const char* defaultValue, const char* unit, const char* min, const char* max, const char* description) {
    Parameter param;
    param.name = name;
    param.factoryDefault.stringValue = defaultValue; // Инициализация floatValue
    param.unit = unit;
    param.minSetting.stringValue = min; // Инициализация minSetting
    param.maxSetting.stringValue = max; // Инициализация maxSetting
    param.description = description;
    param.type = ParameterType::STRING;
    return param;
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
  allParameters[GROUP_F0].addParametr(createParameter("F0.00", getPower(model), "кВт", 0.0f, 99.9f, "Текущая мощность привода AC"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.01", 0, " ", 0, 1, "Режим управления: 0 - V/F управление, 1 - Открытая петля векторного управления"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.02", 0, " ", 0, 2, "Выбор команды запуска: 0 - Команда с панели, 1 - Команда с терминала, 2 - Команда по связи"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.03", 4, " ", 0, 8, "Выбор источника основной частоты X: 0 - Цифровая установка, 1 - AI1, 2 - AI2, 3 - AI3, 4 - Многоскоростная команда, 5 - Простая ПЛК, 6 - PID, 7 - Связь"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.04", 0, " ", 0, 8, "Выбор вспомогательного источника частоты Y: 0 - Цифровая установка, 1 - AI1, 2 - AI2, 3 - AI3, 4 - Многоскоростная команда, 5 - Простая ПЛК, 6 - PID, 7 - Связь"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.05", 0, " ", 0, 3, "Расчет основной вспомогательной частоты: 0 - Основная + Вспомогательная, 1 - Основная - Вспомогательная, 2 - Макс. (основная, вспомогательная), 3 - Мин. (основная, вспомогательная)"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.06", 0, " ", 0, 4, "Выбор источника частоты: 0 - Основной источник частоты X, 1 - Основная и вспомогательная расчет, 2 - Переключение между основным и вспомогательным источником"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.07", 50.0f, "Гц", 0.0f, 400.0f, "Цифровая установка частоты: Установленное значение - начальное значение цифровой частоты"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.08", 50.0f, "Гц", 0.0f, 400.0f, "Максимальная выходная частота: Максимальная частота, разрешенная для выхода привода AC. Максимальная выходная частота — это наивысшая частота, разрешенная для выхода привода переменного тока, и она служит эталоном для настройки ускорения и замедления."));
  allParameters[GROUP_F0].addParametr(createParameter("F0.09", 50.0f, "Гц", 0.0f, 400.0f, "Частота верхнего предела: Нижний предел частоты - максимальная выходная частота"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.10", 0.0f, "Гц", 0.0f, 50.0f, "Частота нижнего предела: Частота не должна быть ниже этого значения"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.11", 0.0f, " ", 0.0f, 20.0f, "Обработка частоты верхнего предела: 0 - Нулевая скорость, 1 - Работа на нижнем пределе, 2 - Остановка"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.12", 10.0f, "с", 0.1f, 999.9f, "Время ускорения: Время, необходимое для разгона привода AC от нулевой частоты до максимальной выходной частоты"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.13", 10.0f, "с", 0.1f, 999.9f, "Время торможения: Время, необходимое для торможения привода AC от максимальной выходной частоты до нуля"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.14", 0.0f, " ", 0.0f, 2.0f, "Направление работы: 0 - Прямое вращение, 1 - Обратное вращение, 2 - Обратное вращение запрещено"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.15", 0.0f, " ", 0.0f, 9999.0f, "Пароль пользователя: Если установленное число не равно 0, пароль будет работать"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.16", 0.0f, " ", 0.0f, 99.99f, "Версия программного обеспечения"));

  // Добавляем параметры в группу F1
  allParameters[GROUP_F1].addParametr(createParameter("F1.00", 0, "", 0, 4, "0: Линейная кривая, 1: Квадратная кривая, 2: Кривая 1.5 мощности, 3: Кривая 1.2 мощности, 4: Многоточечная VF кривая"));
  allParameters[GROUP_F1].addParametr(createParameter("F1.01", 3.0, "%", 0.0f, 30.0f, "Ручное увеличение момента, это значение установлено в процентах относительно номинального напряжения двигателя."));
  allParameters[GROUP_F1].addParametr(createParameter("F1.02", 15.0, "Hz", 0.0f, 50.0f, "Частота отсечения для ручного увеличения момента"));

}