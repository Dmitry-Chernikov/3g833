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

  // Группа F0 - Основные рабочие параметры
  allParameters[GROUP_F0].addParametr(createParameter("F0.00", getPower(model), "кВт", 0.0f, 99.9f, "Текущая мощность переменного привода"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.01", 0, "", 0, 1, "0: V/F управление\n1: Открытый вектор"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.02", 0, "", 0, 2, "0: Команда запуска с панели\n1: Команда запуска с терминала\n2: Команда запуска по связи"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.03", 4, "", 0, 8, "0: Цифровая установка (предустановленная частота F0-07, регулируется с помощью UP/DOWN, отключение без памяти)\n1: Цифровая установка (предустановленная частота F0-07, регулируется с помощью UP/DOWN, отключение с памятью)\n2: AI1 (AVI)\n3: AI2 (ACI)\n4: AI3 (Клавиатурный потенциометр)\n5: Команда многоскорости\n6: Простой ПЛК\n7: PID\n8: Связь"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.04", 0, "", 0, 8, "0: Цифровая установка (предустановленная частота F0-07, регулируется с помощью UP/DOWN, отключение без памяти)\n1: Цифровая установка (предустановленная частота F0-07, регулируется с помощью UP/DOWN, отключение с памятью)\n2: AI1 (AVI)\n3: AI2 (ACI)\n4: AI3 (Клавиатурный потенциометр)\n5: Команда многоскорости\n6: Простой ПЛК\n7: PID\n8: Связь"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.05", 0, "", 0, 3, "0: Основная + вспомогательная\n1: Основная - вспомогательная\n2: Макс. (основная, вспомогательная)\n3: Мин. (основная, вспомогательная)"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.06", 0, "", 0, 4, "0: Основной источник частоты X\n1: Основной и вспомогательный расчет (определяется расчетом в F0.05)\n2: Переключение между основным источником частоты X и вспомогательным источником частоты Y\n3: Переключение между основным источником частоты X и “основным & вспомогательным расчетом”\n4: Переключение между вспомогательным источником частоты Y и “основным & вспомогательным расчетом”"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.07", 50.0f, "Гц", 0.0f, 400.0f, "Установленное значение является заданным начальным значением цифровой частоты"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.08", 50.0f, "Гц", 0.0f, 400.0f, "Максимальная выходная частота является наивысшей частотой, разрешенной для выхода переменного привода, и эталоном для настроек ускорения и замедления."));
  allParameters[GROUP_F0].addParametr(createParameter("F0.09", 50.0f, "Гц", 0.0f, 400.0f, "Рабочая частота не должна превышать эту частоту"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.10", 0.0f, "Гц", 0.0f, 400.0f, "Рабочая частота не должна быть ниже этой частоты"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.11", 0, "", 0, 2, "0: Работает на нулевой скорости\n1: Работает на нижнем пределе частоты\n2: Остановка"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.12", 10.0f, "с", 0.1f, 999.9f, "Время, необходимое для ускорения переменного привода от нулевой частоты до максимальной выходной частоты"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.13", 10.0f, "с", 0.1f, 999.9f, "Время, необходимое для замедления переменного привода от максимальной выходной частоты до нулевой частоты"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.14", 0, "", 0, 2, "0: Прямое вращение\n1: Обратное вращение\n2: Запрещено обратное вращение"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.15", 0, "", 0, 9999, "При установке числа, отличного от 0, пароль будет работать; после расшифровки, если установлен 0000, функция пароля будет отменена."));
  allParameters[GROUP_F0].addParametr(createParameter("F0.16", "xx.xx", "", "01.00", "99.99", "Текущая версия программного обеспечения."));
  allParameters[GROUP_F0].addParametr(createParameter("F0.17", 0, "", 0, 3, "0: Без действия\n1: Восстановить заводские настройки (исключая параметры двигателя)\n2: Очистка ошибок\n3: Восстановить все параметры до заводских настроек (включая параметры двигателя)"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.18", "", "", "", "", "Резерв"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.19", "", "", "", "", "Резерв"));
  allParameters[GROUP_F0].addParametr(createParameter("F0.20", 1, "", 0, 1, "0: Не сохранять\n1: Сохранять"));

  // Группа F1 - Параметры управления V/F
  allParameters[GROUP_F1].addParametr(createParameter("F1.00", 0, "", 0, 4, "0: Линейная кривая\n1: Квадратная кривая\n2: Кривая 1,5 степени\n3: Кривая 1,2 степени\n4: Многоточечная кривая VF"));
  allParameters[GROUP_F1].addParametr(createParameter("F1.01", 3.0f, "%", 0.0f, 30.0f, "Ручное увеличение крутящего момента, это значение устанавливается как процент относительно номинального напряжения двигателя.\nКогда оно равно 0, переключается на автоматическое увеличение крутящего момента."));
  allParameters[GROUP_F1].addParametr(createParameter("F1.02", 15.00f, "Гц", 0.0f, 50.00f, "Частота отсечения для ручного увеличения крутящего момента"));
  allParameters[GROUP_F1].addParametr(createParameter("F1.03", getCarrierFrequency(model), "КГц", 2.0f, 16.0f, "Увеличение несущей частоты может снизить шум, но увеличит тепловыделение переменного привода."));
  allParameters[GROUP_F1].addParametr(createParameter("F1.04", 12.50f, "Гц", 0.01f, getFrequencyF2(model), "Частотное значение V/F F1"));
  allParameters[GROUP_F1].addParametr(createParameter("F1.05", 25.0f, "%", 0.0f, getVoltageV2(model), "Напряжение V/F V1"));
  allParameters[GROUP_F1].addParametr(createParameter("F1.06", 25.00f, "Гц", getFrequencyF1(model), getFrequencyF3(model), "Частотное значение V/F F2"));
  allParameters[GROUP_F1].addParametr(createParameter("F1.07", 50.0f, "%", getVoltageV1(model), 100.0f, "Напряжение V/F V2"));
  allParameters[GROUP_F1].addParametr(createParameter("F1.08", 37.50f, "Гц", getFrequencyF2(model), getNominalFrequency(model), "Частотное значение V/F F3"));
  allParameters[GROUP_F1].addParametr(createParameter("F1.09", 75.0f, "%", getVoltageV2(model), 100.0f, "Напряжение V/F V3"));
  allParameters[GROUP_F1].addParametr(createParameter("F1.10", 0, "", 0, 2, "0: Недействительно; 1: Действительно на всем протяжении; 2: Недействительно во время замедления, действительно во время ускорения и на постоянной скорости"));
  allParameters[GROUP_F1].addParametr(createParameter("F1.11", 0.9f, "%", 0.0f, 100.0f, "Коэффициент торможения тормозного резистора"));
  allParameters[GROUP_F1].addParametr(createParameter("F1.12", 0, "%", 0.0f, 150.0f, "Увеличение компенсации крутящего момента"));
  allParameters[GROUP_F1].addParametr(createParameter("F1.13", 0.84f, "%", 0.0f, 200.0f, "Увеличение возбуждения V/F"));
  allParameters[GROUP_F1].addParametr(createParameter("F1.14", 5, "", 0, 6, "Режим подавления колебаний"));

  // Группа F2 - Параметры векторного управления
  allParameters[GROUP_F2].addParametr(createParameter("F2.00", 20, "", 1, 100, "Kp низкоскоростного контура скорости"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.01", 0.50f, "", 1.0f, 10.0f, "Ki низкоскоростного контура скорости"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.02", 10, "", 1, 100, "Kp высокоскоростного контура скорости"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.03", 1.00f, "", 1.0f, 10.0f, "Ki высокоскоростного контура скорости"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.04", 10.00f, "Гц", getLowerFrequency(model), getMaxFrequency(model), "Точка переключения расчета частоты низкоскоростного контура"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.05", 30.00f, "Гц", getLowerFrequency(model), getMaxFrequency(model), "Точка переключения расчета частоты высокоскоростного контура"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.06", 0, "%", 0.0f, 100.0f, "Компенсация электрического скольжения"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.07", "", "", "", "", "Резерв"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.08", "", "", "", "", "Резерв"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.09", "", "", "", "", "Резерв"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.10", 2000, "", 0, 60000, "Kp контура тока"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.11", 1300, "", 0, 60000, "Ki контура тока"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.12", "", "", "", "", "Резерв"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.13", "", "", "", "", "Резерв"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.14", 1, "%", 0, 200, "Коэффициент компенсации скольжения открытого векторного управления"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.15", "", "", "", "", "Резерв"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.16", "", "", "", "", "Резерв"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.17", "", "", "", "", "Резерв"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.18", "", "", "", "", "Резерв"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.19", 150.0f, "%", 0, 200.0f, "Цифровая установка предела крутящего момента в контроле скорости (привод)"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.20", 1, "%", 50, 200, "Максимальный коэффициент крутящего момента зоны ослабления поля"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.21", 5, "", 5, 300, "М-осевой коэффициент масштаба контура тока"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.22", 0, "", 0, 65535, "М-осевой интегральный коэффициент контура тока"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.23", 25, "", 0, 100, "Фильтр временной константы контура скорости открытого векторного управления"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.24", 100, "", 0, 500, "Открытое векторное управление увеличения крутящего момента"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.25", 20.00f, "Гц", getLowerFrequency(model), getMaxFrequency(model), "Частота отсечения открытого векторного управления увеличения крутящего момента"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.26", 28, "", 0, 31, "Фильтр заданного крутящего момента"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.27", 1.05f, "%", 0, 110, "Максимальный коэффициент модуляции ослабления поля"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.28", 1, "%", 0, 100, "Коэффициент компенсации наблюдения потока"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.29", 300, "", 0, 2000, "Коэффициент фильтрации наблюдения потока"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.30", 0, "", 0, 500, "T-осевой коэффициент замкнутого контура тока"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.31", 0, "", 0, 1, "Метод ограничения крутящего момента"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.32", "", "", "", "", "Резерв"));
  allParameters[GROUP_F2].addParametr(createParameter("F2.33", "", "", "", "", "Резерв"));

  // Группа F3 - Вспомогательные рабочие параметры
  allParameters[GROUP_F3].addParametr(createParameter("F3.00", 0, "", 0, 1, "0: Запуск по стартовой частоте\n1: Запуск по стартовой частоте после торможения постоянным током"));
  allParameters[GROUP_F3].addParametr(createParameter("F3.01", 0.50f, "Гц", 0.50f, 20.00f, "Начальная частота запуска переменного привода"));
  allParameters[GROUP_F3].addParametr(createParameter("F3.02", 0, "с", 0.0f, 60.0f, "Время работы на стартовой частоте"));
  allParameters[GROUP_F3].addParametr(createParameter("F3.03", 0.0f, "%", 0.0f, 100.0f, "Текущая величина для применения торможения постоянным током\nКогда номинальный ток двигателя меньше или равен 80% от номинального тока переменного привода, это процентная база относительно номинального тока двигателя;\nКогда номинальный ток двигателя больше 80% от номинального тока переменного привода, это процентная база относительно 80% от номинального тока переменного привода;"));
  allParameters[GROUP_F3].addParametr(createParameter("F3.04", 0.0f, "с", 0.0f, 60.0f, "Продолжительность применения торможения постоянным током"));
  allParameters[GROUP_F3].addParametr(createParameter("F3.05", 0, "", 0, 2, "0: Замедление до остановки\n1: Замедление до остановки + торможение постоянным током\n2: Свободная остановка"));
  allParameters[GROUP_F3].addParametr(createParameter("F3.06", 0.00f, "Гц", 0.00f, getUpperFrequency(model), "Когда частота достигает предустановленной частоты, начинает работать торможение постоянным током"));
  allParameters[GROUP_F3].addParametr(createParameter("F3.07", 0.0f, "%", 0.0f, 100.0f, "Текущая величина для применения торможения постоянным током такая же, как и “торможение постоянным током при запуске”"));
  allParameters[GROUP_F3].addParametr(createParameter("F3.08", 0.0f, "с", 0.0f, 30.0f, "Продолжительность применения торможения постоянным током"));

    // Резервные параметры
    for (int i = 9; i <= 15; ++i) {
        allParameters[GROUP_F3].addParametr(createParameter("F3." + std::to_string(i), "", "", "", "", "Резерв"));
  
  // Группа F4 - Вспомогательные рабочие параметры 2
  allParameters[GROUP_F4].addParametr(createParameter("F4.00", 10.00f, "Гц", 0.00f, 50.00f, "Установка частоты джога FWD & REV"));
  allParameters[GROUP_F4].addParametr(createParameter("F4.01", 0.00f, "Гц", 0.00f, 50.00f, "Установка частоты для REV джога"));
  allParameters[GROUP_F4].addParametr(createParameter("F4.02", getAccelerationTime(model), "с", 0.1f, 999.9f, "Установка времени ускорения и замедления джога"));
  allParameters[GROUP_F4].addParametr(createParameter("F4.03", 0.00f, "с", 0.0f, 999.9f, "Время замедления джога"));
  allParameters[GROUP_F4].addParametr(createParameter("F4.04", 10.0f, "с", 0.1f, 999.9f, "Время ускорения 2"));
  allParameters[GROUP_F4].addParametr(createParameter("F4.05", 10.0f, "с", 0.1f, 999.9f, "Время замедления 2"));
  allParameters[GROUP_F4].addParametr(createParameter("F4.06", 1, "", 0, 1, "0: Недействительно\n1: Когда переменный привод работает, JOG имеет самый высокий приоритет"));
  allParameters[GROUP_F4].addParametr(createParameter("F4.07", 0.00f, "Гц", 0.0f, getUpperFrequency(model), "Установив пропускаемую частоту и диапазон, переменный привод может избежать механической резонансной точки нагрузки."));
  allParameters[GROUP_F4].addParametr(createParameter("F4.08", 0.00f, "Гц", 0.0f, 10.0f, "Пропускаемый диапазон"));

  // Группа F5 - Параметры цифровых входов/выходов
  allParameters[GROUP_F5].addParametr(createParameter("F5.00", 0, "", 0, 3, "0: Двухпроводной режим управления 1\n1: Двухпроводной режим управления 2\n2: Трехпроводной режим управления 1\n3: Трехпроводной режим управления 2"));
  allParameters[GROUP_F5].addParametr(createParameter("F5.01", 3, "", 0, 1, "0: Команда запуска терминала недействительна при включении\n1: Команда запуска терминала действительна при включении"));
  allParameters[GROUP_F5].addParametr(createParameter("F5.02", 4, "", 0, 27, "0: Нет функции\n1: Контроль прямого джога\n2: Контроль обратного джога\n3: Контроль прямого вращения (FWD)\n4: Контроль обратного вращения (REV)\n5: Трехпроводной контроль\n6: Свободная остановка\n7: Вход внешнего сигнала остановки (STOP)\n8: Вход внешнего сигнала сброса (RST)\n9: Вход внешнего сигнала неисправности нормально открытый (NO)\n10: Команда увеличения частоты (UP)\n11: Команда уменьшения частоты (DOWN)\n12: Выбор многоскорости S1\n13: Выбор многоскорости S2\n14: Выбор многоскорости S3\n15: Канал команды запуска принудительно на терминал\n16: Резерв\n17: Команда торможения постоянным током\n18: Переключение источника частоты (F0.06)\n19: Резерв\n20: Резерв\n21: Резерв\n22: Сигнал сброса счетчика (Fb.10 функция подсчета)\n23: Сигнал триггера счетчика (Fb.10 функция подсчета)\n24: Сигнал сброса таймера (Fb.10 функция таймера)\n25: Сигнал триггера таймера (Fb.10 функция таймера)\n26: Время ускорения/замедления"));
  allParameters[GROUP_F5].addParametr(createParameter("F5.03", 12, "", 0, 27, "Функции входного терминала X2"));
  allParameters[GROUP_F5].addParametr(createParameter("F5.04", 0, "", 0, 27, "Функции входного терминала X3"));
  allParameters[GROUP_F5].addParametr(createParameter("F5.05", 8, "", 0, 27, "Функции входного терминала X4 (версия связи: 485+)"));
  allParameters[GROUP_F5].addParametr(createParameter("F5.06", 5, "", 0, 27, "Функции входного терминала X5 (версия связи: 485-)"));
  allParameters[GROUP_F5].addParametr(createParameter("F5.07", 0, "с", 0, 14, "0: Нет функции\n1: Переменный привод готов к запуску\n2: Переменный привод работает\n3: Переменный привод работает на нулевой скорости\n4: Внешняя неисправность остановила\n5: Неисправность переменного привода\n6: Сигнал достижения частоты/скорости (FAR)\n7: Сигнал уровня частоты/скорости (FDT)\n8: Выходная частота достигает верхнего предела\n9: Выходная частота достигает нижнего предела\n10: Предупреждение о перегрузке переменного привода\n11: Сигнал переполнения таймера (выход реле, когда время таймирования достигает установленного времени в Fb.13)\n12: Сигнал обнаружения счетчика (выход реле, когда значение подсчета достигает значения, обнаруженного счетчиком в Fb.12)\n13: Сигнал сброса счетчика (резерв)\n14: Резерв"));
  allParameters[GROUP_F5].addParametr(createParameter("F5.08", 0.0f, "с", 0.0f, 999.9f, "Задержка от изменения состояния реле R до изменения выхода"));
  allParameters[GROUP_F5].addParametr(createParameter("F5.09", 5.00f, "Гц", 0.0f, 50.0f, "Задержка открытия R"));
  allParameters[GROUP_F5].addParametr(createParameter("F5.10", 10.00f, "Гц", 0.00f, 15.00f, "Когда выходная частота попадает в положительную и отрицательную ширину обнаружения установленной частоты, терминал выдает действительный сигнал (низкий уровень)."));
  allParameters[GROUP_F5].addParametr(createParameter("F5.11", 5, "", 0.00f, getUpperFrequency(model), "Установленное значение уровня FDT"));
  allParameters[GROUP_F5].addParametr(createParameter("F5.16", 5, "", 0, 9999, "Коэффициент фильтрации X1"));
  allParameters[GROUP_F5].addParametr(createParameter("F5.17", 5, "", 0, 9999, "Коэффициент фильтрации X2"));
  allParameters[GROUP_F5].addParametr(createParameter("F5.18", 5, "", 0, 9999, "Коэффициент фильтрации X3"));
  allParameters[GROUP_F5].addParametr(createParameter("F5.19", 5, "", 0, 9999, "Коэффициент фильтрации X4"));
  allParameters[GROUP_F5].addParametr(createParameter("F5.20", 0, "", 0, 9999, "Коэффициент фильтрации X5"));

  // Группа F6 - Функции аналогового ввода и вывода
  allParameters[GROUP_F6].addParametr(createParameter("F6.00", 0.0f, "%", 0.0f, 100.0f, "Установить нижний предел напряжения AVI"));
  allParameters[GROUP_F6].addParametr(createParameter("F6.01", 100.0f, "%", 0.0f, 100.0f, "Установить верхний предел напряжения AVI"));
  allParameters[GROUP_F6].addParametr(createParameter("F6.02", 0.0f, "%", -100.0f, 100.0f, "Установить соответствующий процент нижнего предела AVI, который соответствует проценту максимальной частоты."));
  allParameters[GROUP_F6].addParametr(createParameter("F6.03", 100.0f, "%", -100.0f, 100.0f, "Установить соответствующий процент верхнего предела AVI, который соответствует проценту максимальной частоты."));
  allParameters[GROUP_F6].addParametr(createParameter("F6.04", 0.0f, "%", 0.0f, 100.0f, "Установить нижний предел тока ACI"));
  allParameters[GROUP_F6].addParametr(createParameter("F6.05", 100.0f, "%", 0.0f, 100.0f, "Установить верхний предел тока ACI"));
  allParameters[GROUP_F6].addParametr(createParameter("F6.06", 0.0f, "%", -100.0f, 100.0f, "Установить соответствующий процент нижнего предела ACI, который соответствует проценту максимальной частоты."));
  allParameters[GROUP_F6].addParametr(createParameter("F6.07", 100.0f, "%", -100.0f, 100.0f, "Установить соответствующий процент верхнего предела ACI, который соответствует проценту максимальной частоты."));
  allParameters[GROUP_F6].addParametr(createParameter("F6.08", 0.1f, "с", 0.1f, 5.0f, "Этот параметр используется для фильтрации входного сигнала AVI, ACI и клавиатурного потенциометра, чтобы устранить влияние помех."));
  allParameters[GROUP_F6].addParametr(createParameter("F6.09", 0, "%", 0.0f, 100.0f, "Когда аналоговый входной сигнал часто колеблется вокруг установленного значения, установите этот параметр, чтобы подавить колебания частоты, вызванные таким колебанием."));
  allParameters[GROUP_F6].addParametr(createParameter("F6.10", 0, "", 0, 5, "0: Выходная частота, 0~Максимальная частота\n1: Установленная частота, 0~Максимальная частота\n2: Выходной ток, 0~2 раза номинального тока\n3: Выходное напряжение, 0~2 раза номинального напряжения\n4: AVI, 0~10В\n5: ACI, 0~20мА"));
  allParameters[GROUP_F6].addParametr(createParameter("F6.11", 0.0f, "%", 0.0f, 100.0f, "Установить нижний предел функции AO"));
  allParameters[GROUP_F6].addParametr(createParameter("F6.12", 100.0f, "%", 0.0f, 100.0f, "Установить верхний предел функции AO"));
  allParameters[GROUP_F6].addParametr(createParameter("F6.13", 0.0f, "%", 0.0f, 100.0f, "Установить нижний предел AO вывода"));
  allParameters[GROUP_F6].addParametr(createParameter("F6.14", 100.0f, "%", 0.0f, 100.0f, "Установить верхний предел AO вывода"));

  // Группа F7 - Параметры запуска программы (PLC)
  allParameters[GROUP_F7].addParametr(createParameter("F7.00", 5.00f, "Гц", getLowerFrequency(model), getUpperFrequency(model), "Установить частоту 1"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.01", 10.00f, "Гц", getLowerFrequency(model), getUpperFrequency(model), "Установить частоту 2"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.02", 15.00f, "Гц", getLowerFrequency(model), getUpperFrequency(model), "Установить частоту 3"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.03", 20.00f, "Гц", getLowerFrequency(model), getUpperFrequency(model), "Установить частоту 4"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.04", 25.00f, "Гц", getLowerFrequency(model), getUpperFrequency(model), "Установить частоту 5"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.05", 37.50f, "Гц", getLowerFrequency(model), getUpperFrequency(model), "Установить частоту 6"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.06", 50.00f, "Гц", getLowerFrequency(model), getUpperFrequency(model), "Установить частоту 7"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.07", 0, "", 0, 2, "0: Однократный цикл\n1: Непрерывный цикл\n2: Сохранить конечное значение после одного цикла"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.08", 0, "", 0, 1, "0: Остановка без памяти, 1: Остановка с памятью"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.09", 0, "", 0, 1, "0: Отключение без памяти, 1: Отключение с памятью"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.10", 10.0f, "с", 0.0f, 999.9f, "Установить время работы скорости 1"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.11", 10.0f, "с", 0.0f, 999.9f, "Установить время работы скорости 2"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.12", 10.0f, "с", 0.0f, 999.9f, "Установить время работы скорости 3"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.13", 10.0f, "с", 0.0f, 999.9f, "Установить время работы скорости 4"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.14", 10.0f, "с", 0.0f, 999.9f, "Установить время работы скорости 5"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.15", 10.0f, "с", 0.0f, 999.9f, "Установить время работы скорости 6"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.16", 10.0f, "с", 0.0f, 999.9f, "Установить время работы скорости 7"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.17", 0, "", 0, 3, "0: FWD, выбрать время ускорения 1\n1: FWD, выбрать время ускорения 2\n2: REV, выбрать время ускорения 1\n3: REV, выбрать время ускорения 2"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.18", 0, "", 0, 3, "Режим работы T2"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.19", 0, "", 0, 3, "Режим работы T3"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.20", 0, "", 0, 3, "Режим работы T4"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.21", 0, "", 0, 3, "Режим работы T5"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.22", 0, "", 0, 3, "Режим работы T6"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.23", 0, "", 0, 3, "Режим работы T7"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.24", "", "", "", "", "Текущий рабочий раздел (резерв)"));
  allParameters[GROUP_F7].addParametr(createParameter("F7.25", "", "", "", "", "Текущее рабочее время (резерв)"));

  // Группа F8 - Параметры PID
  allParameters[GROUP_F8].addParametr(createParameter("F8.00", 0, "", 0, 1, "0: Прямое действие\n1: Обратное действие"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.01", 0, "", 0, 3, "0: Цифровая установка\n1: Настройка клавиатурного потенциометра\n2: Вход AVI\n3: Вход ACI"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.02", 0, "", 0, 1, "0: Вход AVI\n1: Вход ACI"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.03", 0.5f, "", getLowerPIDValue(model), getUpperPIDValue(model), "Установленное значение, когда источник заданного PID является цифровой установкой"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.04", 0.0f, "с", 0.0f, 100.0f, "Время ускорения/замедления PID"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.05", 0.0f, "%", 0.0f, 100.0f, "Установка смещения PID"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.06", 0.0f, "с", 0.0f, 6000.0f, "Время удержания смещения PID"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.07", 100.0f, "%", 0.0f, 100.0f, "Верхний предел отклонения PID"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.08", 0.0f, "%", 0.0f, 100.0f, "Нижний предел отклонения PID (Максимальная частота)"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.09", 25.00f, "", 0.0f, 600.0f, "Пропорциональный коэффициент"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.10", 1.0f, "с", 0.0f, 100.0f, "Интегральное время"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.11", 0.00f, "с", 0.0f, 10.0f, "Дифференциальное время"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.12", 100.0f, "%", 0.0f, 100.0f, "Верхний предел выхода PID"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.13", 0.0f, "%", 0.0f, 100.0f, "Нижний предел выхода PID"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.14", 0.00f, "с", 0.0f, 10.0f, "Фильтр времени выхода PID"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.15", 2, "", 0, 4, "0: Работает на верхнем пределе частоты\n1: Работает на нижнем пределе частоты\n2: Работает на частоте цифровой установки\n3: Замедление до остановки\n4: Свободная остановка"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.16", 0.0f, "%", 0.0f, 100.0f, "Значение обнаружения потери"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.17", 1.0f, "с", 0.0f, 100.0f, "Время обнаружения потери"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.18", 100.0f, "%", 0.0f, 100.0f, "Значение обнаружения избыточности"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.19", 1.0f, "с", 0.0f, 100.0f, "Время обнаружения избыточности"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.20", 0, "", 0, 2, "0: Нет функции сна\n1: Внутреннее пробуждение\n2: Управление внешним входным терминалом"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.21", 0, "", 0, 1, "0: Замедление до остановки\n2: Свободная остановка"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.22", 0.00f, "Гц", 0.00f, getUpperFrequency(model), "Частота сна"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.23", 95.0f, "%", 0.0f, 100.0f, "Давление сна"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.24", 30.0f, "с", 0.0f, 6000.0f, "Время задержки сна"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.25", 80.0f, "%", 0.0f, 100.0f, "Давление пробуждения"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.26", 3.0f, "с", 0.0f, 60.0f, "Время задержки пробуждения"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.27", 0.0f, "", -3276.8f, 3276.8f, "Нижний предел диапазона PID"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.28", 10.0f, "", -3276.8f, 3276.8f, "Верхний предел диапазона PID"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.29", 1, "", 0, 3, "0: Не отображать десятичные разряды\n1: Отображать одну десятичную точку\n2: Отображать две десятичные точки\n3: Отображать три десятичные точки"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.30", 48.00f, "Гц", 0.00f, getUpperFrequency(model), "Частота обнаружения нехватки воды"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.31", 0.0f, "", 0.0f, getUpperPIDValue(model), "Давление обнаружения нехватки воды"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.32", 60.0f, "с", 0.0f, 6500.0f, "Время обнаружения нехватки воды"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.33", 600.0f, "с", 0.0f, 6500.0f, "Время перезапуска после нехватки воды"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.34", 6, "", 0, 9999, "Количество перезапусков после нехватки воды"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.35", "", "", "", "", "Резерв"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.36", 0, "", 0, 3, "0: Отключен\n1: Режим работы насоса PV 1\n2: Режим работы насоса PV 2"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.37", 0, "", 0, 3, "0: Отключен\n1: MPPT включен\n2: Насос PV включен\n3: MPPT и насос PV включены"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.38", getMaxMPPTVoltage(model), "В", getMinMPPTVoltage(model), 1000.0f, "Максимальное рабочее напряжение MPPT"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.39", 0, "", 0, 1, "0: Включено\n1: Отключено"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.40", 0, "", 0, 1, "0: Отключено\n1: Включено"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.41", 10.0f, "с", 0.0f, 360.0f, "Задержка перезапуска при недостаточном напряжении"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.42", 0, "", 0, 1, "0: Отключено\n1: Включено"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.43", 0.0f, "%", 0.0f, 300.0f, "Соотношение тока без нагрузки, соответствующее току обнаружения нехватки воды насоса PV"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.44", 0.00f, "Гц", 0.0f, 99.99f, "Минимальная частота отлива насоса PV"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.45", 0.0f, "с", 0.0f, 250.0f, "Время обнаружения нехватки воды насоса PV"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.46", 0, "", 0, 1, "0: Отключено\n1: Включено"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.47", 0, "", 0, 1, "0: Относительно максимальной частоты\n1: Относительно центральной частоты"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.48", 0, "", 0, 1, "0: Запомнить состояние перед остановкой\n1: Перезапустить старт"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.49", 0.0f, "%", 0.0f, 100.0f, "Амплитуда колебаний"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.50", 0.0f, "%", 0.0f, 50.0f, "Шаг колебаний"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.51", 5.0f, "с", 0.1f, 400.0f, "Время нарастания колебаний"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.52", 5.0f, "с", 0.1f, 400.0f, "Время спада колебаний"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.53", 5.0f, "с", 0.1f, 999.9f, "Задержка верхней частоты"));
  allParameters[GROUP_F8].addParametr(createParameter("F8.54", 5.0f, "с", 0.1f, 999.9f, "Задержка нижней частоты"));

  // Группа F9 - Параметры двигателя
  allParameters[GROUP_F9].addParametr(createParameter("F9.00", getNominalPower(model), "", "", "", "Настройка параметров двигателя"));
  allParameters[GROUP_F9].addParametr(createParameter("F9.01", getNominalVoltage(model), "В", 1.0f, 500.0f, "Настройка параметров двигателя"));
  allParameters[GROUP_F9].addParametr(createParameter("F9.02", getNominalCurrent(model), "А", 0.01f, 99.99f, ""));
  allParameters[GROUP_F9].addParametr(createParameter("F9.03", getNominalSpeed(model), "Об/мин", 0.0f, 60000.0f, ""));
  allParameters[GROUP_F9].addParametr(createParameter("F9.04", 50.0f, "Гц", 1.0f, 400.0f, ""));
  allParameters[GROUP_F9].addParametr(createParameter("F9.05", 0, "", 0, 1, "0: Отключить идентификацию параметров; 1: Включить статическую идентификацию параметров, автоматически устанавливается в 0 после идентификации"));
  allParameters[GROUP_F9].addParametr(createParameter("F9.06", getStatorResistance(model), "Ω", 0.001f, 65.535f, "У разных моделей есть соответствующие заводские значения, и идентификация параметров автоматически изменит значение"));
  // Параметры F9.07, F9.08, F9.09 могут быть добавлены аналогично, если они известны
  allParameters[GROUP_F9].addParametr(createParameter("F9.11", getNoLoadCurrent(model), "А", 0.01f, "", "Установить ток без нагрузки двигателя; У разных моделей есть соответствующие заводские значения, и идентификация параметров автоматически изменит значение"));

  // Группа FA - Параметры защиты
  allParameters[GROUP_FA].addParametr(createParameter("FA.00", 0, "", 0, 1, "0: Недействительно\n1: Действительно"));
  allParameters[GROUP_FA].addParametr(createParameter("FA.01", 1, "%", 30.0f, 110.0f, "Коэффициент защиты от перегрузки двигателя - это процентное соотношение номинального тока двигателя к номинальному выходному току переменного привода."));
  allParameters[GROUP_FA].addParametr(createParameter("FA.02", "180/360В", "", "150-280 / 300~480В", "", "Допустимое нижнее значение напряжения на шине постоянного тока, когда переменный привод работает нормально."));
  allParameters[GROUP_FA].addParametr(createParameter("FA.03", 1, "", 0, 1, "0: Отключить\n1: Включить"));
  allParameters[GROUP_FA].addParametr(createParameter("FA.04", "375/660В", "", "350-380 / 660~760В", "", "Рабочее напряжение во время защиты от перенапряжения"));
  allParameters[GROUP_FA].addParametr(createParameter("FA.05", 1.5f, "%", 30.0f, 200.0f, "Порог тока автоматического ограничения тока, установленное значение является процентом относительно номинального тока переменного привода."));
  allParameters[GROUP_FA].addParametr(createParameter("FA.06", 0, "Гц/с", 0, 99.99f, "Скорость падения частоты при ограничении тока"));
  allParameters[GROUP_FA].addParametr(createParameter("FA.07", 0, "", 0, 2, "0: Недействительно\n1: Действительно во время ускорения/замедления, недействительно на постоянной скорости\n2: Действительно во время ускорения и замедления, действительно на постоянной скорости"));
  allParameters[GROUP_FA].addParametr(createParameter("FA.08", 1.2f, "%", 120.0f, 150.0f, "Текущий порог действия предварительного предупреждения о перегрузке переменного привода."));
  allParameters[GROUP_FA].addParametr(createParameter("FA.09", 5.0f, "с", 0.0f, 15.0f, "Время задержки от превышения уровня предварительного предупреждения о перегрузке."));
  allParameters[GROUP_FA].addParametr(createParameter("FA.10", 30, "", 0, 200, "Увеличьте эту настройку, когда возникают колебания двигателя."));
  allParameters[GROUP_FA].addParametr(createParameter("FA.11", 20, "", 0, 1000, "Установить максимальную величину корректировки для подавления колебаний."));
  allParameters[GROUP_FA].addParametr(createParameter("FA.12", 5.00f, "Гц", 0.0f, getUpperFrequencyDamping(model), "Ниже этой частоты подавление колебаний будет неэффективным."));
  allParameters[GROUP_FA].addParametr(createParameter("FA.13", 50.00f, "Гц", getLowerFrequencyDamping(model), 200.00f, "Выше этой частоты подавление колебаний будет неэффективным."));
  allParameters[GROUP_FA].addParametr(createParameter("FA.14", 11, "", 0, 111, "Выбор во время ускорения, 0: Недействительно, 1: Действительно; выбор во время замедления и на постоянной скорости."));
  allParameters[GROUP_FA].addParametr(createParameter("FA.15", 180, "%", 80.0f, 200.0f, "Номинальный ток переменного привода."));
  allParameters[GROUP_FA].addParametr(createParameter("FA.16", 0, "", 0, 10, "Когда установлено в 0, автоматический сброс отключен."));
  allParameters[GROUP_FA].addParametr(createParameter("FA.17", 3.0f, "с", 0.5f, 25.0f, "Установить интервал автоматического сброса неисправностей."));
  allParameters[GROUP_FA].addParametr(createParameter("FA.18", 3, "", 0, 3, "0: Без действия\n1: Включение подавления перегрузки\n2: Включение подавления перенапряжения\n3: Включение подавления перегрузки/перенапряжения"));
  allParameters[GROUP_FA].addParametr(createParameter("FA.19", 20, "", 0, 100, "Подавление перегрузки VF Kp"));
  allParameters[GROUP_FA].addParametr(createParameter("FA.20", 50, "", 50, 200, "Коэффициент компенсации предела тока, умноженного на скорость."));
  allParameters[GROUP_FA].addParametr(createParameter("FA.21", 60, "", 0, 100, "Подавление перенапряжения VF Kp"));
  allParameters[GROUP_FA].addParametr(createParameter("FA.22", 5, "", 0, 50, "Порог частоты VF при подавлении перенапряжения."));
  allParameters[GROUP_FA].addParametr(createParameter("FA.23", 80, "", 0, 100, "Регулирование напряжения VF во время защиты от перенапряжения Kp."));
  allParameters[GROUP_FA].addParametr(createParameter("FA.24", 0, "", 0, 1, "0: Сообщить об ошибке недостаточного напряжения, свободная остановка;\n1: Не сообщать об ошибке недостаточного напряжения, остановка по установленному режиму остановки (F3.05)."));
  allParameters[GROUP_FA].addParametr(createParameter("FA.25", "", "", "", "", "Резерв"));
  allParameters[GROUP_FA].addParametr(createParameter("FA.26", 1, "", 0, 1, "0: Защита от потери фазы на выходе отключена\n1: Защита от потери фазы на выходе включена"));

  // Группа Fb - Параметры отображения и специальные параметры
  allParameters[GROUP_FB].addParametr(createParameter("Fb.00", 0, "", 0, 15, "Элементы отображения по умолчанию на главном интерфейсе мониторинга. Соответствующие номера являются параметрами группы d."));
  allParameters[GROUP_FB].addParametr(createParameter("Fb.01", 1, "", 0, 15, "Элементы отображения по умолчанию на главном интерфейсе мониторинга. Соответствующие номера являются параметрами группы d."));
  allParameters[GROUP_FB].addParametr(createParameter("Fb.02", 1.00f, "", 0.01f, 99.99f, "Используется для коррекции ошибки отображения шкалы скорости и не влияет на фактическую скорость."));
  allParameters[GROUP_FB].addParametr(createParameter("Fb.03", 0, "", 0, 9999, "Код текущей ошибки"));
  allParameters[GROUP_FB].addParametr(createParameter("Fb.04", 0, "", 0, 9999, "Код предыдущей ошибки"));
  allParameters[GROUP_FB].addParametr(createParameter("Fb.05", 0, "", 0, 9999, "Код предыдущей ошибки два"));
  allParameters[GROUP_FB].addParametr(createParameter("Fb.06", 0, "", 0, 9999, "Напряжение на шине при ошибке"));
  allParameters[GROUP_FB].addParametr(createParameter("Fb.07", 0, "", 0, 999.9f, "Ток на шине при ошибке"));
  allParameters[GROUP_FB].addParametr(createParameter("Fb.08", 0, "", 0, 300.0f, "Установленная частота при ошибке"));
  allParameters[GROUP_FB].addParametr(createParameter("Fb.09", 0, "", 0, 300.0f, "Рабочая частота при ошибке"));
  allParameters[GROUP_FB].addParametr(createParameter("Fb.10", 103, "", 0, 303, "Единицы: Обработка прихода подсчета, 0: Однократный подсчет, остановить выход; 1: Однократный подсчет, продолжить выход; 2: Циклический подсчет, остановить выход; 3: Циклический подсчет, продолжить выход. Десятки: Резерв Сотни: Обработка прихода таймирования."));
  allParameters[GROUP_FB].addParametr(createParameter("Fb.11", 1, "", 0, 9999, "Установить значение сброса счетчика"));
  allParameters[GROUP_FB].addParametr(createParameter("Fb.12", 1, "", 0, 9999, "Установить значение обнаружения счетчика"));
  allParameters[GROUP_FB].addParametr(createParameter("Fb.13", 0, "с", 0, 9999, "Установить время таймирования"));
    // Резерв
    for (int i = 14; i <= 19; i++) {
        allParameters[GROUP_FB].addParametr(createParameter("Fb." + String(i), "", "", "", "", "Резерв"));
    }
  allParameters[GROUP_FB].addParametr(createParameter("Fb.20", "", "", "", "", "Дата обновления программного обеспечения (год)"));
  allParameters[GROUP_FB].addParametr(createParameter("Fb.21", "", "", "", "", "Дата обновления программного обеспечения (месяц день)"));
  allParameters[GROUP_FB].addParametr(createParameter("Fb.22", 1.00f, "", "", "", "Отображение версии программного обеспечения"));

  // Группа FC - Параметры связи
  allParameters[GROUP_FC].addParametr(createParameter("FC.00", 3, "", 0, 5, "0: 1200\n1: 2400\n2: 4800\n3: 9600\n4: 19200\n5: 38400"));
  allParameters[GROUP_FC].addParametr(createParameter("FC.01", 0, "", 0, 6, "Формат данных: <Длина данных, позиция остановки>\n0: Без проверки, <8,1>\n1: Проверка нечетности, <9,1>\n2: Проверка четности, <9,1>\n3: Без проверки, <8,1>\n4: Проверка четности, <8,1>\n5: Проверка нечетности, <8,1>\n6: Без проверки, <8,2>"));
  allParameters[GROUP_FC].addParametr(createParameter("FC.02", 1, "", 1, 247, "1-247 представляет местный адрес"));
  allParameters[GROUP_FC].addParametr(createParameter("FC.03", 10.0f, "с", 0.0f, 600.0f, "Тайм-аут связи"));
  allParameters[GROUP_FC].addParametr(createParameter("FC.04", "", "", "", "", "Резерв"));
  allParameters[GROUP_FC].addParametr(createParameter("FC.05", 1, "", 0, 2, "0: Без действия\n1: Сигнал тревоги\n2: Остановка по неисправности"));

  // Группа FP - Заводские параметры
  allParameters[GROUP_FP].addParametr(createParameter("FP.00", "", "", 1, 9999, "Специфический пароль для настройки системы"));

  // Группа d - Параметры мониторинга
  allParameters[GROUP_D].addParametr(createParameter("d-00", 0.00f, "Гц", 0.00f, 400.00f, ""));
  allParameters[GROUP_D].addParametr(createParameter("d-01", 0.00f, "Гц", 0.00f, 400.00f, ""));
  allParameters[GROUP_D].addParametr(createParameter("d-02", 0, "В", 0, 999, ""));
  allParameters[GROUP_D].addParametr(createParameter("d-03", 0, "В", 0, 999, ""));
  allParameters[GROUP_D].addParametr(createParameter("d-04", 0.0f, "А", 0.0f, 999.9f, ""));
  allParameters[GROUP_D].addParametr(createParameter("d-05", 0, "Об/мин", 0, 60000, ""));
  allParameters[GROUP_D].addParametr(createParameter("d-06", 0.00f, "В", 0.00f, 10.00f, ""));
  allParameters[GROUP_D].addParametr(createParameter("d-07", 0.00f, "мА", 0.00f, 20.00f, ""));
  allParameters[GROUP_D].addParametr(createParameter("d-08", 0.00f, "В", 0.00f, 10.00f, ""));
  allParameters[GROUP_D].addParametr(createParameter("d-09", 0, "", 0, 0x3F, "Состояние входного терминала (Реле, X1-X5)"));
  allParameters[GROUP_D].addParametr(createParameter("d-10", 0, "℃", 0, 9999, ""));
  allParameters[GROUP_D].addParametr(createParameter("d-11", getLowerPIDValue(model), "", getLowerPIDValue(model), getUpperPIDValue(model), "Заданное значение PID"));
  allParameters[GROUP_D].addParametr(createParameter("d-12", getLowerPIDValue(model), "", getLowerPIDValue(model), getUpperPIDValue(model), "Значение обратной связи PID"));
  allParameters[GROUP_D].addParametr(createParameter("d-13", 0, "", 0, 9999, "Текущее значение счетчика"));
  allParameters[GROUP_D].addParametr(createParameter("d-14", 0, "с", 0, 9999, "Текущее значение таймера (с)"));
  allParameters[GROUP_D].addParametr(createParameter("d-15", 0, "ч", 0, 9999, "Накопительное время работы переменного привода (ч)"));
  allParameters[GROUP_D].addParametr(createParameter("d-16", 0, "ч", 0, 9999, "Накопительное время включения переменного привода (ч)"));
  allParameters[GROUP_D].addParametr(createParameter("d-17", 0, "", 0, 4095, "Смещение выборки тока фазы U"));
  allParameters[GROUP_D].addParametr(createParameter("d-18", 0, "", 0, 4095, "Смещение выборки тока фазы V"));
  allParameters[GROUP_D].addParametr(createParameter("d-19", 0, "", 0, 4095, "Смещение выборки тока фазы W"));
  }
}