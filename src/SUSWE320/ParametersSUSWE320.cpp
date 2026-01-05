#include "ParametersSUSWE320.h"

// Функция для получения мощности модели
float ParametersSUSWE320::getPower(Model model) { 
  return modelPowers[static_cast<int>(model)]; 
}

// Функция возвращает указатель на информацию об ошибке по её коду
const FaultInfo* ParametersSUSWE320::getFaultInfo(int code ) {
    // Проверяем, что код находится в допустимом диапазоне
    if (0 <= code - 1 && code - 1 < _faultCount) {
        // Возвращаем указатель на элемент массива faultTable, соответствующий коду ошибки
        return &_faultTable[code - 1];
    } else {
        // Если код не найден, возвращаем nullptr
        return nullptr; 
    }    
}

Parameter ParametersSUSWE320::createParameter(const char* name, const float defaultValue, const char* unit, const float min, const float max, const char* description) {
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

ParametersSUSWE320::~ParametersSUSWE320(){
    //delete[] _allParameters;
}

// Конструктор ParametersSUSWE320
ParametersSUSWE320::ParametersSUSWE320(const Model model): _allParameters{ParameterGroup("F0 - Основные рабочие параметры", 21),
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
                                                               ParameterGroup("d - Параметры мониторинга", 20)},
                   _model(model) {

  // Группа F0 - Основные рабочие параметры
  _allParameters[GROUP_F0].addParameter(createParameter("F0.00", getPower(model), "кВт", 0.0f, 99.9f, "Текущая мощность переменного привода"));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.01", 0, "", 0, 1, "0: V/F управление\n1: Открытый вектор"));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.02", 0, "", 0, 2, "0: Команда запуска с панели\n1: Команда запуска с терминала\n2: Команда запуска по связи"));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.03", 4, "", 0, 8, "0: Цифровая установка (предустановленная частота F0-07, регулируется с помощью UP/DOWN, отключение без памяти)\n1: Цифровая установка (предустановленная частота F0-07, регулируется с помощью UP/DOWN, отключение с памятью)\n2: AI1 (AVI)\n3: AI2 (ACI)\n4: AI3 (Клавиатурный потенциометр)\n5: Команда многоскорости\n6: Простой ПЛК\n7: PID\n8: Связь"));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.04", 0, "", 0, 8, "0: Цифровая установка (предустановленная частота F0-07, регулируется с помощью UP/DOWN, отключение без памяти)\n1: Цифровая установка (предустановленная частота F0-07, регулируется с помощью UP/DOWN, отключение с памятью)\n2: AI1 (AVI)\n3: AI2 (ACI)\n4: AI3 (Клавиатурный потенциометр)\n5: Команда многоскорости\n6: Простой ПЛК\n7: PID\n8: Связь"));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.05", 0, "", 0, 3, "0: Основная + вспомогательная\n1: Основная - вспомогательная\n2: Макс. (основная, вспомогательная)\n3: Мин. (основная, вспомогательная)"));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.06", 0, "", 0, 4, "0: Основной источник частоты X\n1: Основной и вспомогательный расчет (определяется расчетом в F0.05)\n2: Переключение между основным источником частоты X и вспомогательным источником частоты Y\n3: Переключение между основным источником частоты X и “основным & вспомогательным расчетом”\n4: Переключение между вспомогательным источником частоты Y и “основным & вспомогательным расчетом”"));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.07", 50.0f, "Гц", 0.0f, 400.0f, "Установленное значение является заданным начальным значением цифровой частоты"));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.08", 50.0f, "Гц", 0.0f, 400.0f, "Максимальная выходная частота является наивысшей частотой, разрешенной для выхода переменного привода, и эталоном для настроек ускорения и замедления."));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.09", 50.0f, "Гц", 0.0f, 400.0f, "Рабочая частота не должна превышать эту частоту"));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.10", 0.0f, "Гц", 0.0f, 400.0f, "Рабочая частота не должна быть ниже этой частоты"));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.11", 0, "", 0, 2, "0: Работает на нулевой скорости\n1: Работает на нижнем пределе частоты\n2: Остановка"));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.12", 10.0f, "с", 0.1f, 999.9f, "Время, необходимое для ускорения переменного привода от нулевой частоты до максимальной выходной частоты"));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.13", 10.0f, "с", 0.1f, 999.9f, "Время, необходимое для замедления переменного привода от максимальной выходной частоты до нулевой частоты"));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.14", 0, "", 0, 2, "0: Прямое вращение\n1: Обратное вращение\n2: Запрещено обратное вращение"));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.15", 0, "", 0, 9999, "При установке числа, отличного от 0, пароль будет работать; после расшифровки, если установлен 0000, функция пароля будет отменена."));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.16", "xx.xx", "", "01.00", "99.99", "Текущая версия программного обеспечения."));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.17", 0, "", 0, 3, "0: Без действия\n1: Восстановить заводские настройки (исключая параметры двигателя)\n2: Очистка ошибок\n3: Восстановить все параметры до заводских настроек (включая параметры двигателя)"));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.18", "", "", "", "", "Резерв"));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.19", "", "", "", "", "Резерв"));
  _allParameters[GROUP_F0].addParameter(createParameter("F0.20", 1, "", 0, 1, "0: Не сохранять\n1: Сохранять"));

  // Группа F1 - Параметры управления V/F
  _allParameters[GROUP_F1].addParameter(createParameter("F1.00", 0, "", 0, 4, "0: Линейная кривая\n1: Квадратная кривая\n2: Кривая 1,5 степени\n3: Кривая 1,2 степени\n4: Многоточечная кривая VF"));
  _allParameters[GROUP_F1].addParameter(createParameter("F1.01", 3.0f, "%", 0.0f, 30.0f, "Ручное увеличение крутящего момента, это значение устанавливается как процент относительно номинального напряжения двигателя.\nКогда оно равно 0, переключается на автоматическое увеличение крутящего момента."));
  _allParameters[GROUP_F1].addParameter(createParameter("F1.02", 15.00f, "Гц", 0.0f, 50.00f, "Частота отсечения для ручного увеличения крутящего момента"));
  _allParameters[GROUP_F1].addParameter(createParameter("F1.03", 16.0f, "КГц", 2.0f, 16.0f, "Увеличение несущей частоты может снизить шум, но увеличит тепловыделение переменного привода."));
  _allParameters[GROUP_F1].addParameter(createParameter("F1.04", 12.50f, "Гц", 0.01f, 100.0f, "Частотное значение V/F F1"));
  _allParameters[GROUP_F1].addParameter(createParameter("F1.05", 25.0f, "%", 0.0f, 100.0f, "Напряжение V/F V1"));
  _allParameters[GROUP_F1].addParameter(createParameter("F1.06", 25.00f, "Гц", 0.0f, 100.0f, "Частотное значение V/F F2"));
  _allParameters[GROUP_F1].addParameter(createParameter("F1.07", 50.0f, "%", 0.0f, 100.0f, "Напряжение V/F V2"));
  _allParameters[GROUP_F1].addParameter(createParameter("F1.08", 37.50f, "Гц", 0.0f, 100.0f, "Частотное значение V/F F3"));
  _allParameters[GROUP_F1].addParameter(createParameter("F1.09", 75.0f, "%", 0.0f, 100.0f, "Напряжение V/F V3"));
  _allParameters[GROUP_F1].addParameter(createParameter("F1.10", 0, "", 0, 2, "0: Недействительно; 1: Действительно на всем протяжении; 2: Недействительно во время замедления, действительно во время ускорения и на постоянной скорости"));
  _allParameters[GROUP_F1].addParameter(createParameter("F1.11", 0.9f, "%", 0.0f, 100.0f, "Коэффициент торможения тормозного резистора"));
  _allParameters[GROUP_F1].addParameter(createParameter("F1.12", 0.0f,    "%", 0.0f, 150.0f, "Увеличение компенсации крутящего момента"));
  _allParameters[GROUP_F1].addParameter(createParameter("F1.13", 0.84f, "%", 0.0f, 200.0f, "Увеличение возбуждения V/F"));
  _allParameters[GROUP_F1].addParameter(createParameter("F1.14", 5, "", 0, 6, "Режим подавления колебаний"));

  // Группа F2 - Параметры векторного управления
  _allParameters[GROUP_F2].addParameter(createParameter("F2.00", 20, "", 1, 100, "Kp низкоскоростного контура скорости"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.01", 0.50f, "", 1.0f, 10.0f, "Ki низкоскоростного контура скорости"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.02", 10, "", 1, 100, "Kp высокоскоростного контура скорости"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.03", 1.0f, "", 1.0f, 10.0f, "Ki высокоскоростного контура скорости"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.04", 10.0f, "Гц", 0.0f, 50.0f, "Точка переключения расчета частоты низкоскоростного контура"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.05", 30.0f, "Гц", 0.0f, 50.0f, "Точка переключения расчета частоты высокоскоростного контура"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.06", 0.0f, "%", 0.0f, 100.0f, "Компенсация электрического скольжения"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.07", "", "", "", "", "Резерв"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.08", "", "", "", "", "Резерв"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.09", "", "", "", "", "Резерв"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.10", 2000, "", 0, 60000, "Kp контура тока"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.11", 1300, "", 0, 60000, "Ki контура тока"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.12", "", "", "", "", "Резерв"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.13", "", "", "", "", "Резерв"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.14", 1, "%", 0, 200, "Коэффициент компенсации скольжения открытого векторного управления"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.15", "", "", "", "", "Резерв"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.16", "", "", "", "", "Резерв"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.17", "", "", "", "", "Резерв"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.18", "", "", "", "", "Резерв"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.19", 150.0f, "%", 0.0f, 200.0f, "Цифровая установка предела крутящего момента в контроле скорости (привод)"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.20", 1, "%", 50, 200, "Максимальный коэффициент крутящего момента зоны ослабления поля"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.21", 5, "", 5, 300, "М-осевой коэффициент масштаба контура тока"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.22", 0, "", 0, 65535, "М-осевой интегральный коэффициент контура тока"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.23", 25, "", 0, 100, "Фильтр временной константы контура скорости открытого векторного управления"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.24", 100, "", 0, 500, "Открытое векторное управление увеличения крутящего момента"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.25", 20.00f, "Гц", 0.0f, 50.0f, "Частота отсечения открытого векторного управления увеличения крутящего момента"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.26", 28, "", 0, 31, "Фильтр заданного крутящего момента"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.27", 1.05f, "%", 0.0f, 110.0f, "Максимальный коэффициент модуляции ослабления поля"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.28", 1, "%", 0, 100, "Коэффициент компенсации наблюдения потока"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.29", 300, "", 0, 2000, "Коэффициент фильтрации наблюдения потока"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.30", 0, "", 0, 500, "T-осевой коэффициент замкнутого контура тока"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.31", 0, "", 0, 1, "Метод ограничения крутящего момента"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.32", "", "", "", "", "Резерв"));
  _allParameters[GROUP_F2].addParameter(createParameter("F2.33", "", "", "", "", "Резерв"));

  // Группа F3 - Вспомогательные рабочие параметры
  _allParameters[GROUP_F3].addParameter(createParameter("F3.00", 0, "", 0, 1, "0: Запуск по стартовой частоте\n1: Запуск по стартовой частоте после торможения постоянным током"));
  _allParameters[GROUP_F3].addParameter(createParameter("F3.01", 0.50f, "Гц", 0.50f, 20.00f, "Начальная частота запуска переменного привода"));
  _allParameters[GROUP_F3].addParameter(createParameter("F3.02", 0.0f, "с", 0.0f, 60.0f, "Время работы на стартовой частоте"));
  _allParameters[GROUP_F3].addParameter(createParameter("F3.03", 0.0f, "%", 0.0f, 100.0f, "Текущая величина для применения торможения постоянным током\nКогда номинальный ток двигателя меньше или равен 80% от номинального тока переменного привода, это процентная база относительно номинального тока двигателя;\nКогда номинальный ток двигателя больше 80% от номинального тока переменного привода, это процентная база относительно 80% от номинального тока переменного привода;"));
  _allParameters[GROUP_F3].addParameter(createParameter("F3.04", 0.0f, "с", 0.0f, 60.0f, "Продолжительность применения торможения постоянным током"));
  _allParameters[GROUP_F3].addParameter(createParameter("F3.05", 0, "", 0, 2, "0: Замедление до остановки\n1: Замедление до остановки + торможение постоянным током\n2: Свободная остановка"));
  _allParameters[GROUP_F3].addParameter(createParameter("F3.06", 0.0f, "Гц", 0.0f, 50.0f, "Когда частота достигает предустановленной частоты, начинает работать торможение постоянным током"));
  _allParameters[GROUP_F3].addParameter(createParameter("F3.07", 0.0f, "%", 0.0f, 100.0f, "Текущая величина для применения торможения постоянным током такая же, как и “торможение постоянным током при запуске”"));
  _allParameters[GROUP_F3].addParameter(createParameter("F3.08", 0.0f, "с", 0.0f, 30.0f, "Продолжительность применения торможения постоянным током"));

    // Резервные параметры
    for (int i = 9; i <= 15; ++i) {
        _allParameters[GROUP_F3].addParameter(createParameter("F3." + static_cast<char>(i), "", "", "", "", "Резерв"));
  
  // Группа F4 - Вспомогательные рабочие параметры 2
  _allParameters[GROUP_F4].addParameter(createParameter("F4.00", 10.00f, "Гц", 0.00f, 50.00f, "Установка частоты джога FWD & REV"));
  _allParameters[GROUP_F4].addParameter(createParameter("F4.01", 0.00f, "Гц", 0.00f, 50.00f, "Установка частоты для REV джога"));
  _allParameters[GROUP_F4].addParameter(createParameter("F4.02", 0.1f, "с", 0.1f, 999.9f, "Установка времени ускорения и замедления джога"));
  _allParameters[GROUP_F4].addParameter(createParameter("F4.03", 0.00f, "с", 0.0f, 999.9f, "Время замедления джога"));
  _allParameters[GROUP_F4].addParameter(createParameter("F4.04", 10.0f, "с", 0.1f, 999.9f, "Время ускорения 2"));
  _allParameters[GROUP_F4].addParameter(createParameter("F4.05", 10.0f, "с", 0.1f, 999.9f, "Время замедления 2"));
  _allParameters[GROUP_F4].addParameter(createParameter("F4.06", 1, "", 0, 1, "0: Недействительно\n1: Когда переменный привод работает, JOG имеет самый высокий приоритет"));
  _allParameters[GROUP_F4].addParameter(createParameter("F4.07", 0.00f, "Гц", 0.0f, 50.0f, "Установив пропускаемую частоту и диапазон, переменный привод может избежать механической резонансной точки нагрузки."));
  _allParameters[GROUP_F4].addParameter(createParameter("F4.08", 0.00f, "Гц", 0.0f, 10.0f, "Пропускаемый диапазон"));

  // Группа F5 - Параметры цифровых входов/выходов
  _allParameters[GROUP_F5].addParameter(createParameter("F5.00", 0, "", 0, 3, "0: Двухпроводной режим управления 1\n1: Двухпроводной режим управления 2\n2: Трехпроводной режим управления 1\n3: Трехпроводной режим управления 2"));
  _allParameters[GROUP_F5].addParameter(createParameter("F5.01", 3, "", 0, 1, "0: Команда запуска терминала недействительна при включении\n1: Команда запуска терминала действительна при включении"));
  _allParameters[GROUP_F5].addParameter(createParameter("F5.02", 4, "", 0, 27, "0: Нет функции\n1: Контроль прямого джога\n2: Контроль обратного джога\n3: Контроль прямого вращения (FWD)\n4: Контроль обратного вращения (REV)\n5: Трехпроводной контроль\n6: Свободная остановка\n7: Вход внешнего сигнала остановки (STOP)\n8: Вход внешнего сигнала сброса (RST)\n9: Вход внешнего сигнала неисправности нормально открытый (NO)\n10: Команда увеличения частоты (UP)\n11: Команда уменьшения частоты (DOWN)\n12: Выбор многоскорости S1\n13: Выбор многоскорости S2\n14: Выбор многоскорости S3\n15: Канал команды запуска принудительно на терминал\n16: Резерв\n17: Команда торможения постоянным током\n18: Переключение источника частоты (F0.06)\n19: Резерв\n20: Резерв\n21: Резерв\n22: Сигнал сброса счетчика (Fb.10 функция подсчета)\n23: Сигнал триггера счетчика (Fb.10 функция подсчета)\n24: Сигнал сброса таймера (Fb.10 функция таймера)\n25: Сигнал триггера таймера (Fb.10 функция таймера)\n26: Время ускорения/замедления"));
  _allParameters[GROUP_F5].addParameter(createParameter("F5.03", 12, "", 0, 27, "Функции входного терминала X2"));
  _allParameters[GROUP_F5].addParameter(createParameter("F5.04", 0, "", 0, 27, "Функции входного терминала X3"));
  _allParameters[GROUP_F5].addParameter(createParameter("F5.05", 8, "", 0, 27, "Функции входного терминала X4 (версия связи: 485+)"));
  _allParameters[GROUP_F5].addParameter(createParameter("F5.06", 5, "", 0, 27, "Функции входного терминала X5 (версия связи: 485-)"));
  _allParameters[GROUP_F5].addParameter(createParameter("F5.07", 0, "с", 0, 14, "0: Нет функции\n1: Переменный привод готов к запуску\n2: Переменный привод работает\n3: Переменный привод работает на нулевой скорости\n4: Внешняя неисправность остановила\n5: Неисправность переменного привода\n6: Сигнал достижения частоты/скорости (FAR)\n7: Сигнал уровня частоты/скорости (FDT)\n8: Выходная частота достигает верхнего предела\n9: Выходная частота достигает нижнего предела\n10: Предупреждение о перегрузке переменного привода\n11: Сигнал переполнения таймера (выход реле, когда время таймирования достигает установленного времени в Fb.13)\n12: Сигнал обнаружения счетчика (выход реле, когда значение подсчета достигает значения, обнаруженного счетчиком в Fb.12)\n13: Сигнал сброса счетчика (резерв)\n14: Резерв"));
  _allParameters[GROUP_F5].addParameter(createParameter("F5.08", 0.0f, "с", 0.0f, 999.9f, "Задержка от изменения состояния реле R до изменения выхода"));
  _allParameters[GROUP_F5].addParameter(createParameter("F5.09", 5.00f, "Гц", 0.0f, 50.0f, "Задержка открытия R"));
  _allParameters[GROUP_F5].addParameter(createParameter("F5.10", 10.00f, "Гц", 0.00f, 15.00f, "Когда выходная частота попадает в положительную и отрицательную ширину обнаружения установленной частоты, терминал выдает действительный сигнал (низкий уровень)."));
  _allParameters[GROUP_F5].addParameter(createParameter("F5.11", 5.0f, "", 0.00f, 100.0f, "Установленное значение уровня FDT"));
  _allParameters[GROUP_F5].addParameter(createParameter("F5.16", 5, "", 0, 9999, "Коэффициент фильтрации X1"));
  _allParameters[GROUP_F5].addParameter(createParameter("F5.17", 5, "", 0, 9999, "Коэффициент фильтрации X2"));
  _allParameters[GROUP_F5].addParameter(createParameter("F5.18", 5, "", 0, 9999, "Коэффициент фильтрации X3"));
  _allParameters[GROUP_F5].addParameter(createParameter("F5.19", 5, "", 0, 9999, "Коэффициент фильтрации X4"));
  _allParameters[GROUP_F5].addParameter(createParameter("F5.20", 0, "", 0, 9999, "Коэффициент фильтрации X5"));

  // Группа F6 - Функции аналогового ввода и вывода
  _allParameters[GROUP_F6].addParameter(createParameter("F6.00", 0.0f, "%", 0.0f, 100.0f, "Установить нижний предел напряжения AVI"));
  _allParameters[GROUP_F6].addParameter(createParameter("F6.01", 100.0f, "%", 0.0f, 100.0f, "Установить верхний предел напряжения AVI"));
  _allParameters[GROUP_F6].addParameter(createParameter("F6.02", 0.0f, "%", -100.0f, 100.0f, "Установить соответствующий процент нижнего предела AVI, который соответствует проценту максимальной частоты."));
  _allParameters[GROUP_F6].addParameter(createParameter("F6.03", 100.0f, "%", -100.0f, 100.0f, "Установить соответствующий процент верхнего предела AVI, который соответствует проценту максимальной частоты."));
  _allParameters[GROUP_F6].addParameter(createParameter("F6.04", 0.0f, "%", 0.0f, 100.0f, "Установить нижний предел тока ACI"));
  _allParameters[GROUP_F6].addParameter(createParameter("F6.05", 100.0f, "%", 0.0f, 100.0f, "Установить верхний предел тока ACI"));
  _allParameters[GROUP_F6].addParameter(createParameter("F6.06", 0.0f, "%", -100.0f, 100.0f, "Установить соответствующий процент нижнего предела ACI, который соответствует проценту максимальной частоты."));
  _allParameters[GROUP_F6].addParameter(createParameter("F6.07", 100.0f, "%", -100.0f, 100.0f, "Установить соответствующий процент верхнего предела ACI, который соответствует проценту максимальной частоты."));
  _allParameters[GROUP_F6].addParameter(createParameter("F6.08", 0.1f, "с", 0.1f, 5.0f, "Этот параметр используется для фильтрации входного сигнала AVI, ACI и клавиатурного потенциометра, чтобы устранить влияние помех."));
  _allParameters[GROUP_F6].addParameter(createParameter("F6.09", 0.0f, "%", 0.0f, 100.0f, "Когда аналоговый входной сигнал часто колеблется вокруг установленного значения, установите этот параметр, чтобы подавить колебания частоты, вызванные таким колебанием."));
  _allParameters[GROUP_F6].addParameter(createParameter("F6.10", 0, "", 0, 5, "0: Выходная частота, 0~Максимальная частота\n1: Установленная частота, 0~Максимальная частота\n2: Выходной ток, 0~2 раза номинального тока\n3: Выходное напряжение, 0~2 раза номинального напряжения\n4: AVI, 0~10В\n5: ACI, 0~20мА"));
  _allParameters[GROUP_F6].addParameter(createParameter("F6.11", 0.0f, "%", 0.0f, 100.0f, "Установить нижний предел функции AO"));
  _allParameters[GROUP_F6].addParameter(createParameter("F6.12", 100.0f, "%", 0.0f, 100.0f, "Установить верхний предел функции AO"));
  _allParameters[GROUP_F6].addParameter(createParameter("F6.13", 0.0f, "%", 0.0f, 100.0f, "Установить нижний предел AO вывода"));
  _allParameters[GROUP_F6].addParameter(createParameter("F6.14", 100.0f, "%", 0.0f, 100.0f, "Установить верхний предел AO вывода"));

  // Группа F7 - Параметры запуска программы (PLC)
  _allParameters[GROUP_F7].addParameter(createParameter("F7.00", 5.00f, "Гц", 0.0f, 50.0f, "Установить частоту 1"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.01", 10.00f, "Гц", 0.0f, 50.0f, "Установить частоту 2"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.02", 15.00f, "Гц", 0.0f, 50.0f, "Установить частоту 3"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.03", 20.00f, "Гц", 0.0f, 50.0f, "Установить частоту 4"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.04", 25.00f, "Гц", 0.0f, 50.0f, "Установить частоту 5"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.05", 37.50f, "Гц", 0.0f, 50.0f, "Установить частоту 6"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.06", 50.00f, "Гц", 0.0f, 50.0f, "Установить частоту 7"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.07", 0, "", 0, 2, "0: Однократный цикл\n1: Непрерывный цикл\n2: Сохранить конечное значение после одного цикла"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.08", 0, "", 0, 1, "0: Остановка без памяти, 1: Остановка с памятью"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.09", 0, "", 0, 1, "0: Отключение без памяти, 1: Отключение с памятью"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.10", 10.0f, "с", 0.0f, 999.9f, "Установить время работы скорости 1"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.11", 10.0f, "с", 0.0f, 999.9f, "Установить время работы скорости 2"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.12", 10.0f, "с", 0.0f, 999.9f, "Установить время работы скорости 3"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.13", 10.0f, "с", 0.0f, 999.9f, "Установить время работы скорости 4"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.14", 10.0f, "с", 0.0f, 999.9f, "Установить время работы скорости 5"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.15", 10.0f, "с", 0.0f, 999.9f, "Установить время работы скорости 6"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.16", 10.0f, "с", 0.0f, 999.9f, "Установить время работы скорости 7"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.17", 0, "", 0, 3, "0: FWD, выбрать время ускорения 1\n1: FWD, выбрать время ускорения 2\n2: REV, выбрать время ускорения 1\n3: REV, выбрать время ускорения 2"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.18", 0, "", 0, 3, "Режим работы T2"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.19", 0, "", 0, 3, "Режим работы T3"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.20", 0, "", 0, 3, "Режим работы T4"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.21", 0, "", 0, 3, "Режим работы T5"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.22", 0, "", 0, 3, "Режим работы T6"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.23", 0, "", 0, 3, "Режим работы T7"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.24", "", "", "", "", "Текущий рабочий раздел (резерв)"));
  _allParameters[GROUP_F7].addParameter(createParameter("F7.25", "", "", "", "", "Текущее рабочее время (резерв)"));

  // Группа F8 - Параметры PID
  _allParameters[GROUP_F8].addParameter(createParameter("F8.00", 0, "", 0, 1, "0: Прямое действие\n1: Обратное действие"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.01", 0, "", 0, 3, "0: Цифровая установка\n1: Настройка клавиатурного потенциометра\n2: Вход AVI\n3: Вход ACI"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.02", 0, "", 0, 1, "0: Вход AVI\n1: Вход ACI"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.03", 3, "", 0, 100, "Установленное значение, когда источник заданного PID является цифровой установкой"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.04", 0.0f, "с", 0.0f, 100.0f, "Время ускорения/замедления PID"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.05", 0.0f, "%", 0.0f, 100.0f, "Установка смещения PID"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.06", 0.0f, "с", 0.0f, 6000.0f, "Время удержания смещения PID"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.07", 100.0f, "%", 0.0f, 100.0f, "Верхний предел отклонения PID"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.08", 0.0f, "%", 0.0f, 100.0f, "Нижний предел отклонения PID (Максимальная частота)"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.09", 25.00f, "", 0.0f, 600.0f, "Пропорциональный коэффициент"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.10", 1.0f, "с", 0.0f, 100.0f, "Интегральное время"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.11", 0.00f, "с", 0.0f, 10.0f, "Дифференциальное время"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.12", 100.0f, "%", 0.0f, 100.0f, "Верхний предел выхода PID"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.13", 0.0f, "%", 0.0f, 100.0f, "Нижний предел выхода PID"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.14", 0.00f, "с", 0.0f, 10.0f, "Фильтр времени выхода PID"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.15", 2, "", 0, 4, "0: Работает на верхнем пределе частоты\n1: Работает на нижнем пределе частоты\n2: Работает на частоте цифровой установки\n3: Замедление до остановки\n4: Свободная остановка"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.16", 0.0f, "%", 0.0f, 100.0f, "Значение обнаружения потери"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.17", 1.0f, "с", 0.0f, 100.0f, "Время обнаружения потери"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.18", 100.0f, "%", 0.0f, 100.0f, "Значение обнаружения избыточности"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.19", 1.0f, "с", 0.0f, 100.0f, "Время обнаружения избыточности"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.20", 0, "", 0, 2, "0: Нет функции сна\n1: Внутреннее пробуждение\n2: Управление внешним входным терминалом"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.21", 0, "", 0, 1, "0: Замедление до остановки\n2: Свободная остановка"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.22", 0.0f, "Гц", 0.0f, 50.0f, "Частота сна"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.23", 95.0f, "%", 0.0f, 100.0f, "Давление сна"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.24", 30.0f, "с", 0.0f, 6000.0f, "Время задержки сна"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.25", 80.0f, "%", 0.0f, 100.0f, "Давление пробуждения"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.26", 3.0f, "с", 0.0f, 60.0f, "Время задержки пробуждения"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.27", 0.0f, "", -3276.8f, 3276.8f, "Нижний предел диапазона PID"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.28", 10.0f, "", -3276.8f, 3276.8f, "Верхний предел диапазона PID"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.29", 1, "", 0, 3, "0: Не отображать десятичные разряды\n1: Отображать одну десятичную точку\n2: Отображать две десятичные точки\n3: Отображать три десятичные точки"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.30", 48.0f, "Гц", 0.0f, 50.0f, "Частота обнаружения нехватки воды"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.31", 0.0f, "", 0.0f, 9999.0f, "Давление обнаружения нехватки воды"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.32", 60.0f, "с", 0.0f, 6500.0f, "Время обнаружения нехватки воды"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.33", 600.0f, "с", 0.0f, 6500.0f, "Время перезапуска после нехватки воды"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.34", 6, "", 0, 9999, "Количество перезапусков после нехватки воды"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.35", "", "", "", "", "Резерв"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.36", 0, "", 0, 3, "0: Отключен\n1: Режим работы насоса PV 1\n2: Режим работы насоса PV 2"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.37", 0, "", 0, 3, "0: Отключен\n1: MPPT включен\n2: Насос PV включен\n3: MPPT и насос PV включены"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.38", 0.0f, "В", 0.0f, 1000.0f, "Максимальное рабочее напряжение MPPT"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.39", 0, "", 0, 1, "0: Включено\n1: Отключено"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.40", 0, "", 0, 1, "0: Отключено\n1: Включено"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.41", 10.0f, "с", 0.0f, 360.0f, "Задержка перезапуска при недостаточном напряжении"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.42", 0, "", 0, 1, "0: Отключено\n1: Включено"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.43", 0.0f, "%", 0.0f, 300.0f, "Соотношение тока без нагрузки, соответствующее току обнаружения нехватки воды насоса PV"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.44", 0.00f, "Гц", 0.0f, 99.99f, "Минимальная частота отлива насоса PV"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.45", 0.0f, "с", 0.0f, 250.0f, "Время обнаружения нехватки воды насоса PV"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.46", 0, "", 0, 1, "0: Отключено\n1: Включено"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.47", 0, "", 0, 1, "0: Относительно максимальной частоты\n1: Относительно центральной частоты"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.48", 0, "", 0, 1, "0: Запомнить состояние перед остановкой\n1: Перезапустить старт"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.49", 0.0f, "%", 0.0f, 100.0f, "Амплитуда колебаний"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.50", 0.0f, "%", 0.0f, 50.0f, "Шаг колебаний"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.51", 5.0f, "с", 0.1f, 400.0f, "Время нарастания колебаний"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.52", 5.0f, "с", 0.1f, 400.0f, "Время спада колебаний"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.53", 5.0f, "с", 0.1f, 999.9f, "Задержка верхней частоты"));
  _allParameters[GROUP_F8].addParameter(createParameter("F8.54", 5.0f, "с", 0.1f, 999.9f, "Задержка нижней частоты"));

  // Группа F9 - Параметры двигателя
  _allParameters[GROUP_F9].addParameter(createParameter("F9.00", "", "", "", "", "Настройка параметров двигателя"));
  _allParameters[GROUP_F9].addParameter(createParameter("F9.01", 1.0f, "В", 1.0f, 500.0f, "Настройка параметров двигателя"));
  _allParameters[GROUP_F9].addParameter(createParameter("F9.02", 0.01f, "А", 0.01f, 99.99f, ""));
  _allParameters[GROUP_F9].addParameter(createParameter("F9.03", 0.0f, "Об/мин", 0.0f, 60000.0f, ""));
  _allParameters[GROUP_F9].addParameter(createParameter("F9.04", 50.0f, "Гц", 1.0f, 400.0f, ""));
  _allParameters[GROUP_F9].addParameter(createParameter("F9.05", 0, "", 0, 1, "0: Отключить идентификацию параметров; 1: Включить статическую идентификацию параметров, автоматически устанавливается в 0 после идентификации"));
  _allParameters[GROUP_F9].addParameter(createParameter("F9.06", 0.001f, "Ω", 0.001f, 65.535f, "У разных моделей есть соответствующие заводские значения, и идентификация параметров автоматически изменит значение"));
  // Параметры F9.07, F9.08, F9.09 могут быть добавлены аналогично, если они известны
  _allParameters[GROUP_F9].addParameter(createParameter("F9.11", 0.01f, "А", 0.01f, 100.0f, "Установить ток без нагрузки двигателя; У разных моделей есть соответствующие заводские значения, и идентификация параметров автоматически изменит значение"));

  // Группа FA - Параметры защиты
  _allParameters[GROUP_FA].addParameter(createParameter("FA.00", 0, "", 0, 1, "0: Недействительно\n1: Действительно"));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.01", 1.0f, "%", 30.0f, 110.0f, "Коэффициент защиты от перегрузки двигателя - это процентное соотношение номинального тока двигателя к номинальному выходному току переменного привода."));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.02", "180/360В", "", "150-280 / 300~480В", "", "Допустимое нижнее значение напряжения на шине постоянного тока, когда переменный привод работает нормально."));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.03", 1, "", 0, 1, "0: Отключить\n1: Включить"));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.04", "375/660В", "", "350-380 / 660~760В", "", "Рабочее напряжение во время защиты от перенапряжения"));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.05", 1.5f, "%", 30.0f, 200.0f, "Порог тока автоматического ограничения тока, установленное значение является процентом относительно номинального тока переменного привода."));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.06", 0.0f, "Гц/с", 0.0f, 99.99f, "Скорость падения частоты при ограничении тока"));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.07", 0, "", 0, 2, "0: Недействительно\n1: Действительно во время ускорения/замедления, недействительно на постоянной скорости\n2: Действительно во время ускорения и замедления, действительно на постоянной скорости"));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.08", 1.2f, "%", 120.0f, 150.0f, "Текущий порог действия предварительного предупреждения о перегрузке переменного привода."));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.09", 5.0f, "с", 0.0f, 15.0f, "Время задержки от превышения уровня предварительного предупреждения о перегрузке."));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.10", 30, "", 0, 200, "Увеличьте эту настройку, когда возникают колебания двигателя."));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.11", 20, "", 0, 1000, "Установить максимальную величину корректировки для подавления колебаний."));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.12", 5.00f, "Гц", 0.0f, 200.0f, "Ниже этой частоты подавление колебаний будет неэффективным."));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.13", 50.00f, "Гц", 0.0f, 200.0f, "Выше этой частоты подавление колебаний будет неэффективным."));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.14", 11, "", 0, 111, "Выбор во время ускорения, 0: Недействительно, 1: Действительно; выбор во время замедления и на постоянной скорости."));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.15", 180.0f, "%", 80.0f, 200.0f, "Номинальный ток переменного привода."));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.16", 0, "", 0, 10, "Когда установлено в 0, автоматический сброс отключен."));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.17", 3.0f, "с", 0.5f, 25.0f, "Установить интервал автоматического сброса неисправностей."));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.18", 3, "", 0, 3, "0: Без действия\n1: Включение подавления перегрузки\n2: Включение подавления перенапряжения\n3: Включение подавления перегрузки/перенапряжения"));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.19", 20, "", 0, 100, "Подавление перегрузки VF Kp"));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.20", 50, "", 50, 200, "Коэффициент компенсации предела тока, умноженного на скорость."));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.21", 60, "", 0, 100, "Подавление перенапряжения VF Kp"));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.22", 5, "", 0, 50, "Порог частоты VF при подавлении перенапряжения."));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.23", 80, "", 0, 100, "Регулирование напряжения VF во время защиты от перенапряжения Kp."));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.24", 0, "", 0, 1, "0: Сообщить об ошибке недостаточного напряжения, свободная остановка;\n1: Не сообщать об ошибке недостаточного напряжения, остановка по установленному режиму остановки (F3.05)."));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.25", "", "", "", "", "Резерв"));
  _allParameters[GROUP_FA].addParameter(createParameter("FA.26", 1, "", 0, 1, "0: Защита от потери фазы на выходе отключена\n1: Защита от потери фазы на выходе включена"));

  // Группа Fb - Параметры отображения и специальные параметры
  _allParameters[GROUP_FB].addParameter(createParameter("Fb.00", 0, "", 0, 15, "Элементы отображения по умолчанию на главном интерфейсе мониторинга. Соответствующие номера являются параметрами группы d."));
  _allParameters[GROUP_FB].addParameter(createParameter("Fb.01", 1, "", 0, 15, "Элементы отображения по умолчанию на главном интерфейсе мониторинга. Соответствующие номера являются параметрами группы d."));
  _allParameters[GROUP_FB].addParameter(createParameter("Fb.02", 1.00f, "", 0.01f, 99.99f, "Используется для коррекции ошибки отображения шкалы скорости и не влияет на фактическую скорость."));
  _allParameters[GROUP_FB].addParameter(createParameter("Fb.03", 0, "", 0, 9999, "Код текущей ошибки"));
  _allParameters[GROUP_FB].addParameter(createParameter("Fb.04", 0, "", 0, 9999, "Код предыдущей ошибки"));
  _allParameters[GROUP_FB].addParameter(createParameter("Fb.05", 0, "", 0, 9999, "Код предыдущей ошибки два"));
  _allParameters[GROUP_FB].addParameter(createParameter("Fb.06", 0, "", 0, 9999, "Напряжение на шине при ошибке"));
  _allParameters[GROUP_FB].addParameter(createParameter("Fb.07", 0.0f, "", 0.0f, 999.9f, "Ток на шине при ошибке"));
  _allParameters[GROUP_FB].addParameter(createParameter("Fb.08", 0.0f, "", 0.0f, 300.0f, "Установленная частота при ошибке"));
  _allParameters[GROUP_FB].addParameter(createParameter("Fb.09", 0.0f, "", 0.0f, 300.0f, "Рабочая частота при ошибке"));
  _allParameters[GROUP_FB].addParameter(createParameter("Fb.10", 103, "", 0, 303, "Единицы: Обработка прихода подсчета, 0: Однократный подсчет, остановить выход; 1: Однократный подсчет, продолжить выход; 2: Циклический подсчет, остановить выход; 3: Циклический подсчет, продолжить выход. Десятки: Резерв Сотни: Обработка прихода таймирования."));
  _allParameters[GROUP_FB].addParameter(createParameter("Fb.11", 1, "", 0, 9999, "Установить значение сброса счетчика"));
  _allParameters[GROUP_FB].addParameter(createParameter("Fb.12", 1, "", 0, 9999, "Установить значение обнаружения счетчика"));
  _allParameters[GROUP_FB].addParameter(createParameter("Fb.13", 0, "с", 0, 9999, "Установить время таймирования"));
    // Резерв
    for (int i = 14; i <= 19; i++) {
        _allParameters[GROUP_FB].addParameter(createParameter("Fb." + static_cast<char>(i), "", "", "", "", "Резерв"));
    }
  _allParameters[GROUP_FB].addParameter(createParameter("Fb.20", "", "", "", "", "Дата обновления программного обеспечения (год)"));
  _allParameters[GROUP_FB].addParameter(createParameter("Fb.21", "", "", "", "", "Дата обновления программного обеспечения (месяц день)"));
  _allParameters[GROUP_FB].addParameter(createParameter("Fb.22", "1.00f", "", "", "", "Отображение версии программного обеспечения"));

  // Группа FC - Параметры связи
  _allParameters[GROUP_FC].addParameter(createParameter("FC.00", 3, "", 0, 5, "0: 1200\n1: 2400\n2: 4800\n3: 9600\n4: 19200\n5: 38400"));
  _allParameters[GROUP_FC].addParameter(createParameter("FC.01", 0, "", 0, 6, "Формат данных: <Длина данных, позиция остановки>\n0: Без проверки, <8,1>\n1: Проверка нечетности, <9,1>\n2: Проверка четности, <9,1>\n3: Без проверки, <8,1>\n4: Проверка четности, <8,1>\n5: Проверка нечетности, <8,1>\n6: Без проверки, <8,2>"));
  _allParameters[GROUP_FC].addParameter(createParameter("FC.02", 1, "", 1, 247, "1-247 представляет местный адрес"));
  _allParameters[GROUP_FC].addParameter(createParameter("FC.03", 10.0f, "с", 0.0f, 600.0f, "Тайм-аут связи"));
  _allParameters[GROUP_FC].addParameter(createParameter("FC.04", "", "", "", "", "Резерв"));
  _allParameters[GROUP_FC].addParameter(createParameter("FC.05", 1, "", 0, 2, "0: Без действия\n1: Сигнал тревоги\n2: Остановка по неисправности"));

  // Группа FP - Заводские параметры
  _allParameters[GROUP_FP].addParameter(createParameter("FP.00", "", "", "1", "9999", "Специфический пароль для настройки системы"));

  // Группа d - Параметры мониторинга
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-00", 0.00f, "Гц", 0.00f, 400.00f, ""));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-01", 0.00f, "Гц", 0.00f, 400.00f, ""));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-02", 0, "В", 0, 999, ""));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-03", 0, "В", 0, 999, ""));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-04", 0.0f, "А", 0.0f, 999.9f, ""));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-05", 0, "Об/мин", 0, 60000, ""));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-06", 0.00f, "В", 0.00f, 10.00f, ""));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-07", 0.00f, "мА", 0.00f, 20.00f, ""));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-08", 0.00f, "В", 0.00f, 10.00f, ""));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-09", 0, "", 0, 0x3F, "Состояние входного терминала (Реле, X1-X5)"));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-10", 0, "℃", 0, 9999, ""));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-11", 0.0, "", 0.0f, 9999.0f, "Заданное значение PID"));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-12", 0.0, "", 0.0f, 9999.0f, "Значение обратной связи PID"));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-13", 0, "", 0, 9999, "Текущее значение счетчика"));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-14", 0, "с", 0, 9999, "Текущее значение таймера (с)"));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-15", 0, "ч", 0, 9999, "Накопительное время работы переменного привода (ч)"));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-16", 0, "ч", 0, 9999, "Накопительное время включения переменного привода (ч)"));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-17", 0, "", 0, 4095, "Смещение выборки тока фазы U"));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-18", 0, "", 0, 4095, "Смещение выборки тока фазы V"));
  _allParameters[GROUP_COUNT-1].addParameter(createParameter("d-19", 0, "", 0, 4095, "Смещение выборки тока фазы W"));
  }
}