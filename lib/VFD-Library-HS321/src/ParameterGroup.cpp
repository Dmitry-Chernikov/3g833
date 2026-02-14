#include "ParameterGroup.h"

/**
 * @brief Конструктор класса ParameterGroup.
 *
 * Инициализирует группу параметров с указанным именем и максимальным количеством параметров.
 * Выделяет динамически память для массива параметров.
 *
 * @param groupName Указатель на строку с названием группы (например, "Основные параметры").
 * @param maxParameters Максимальное количество параметров, которые можно добавить в группу (по умолчанию — 10).
 */
ParameterGroup::ParameterGroup(const char *groupName, const int maxParameters)
                                                        :GROUP_NAME(groupName),
                                                        MAX_PARAMETERS(maxParameters),
                                                        _parameterCount(0) {
    _parameters = new Parameter[maxParameters]; // Выделение памяти под массив параметров
}

/**
 * @brief Деструктор класса ParameterGroup.
 *
 * Освобождает динамически выделенную память, занятую массивом параметров.
 * Предотвращает утечки памяти при уничтожении объекта.
 */
ParameterGroup::~ParameterGroup() {
    delete[] _parameters; // Освобождение памяти
}

/**
 * @brief Добавляет параметр в группу.
 *
 * Копирует переданный параметр в массив группы, если есть свободное место.
 * Если количество параметров достигло лимита (MAX_PARAMETERS), выводит предупреждение в последовательный порт.
 *
 * @param param Константная ссылка на объект Parameter, который необходимо добавить в группу.
 *
 * @warning Если массив параметров заполнен, новое значение не будет добавлено,
 *          и в порт Serial будет выведено сообщение об ошибке.
 */
void ParameterGroup::addParameter(const Parameter &param) {
    if (_parameterCount < MAX_PARAMETERS) {
        _parameters[_parameterCount++] = param; // Копирование параметра в массив
    } else {
        Serial.println("Достигнуто максимальное количество параметров!"); // Вывод предупреждения
    }
}