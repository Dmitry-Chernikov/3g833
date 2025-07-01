#pragma once

#include <Arduino.h>

#define RS485Transmit HIGH
#define RS485Receive LOW

// Перечисление моделей
enum class Model {
    MODEL_0_4,
    MODEL_0_75,
    MODEL_1_5,
    MODEL_2_2,
    MODEL_3_0,
    MODEL_4_0,
    MODEL_5_5,
    MODEL_7_5,
    MODEL_11_0,
    MODEL_COUNT // Используем для определения размера массива
};

// Массив мощностей для каждой модели
const float modelPowers[] = {
    0.4,  // MODEL_0_4
    0.75, // MODEL_0_75
    1.5,  // MODEL_1_5
    2.2,  // MODEL_2_2
    3.0,  // MODEL_3_0
    4.0,  // MODEL_4_0
    5.5,  // MODEL_5_5
    7.5,  // MODEL_7_5
    11.0  // MODEL_11_0
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
    GROUP_F5,     // Параметры цифровых входов выходов
    GROUP_F6,     // Аналоговые входные и выходные функции
    GROUP_F7,     // Параметры запуска программы (ПЛК)
    GROUP_F8,     // Параметры PID регулятора
    GROUP_F9,     // Параметры мотора
    GROUP_FA,     // Параметры защиты
    GROUP_FB,     // Параметры дисплея и специальные
    GROUP_FC,     // Параметры коммуникации RS485
    GROUP_FP,     // Заводские параметры
    GROUP_d,      // Параметры мониторинга
    GROUP_COUNT   // Количество групп
};

// Структура задаёт каркас для хранения информации об ошибках 
struct FaultInfo {
    //int code;             // Код ошибки будет равен индексу массива
    const char* name;       // Название ошибки
    const char* causes;     // Возможные причины
    const char* solution;   // Решение
};

// Определение перечисления для типов значений
enum ParameterType {
    FLOAT,
    INT,
    STRING
};

// Определение union для хранения различных типов значений
union ParameterValue {
    float floatValue;        // Для хранения значений с плавающей точкой
    int intValue;            // Для хранения целых значений
    const char *stringValue; // Для хранения строковых значений
};

// Структура Parameter
struct Parameter {
    const char *name;              // Название параметра
    ParameterValue factoryDefault; // Значение по умолчанию
    const char *unit;              // Единица измерения
    ParameterValue minSetting;     // Минимальное значение диапазона
    ParameterValue maxSetting;     // Максимальное значение диапазона
    const char *description;       // Описание параметра  
    ParameterType type;            // Тип значения    
};

class SUSWE320 {
public:
    // Конструктор принимает ссылку на объект HardwareSerial
    SUSWE320(HardwareSerial* serialPort, uint8_t transmitterModeContact);
    ~SUSWE320();

    bool readParameter(uint8_t slaveAddress, uint16_t parameterAddress, uint16_t* value);
    bool writeParameter(uint8_t slaveAddress, uint16_t parameterAddress, uint16_t value);

    unsigned int crc_chk_value(unsigned char *data_value, unsigned char length);

    void sendData(const uint8_t* data, size_t length);
    bool receiveData(uint8_t* buffer, size_t length);

    // Дополнительные функции для работы с параметрами
    bool readFaultDescription(uint8_t slaveAddress, uint16_t* faultCode);
    bool readRunningState(uint8_t slaveAddress, uint16_t* state);
    bool writeControlCommand(uint8_t slaveAddress, uint16_t command);

private:
    HardwareSerial* _serialPort; // Указатель на объект HardwareSerial
    uint8_t _transmitterModeContact; // Номер контакта для режима работы приёмник/передатчик
};
