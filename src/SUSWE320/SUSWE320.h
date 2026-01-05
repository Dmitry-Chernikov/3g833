#pragma once

#include <Arduino.h>

#define RS485Transmit HIGH
#define RS485Receive LOW

static const uint16_t crc16_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x0200, 0x1221, 0x2242, 0x3263, 0x4284, 0x52A5, 0x62C6, 0x72E7,
    0x8308, 0x9329, 0xA34A, 0xB36B, 0xC38C, 0xD3AD, 0xE3CE, 0xF3EF,
    0x0400, 0x1421, 0x2442, 0x3463, 0x4484, 0x54A5, 0x64C6, 0x74E7,
    0x8508, 0x9529, 0xA54A, 0xB56B, 0xC58C, 0xD5AD, 0xE5CE, 0xF5EF,
    0x0600, 0x1621, 0x2642, 0x3663, 0x4684, 0x56A5, 0x66C6, 0x76E7,
    0x8708, 0x9729, 0xA74A, 0xB76B, 0xC78C, 0xD7AD, 0xE7CE, 0xF7EF,
    0x0800, 0x1821, 0x2842, 0x3863, 0x4884, 0x58A5, 0x68C6, 0x78E7,
    0x8908, 0x9929, 0xA94A, 0xB96B, 0xC98C, 0xD9AD, 0xE9CE, 0xF9EF,
    0x0A00, 0x1A21, 0x2A42, 0x3A63, 0x4A84, 0x5AA5, 0x6AC6, 0x7AE7,
    0x8B08, 0x9B29, 0xAB4A, 0xBB6B, 0xCB8C, 0xDBAD, 0xEBCE, 0xFBFF,
    0x0C00, 0x1C21, 0x2C42, 0x3C63, 0x4C84, 0x5CA5, 0x6CC6, 0x7CE7,
    0x8D08, 0x9D29, 0xAD4A, 0xBD6B, 0xCD8C, 0xDDAD, 0xEDCE, 0xFDFF,
    0x0E00, 0x1E21, 0x2E42, 0x3E63, 0x4E84, 0x5EA5, 0x6EC6, 0x7EE7,
    0x8F08, 0x9F29, 0xAF4A, 0xBF6B, 0xCF8C, 0xDFAD, 0xEFC0, 0xFFE1,
};

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
constexpr float modelPowers[] = {
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
enum GroupsParameter : uint16_t{
    GROUP_F0 = 0,     // Основные рабочие параметры
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
    GROUP_d = 112,      // Параметры мониторинга
    GROUP_COUNT = GROUP_d - 97  // Количество групп
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
    const char* stringValue; // Для хранения строковых значений
};

// Структура Parameter
struct Parameter {
    const char* name;              // Название параметра
    ParameterValue factoryDefault; // Значение по умолчанию
    const char* unit;              // Единица измерения
    ParameterValue minSetting;     // Минимальное значение диапазона
    ParameterValue maxSetting;     // Максимальное значение диапазона
    const char* description;       // Описание параметра
    ParameterType type;            // Тип значения
};

class SUSWE320 {
public:
    // Конструктор принимает ссылку на объект HardwareSerial
    SUSWE320(HardwareSerial* serialPort, uint8_t transmitterModeContact);
    ~SUSWE320() = default;

    bool readParameter(uint8_t slaveAddress, uint16_t parameterAddress, uint16_t* value) const;
    bool writeParameter(uint8_t slaveAddress, uint16_t parameterAddress, uint16_t value) const;

    static uint16_t calculateCRC(const uint8_t *data, uint8_t length);

    static void generate_crc16_table();

    void sendData(const uint8_t* data, size_t length) const;
    bool receiveData(uint8_t* buffer, size_t length) const;

    // Дополнительные функции для работы с параметрами
    bool readFaultDescription(uint8_t slaveAddress, uint16_t* faultCode) const;
    bool readRunningState(uint8_t slaveAddress, uint16_t* state) const;
    bool readSoftwareVersion(uint8_t slaveAddress, uint16_t* state) const;
    bool writeControlCommand(uint8_t slaveAddress, uint16_t command) const;

    bool checkCommunicationSettings(uint8_t slaveAddress) const;

    // Реализация функций для чтения и записи параметров
    static uint16_t buildParameterAddress(GroupsParameter group, uint8_t subAddress);

private:
    HardwareSerial* _serialPort; // Указатель на объект HardwareSerial
    uint8_t _transmitterModeContact; // Номер контакта для режима работы приёмник/передатчик
};
