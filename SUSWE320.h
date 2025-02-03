#pragma once

#include <Arduino.h>

enum class Model {
    MODEL_0_4S1_220V,
    MODEL_0_75S1_220V,
    MODEL_1_5S1_220V,
    MODEL_2_2S1_220V,
    MODEL_3_0S1_220V,
    MODEL_4_0S1_220V,
    MODEL_5_5S1_220V,
    MODEL_0_4S3_220V,
    MODEL_0_75S3_220V,
    MODEL_1_5S3_220V,
    MODEL_2_2S3_220V,
    MODEL_3_0S3_220V,
    MODEL_4_0S3_220V,
    MODEL_5_5S3_220V,
    MODEL_0_4G3_380V,
    MODEL_0_75G3_380V,
    MODEL_1_5G3_380V,
    MODEL_2_2G3_380V,
    MODEL_3_0G3_380V,
    MODEL_4_0G3_380V,
    MODEL_5_5G3_380V,
    MODEL_7_5G3_380V,
    MODEL_11_0G3_380V,
    MODEL_COUNT // Используем для определения размера массива
};

// Массив мощностей для каждой модели
const float modelPowers[] = {
    0.4,   // MODEL_0_4S1_220V
    0.75,  // MODEL_0_75S1_220V
    1.5,   // MODEL_1_5S1_220V
    2.2,   // MODEL_2_2S1_220V
    3.0,   // MODEL_3_0S1_220V
    4.0,   // MODEL_4_0S1_220V
    5.5,   // MODEL_5_5S1_220V
    0.4,   // MODEL_0_4S3_220V
    0.75,  // MODEL_0_75S3_220V
    1.5,   // MODEL_1_5S3_220V
    2.2,   // MODEL_2_2S3_220V
    3.0,   // MODEL_3_0S3_220V
    4.0,   // MODEL_4_0S3_220V
    5.5,   // MODEL_5_5S3_220V
    0.4,   // MODEL_0_4G3_380V
    0.75,  // MODEL_0_75G3_380V
    1.5,   // MODEL_1_5G3_380V
    2.2,   // MODEL_2_2G3_380V
    3.0,   // MODEL_3_0G3_380V
    4.0,   // MODEL_4_0G3_380V
    5.5,   // MODEL_5_5G3_380V
    7.5,   // MODEL_7_5G3_380V
    11.0   // MODEL_11_0G3_380V
};

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
    ParametersSUSWE320(Model model);

  private:
    ParameterGroup allParameters[15]; // Массив групп параметров  
    Model model; // Хранит значение модели
    float getPower(Model model); // Функция для получения мощности модели
};