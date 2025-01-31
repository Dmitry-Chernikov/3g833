#pragma once

#include <Arduino.h>

const float POWER_0_4KW = 0.4f;    // 0.4 кВт
const float POWER_0_75KW = 0.75f;  // 0.75 кВт
const float POWER_1_5KW = 1.5f;    // 1.5 кВт
const float POWER_2_2KW = 2.2f;    // 2.2 кВт
const float POWER_3_0KW = 3.0f;    // 3.0 кВт
const float POWER_4_0KW = 4.0f;    // 4.0 кВт
const float POWER_5_5KW = 5.5f;    // 5.5 кВт
const float POWER_7_5KW = 7.5f;    // 7.5 кВт

/**
 * @enum GroupsParameter
 * @brief Данные параметров содержат важные параметры привода переменного тока.
 */
enum GroupsParameter {
  GROUP_F0 = 0, // Основные рабочие параметры
  GROUP_F1,     // Параметры управления V/F
  GROUP_F2,     // Параметры векторного управления V
  GROUP_F3,     // Вспомогательные эксплуатационные параметры 1
  GROUP_F4,     // Вспомогательные эксплуатационные параметры 2
  GROUP_F5,     // Параметры цифровых фходов выходов
  GROUP_F6,     // Аналоговые входные и выходные функции
  GROUP_F7,     // Параметры запуска программы (ПЛК)
  GROUP_F8,     // Параметры PID регулятора
  GROUP_F9,     // Параметры мотора
  GROUP_FA,     // Параметры защиты
  GROUP_FB,     // Параметры дисплея и специальные
  GROUP_FC,     // Парамтеры комуникации RS485
  GROUP_FP,     // Заводские параметры
  GROUP_d,      // Параметры мониторинга
  GROUP_COUNT   // Количество групп
};

const int MAX_PARAMETERS = 10; // Максимальное количество параметров в группе

// Определение union для хранения различных типов значений
union ParameterValue {
    float floatValue;      // Для хранения значений с плавающей точкой
    int intValue;          // Для хранения целых значений
    const char* stringValue; // Для хранения строковых значений
};

// Cтруктура Parameter
struct Parameter {
    const char* name;               // Название параметра
    ParameterValue factoryDefault;  // Значение по умолчанию
    const char* unit;               // Единица измерения
    float minSetting;               // Минимальное значение диапазона
    float maxSetting;               // Максимальное значение диапазона
    const char* description;        // Описание параметра
};

class ParameterGroup {

  public:
    ParameterGroup(const char* groupName):groupName(groupName), parameterCount(0){}

    void addParametr(const Parameter& param);

  private:
    const char* groupName;          // Название группы параметров
    Parameter parameters[MAX_PARAMETERS]; // Массив параметров
    int parameterCount;             // Количество добавленных параметров
};

class ParametersSUSWE320 {
  public:
    ParametersSUSWE320();

  private:
    ParameterGroup allParameters[15]; // Массив групп параметров
};