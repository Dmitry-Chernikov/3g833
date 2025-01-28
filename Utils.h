uint16_t crc_chk_value(uint8_t *data_value, uint8_t length) {
    uint16_t crc_value = 0xFFFF;
    int i;
    while (length--) {
        crc_value ^= *data_value++;
        for (i = 0; i < 8; i++) {  // Изменено на i < 8
            if (crc_value & 0x0001) {
                crc_value = (crc_value >> 1) ^ 0xA001;
            } else {
                crc_value >>= 1;
            }
        }
    }
    return crc_value;
}

uint16_t crc16(uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF; // Начальное значение
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i]; // XOR с текущим байтом
        for (uint8_t j = 0; j < 8; j++) { // Обработка каждого бита
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001; // Сдвиг и XOR с полиномом
            } else {
                crc >>= 1; // Просто сдвиг
            }
        }
    }
    return crc; // Возврат результата
}

class SPI_Command_Frame_t {

    public:
        typedef union {
            typedef struct __attribute__ ((__packed__)) {
                uint8_t AddrModbus:1;       //Адрес ведомого устройства
                uint8_t CodeCommand:3;      //Код команды CMD
                uint16_t AddrParam:1;       //Адрес параметра H и Адрес параметра L
                uint16_t DataParam:1;       //Адрес параметра H и Адрес параметра L
                uint16_t CRC:1;       //Адрес параметра H и Адрес параметра L
            } SPI_Command_Frame_values_t;

            uint16_t raw = 0;                       ///< Register values (RAW).
            SPI_Command_Frame_values_t values;      ///< Register values.

        } SPI_Command_Frame_data_t;

        SPI_Command_Frame_data_t data;              ///< The actual data of a "SPI Command Frame".

        SPI_Command_Frame_t(uint16_t raw);

        SPI_Command_Frame_t(uint16_t ADDR, uint16_t RW);

};



#include <iostream>
#include <cstdint>
#include <array>


const int MODBUS_READ                   = 0x0003;
const int MODBUS_WRITE                  = 0x0006;
const int MODBUS_GROUP_WRITE_CONTINIUOU = 0x0001;

/**
 * @enum ParameterGroup
 * @brief Данные параметров содержат важные параметры привода переменного тока.
 */
enum ParameterGroup {
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
    GROUP_COUNT // Количество групп
};

/**
 * @enum Non-ParameterGroup
 * @brief Данные о состоянии делятся на параметры контроля группы d, описание неисправности привода переменного тока, состояние работы привода переменного тока.
 */
enum ParameterGroup {
    GROUP_Diagnostic = 70,  // Параметры мониторинга
    GROUP_FaultCode = 80,   // Коды ошибок
    GROUP_WarningCode,      // Коды предупреждающий
    GROUP_COUNT             // Количество групп
};



/**
 * @class FrequencyConverterParameter
 * @brief Представляет параметр частотного преобразователя.
 */
class FrequencyConverterParameter {
public:
    uint16_t address; // Адрес параметра
    uint16_t value;   // Значение параметра

    FrequencyConverterParameter(uint16_t addr, uint16_t val) : address(addr), value(val) {}

    void print() const {
        std::cout << "Address: " << address << ", Value: " << value << std::endl;
    }
};

/**
 * @class FrequencyConverter
 * @brief Представляет частотный преобразователь с набором параметров.
 */
class FrequencyConverter {
public:
    std::array<std::array<FrequencyConverterParameter, 16>, GROUP_COUNT> parameters; // Массив параметров

    // Конструктор для инициализации параметров
    FrequencyConverter() {
        for (int group = 0; group < GROUP_COUNT; ++group) {
            for (int param = 0; param < 16; ++param) {
                uint16_t address = (group << 8) | param; // Формирование адреса
                parameters[group][param] = FrequencyConverterParameter(address, 0); // Инициализация значений
            }
        }
    }

    // Метод для установки параметра
    void setParameter(ParameterGroup group, int paramIndex, uint16_t value) {
        if (group >= 0 && group < GROUP_COUNT && paramIndex >= 0 && paramIndex < 16) {
            parameters[group][paramIndex].value = value;
        }
    }

    // Метод для отображения всех параметров
    void printParameters() const {
        for (int group = 0; group < GROUP_COUNT; ++group) {
            for (int param = 0; param < 16; ++param) {
                std::cout << "Group " << group << ", ";
                parameters[group][param].print();
            }
        }
    }
};

// Пример использования
int main() {
    FrequencyConverter converter;

    converter.setParameter(GROUP_F1, 5, 123); // Установка значения для группы F1, параметр 5
    converter.setParameter(GROUP_F2, 3, 456); // Установка значения для группы F2, параметр 3

    converter.printParameters();

    return 0;
}



????????????????????????
enum ModbusFunctionCode {
    READ_COILS = 0x01,
    READ_DISCRETE_INPUTS = 0x02,
    READ_HOLDING_REGISTERS = 0x03,
    READ_INPUT_REGISTERS = 0x04,
    WRITE_SINGLE_COIL = 0x05,
    WRITE_SINGLE_REGISTER = 0x06,
    WRITE_MULTIPLE_COILS = 0x0F,
    WRITE_MULTIPLE_REGISTERS = 0x10,
    READ_WRITE_REGISTERS = 0x17,
    READ_DEVICE_IDENTIFICATION = 0x14
};

// Пример использования
ModbusFunctionCode functionCode = READ_COILS;

????????????????????????
struct ModbusFunction {
    uint8_t code;
    const char* description;
};

const ModbusFunction functionCodes[] = {
    { 0x01, "Читать состояния дискретных выходов" },
    { 0x02, "Читать состояния дискретных входов" },
    { 0x03, "Читать регистры хранения" },
    // и так далее...
};

// Пример использования
uint8_t functionCode = functionCodes[0].code; // 0x01

????????????????????????
+----------------+----------------+------------------+------------------+------------------+------------------+
| Адрес устройства| Код функции    | Данные           | CRC              |
+----------------+----------------+------------------+------------------+------------------+
| 1 байт         | 1 байт         | N байт           | 2 байта          |
+----------------+----------------+------------------+------------------+------------------+

- **Адрес устройства (1 байт)**: Адрес устройства (от 1 до 247), к которому направлено сообщение. 
- **Код функции (1 байт)**: Код функции, определяющий тип операции (например, чтение, запись и т.д.). 
- **Данные (N байт)**: Данные, передаваемые в зависимости от кода функции. Это может быть, например, адрес регистра и количество регистров для чтения. 
- **CRC (2 байта)**: Контрольная сумма (Cyclic Redundancy Check) для проверки целостности данных. 

Пример пакета Modbus RTU для чтения 2 регистров:
+----------------+-----------------+------------------+------------------+
| 0x01          | 0x03           | 0x00 0x00 0x00 0x02 | CRC             |
+----------------+-----------------+------------------+------------------+

????????????????????????
Структура для представления регистра
#include <cstdint>
#include <string>

enum class DataType {
    INT16,
    UINT16,
    FLOAT32,
    // Добавьте другие типы по мере необходимости
};

struct ModbusRegister {
    uint16_t address; // Адрес регистра
    DataType type;    // Тип данных
    std::string description; // Описание регистра (по желанию)
    uint16_t size;    // Количество регистров, необходимых для хранения данных

    ModbusRegister(uint16_t addr, DataType t, uint16_t s, const std::string& desc)
        : address(addr), type(t), size(s), description(desc) {}
};

Контейнер для хранения регистров
#include <vector>

class ModbusRegisterMap {
public:
    void addRegister(const ModbusRegister& reg) {
        registers.push_back(reg);
    }

    const ModbusRegister* getRegister(uint16_t address) const {
        for (const auto& reg : registers) {
            if (reg.address == address) {
                return &reg;
            }
        }
        return nullptr; // Регистры не найдены
    }

private:
    std::vector<ModbusRegister> registers; // Вектор для хранения регистров
};

Пример использования
#include <iostream>

int main() {
    ModbusRegisterMap registerMap;

    // Добавление регистров
    registerMap.addRegister(ModbusRegister(0x0001, DataType::INT16, 1, "Температура"));
    registerMap.addRegister(ModbusRegister(0x0002, DataType::FLOAT32, 2, "Давление"));

    // Получение регистра по адресу
    const ModbusRegister* reg = registerMap.getRegister(0x0001);
    if (reg) {
        std::cout << "Адрес: " << reg->address << ", Тип: " << static_cast<int>(reg->type)
                  << ", Размер: " << reg->size << ", Описание: " << reg->description << std::endl;
    } else {
        std::cout << "Регистры не найдены!" << std::endl;
    }

    return 0;
}

????????????????????????
int16_t value = 25; // Пример значения
Serial.print(value); // Отправит значение в текстовом формате
Serial.println(value); // Отправит значение и добавит перевод строки

float floatValue = 25.5; // Пример значения
Serial.print(floatValue, 2); // Отправит значение с двумя знаками после запятой
Serial.println(floatValue, 2); // Отправит значение с переводом строки

uint8_t buffer[] = {0x01, 0x02, 0x03}; // Пример массива байтов
Serial.write(buffer, sizeof(buffer)); // Отправит массив байтов

#include <Arduino.h>

enum class DataType {
    INT16,
    UINT16,
    FLOAT32,
};

struct ModbusRegister {
    uint16_t address; // Адрес регистра
    DataType type;    // Тип данных
    uint16_t size;    // Количество регистров, необходимых для хранения данных
    const char* description; // Описание регистра
    union {
        int16_t intValue; // Для INT16
        uint16_t uintValue; // Для UINT16
        float floatValue; // Для FLOAT32
    };
};

void setup() {
    Serial.begin(9600);

    // Пример создания регистра
    ModbusRegister reg;
    reg.address = 0x0001;
    reg.type = DataType::INT16;
    reg.size = 1;
    reg.description = "Температура";
    reg.intValue = 25; // Пример значения

    // Отправка данных в Serial порт
    switch (reg.type) {
        case DataType::INT16:
            Serial.println(reg.intValue); // Отправка целого значения
            break;
        case DataType::FLOAT32:
            Serial.println(reg.floatValue, 2); // Отправка числа с плавающей запятой
            break;
        // Добавьте другие типы по мере необходимости
        default:
            break;
    }
}

void loop() {
    // Ваш код
}

????????????????????????
#include <Arduino.h>

enum class DataType {
    INT16,
    FLOAT32,
};

struct ModbusRegister {
    uint16_t address; // Адрес регистра
    DataType type;    // Тип данных
    uint16_t size;    // Количество регистров, необходимых для хранения данных
    const char* description; // Описание регистра
    union {
        int16_t intValue; // Для INT16
        float floatValue; // Для FLOAT32
    };
};

// Функция для вычисления CRC
uint16_t calculateCRC(uint8_t* buffer, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= buffer[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void setup() {
    Serial.begin(9600);

    // Пример создания регистра
    ModbusRegister reg;
    reg.address = 0x0001; // Адрес регистра
    reg.type = DataType::INT16; // Тип данных
    reg.size = 1; // Размер в регистрах
    reg.description = "Температура";
    reg.intValue = 25; // Пример значения

    // Подготовка сообщения
    uint8_t message[256]; // Буфер для сообщения
    size_t index = 0;

    // Адрес устройства (например, 1)
    message[index++] = 0x01; // Адрес устройства

    // Код функции (например, 0x03 для чтения регистров)
    message[index++] = 0x03;

    // Адрес регистра
    message[index++] = (reg.address >> 8) & 0xFF; // Старший байт адреса
    message[index++] = reg.address & 0xFF;        // Младший байт адреса

    // Количество регистров
    message[index++] = (reg.size >> 8) & 0xFF; // Старший байт размера
    message[index++] = reg.size & 0xFF;        // Младший байт размера

    // Вычисление CRC
    uint16_t crc = calculateCRC(message, index);
    message[index++] = crc & 0xFF;        // Младший байт CRC
    message[index++] = (crc >> 8) & 0xFF; // Старший байт CRC

    // Отправка сообщения
    Serial.write(message, index); // Отправка всего сообщения
    Serial.println(); // Перевод строки
}

void loop() {
    // Ваш код
}


????????????????????????
#include <Arduino.h>

#define SLAVE_ADDRESS 0x01 // Адрес ведомого устройства
#define READ_FUNCTION_CODE 0x03 // Код функции для чтения
#define WRITE_FUNCTION_CODE 0x06 // Код функции для записи

// Функция для вычисления CRC
uint16_t calculateCRC(uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// Функция для отправки команды чтения параметра
void sendReadCommand(uint16_t parameterAddress) {
    uint8_t message[8]; // Буфер для сообщения
    size_t index = 0;

    // Заполнение сообщения
    message[index++] = SLAVE_ADDRESS; // Адрес ведомого устройства
    message[index++] = READ_FUNCTION_CODE; // Код функции
    message[index++] = (parameterAddress >> 8) & 0xFF; // Старший байт адреса параметра
    message[index++] = parameterAddress & 0xFF; // Младший байт адреса параметра
    message[index++] = 0x00; // Количество слов (1)
    message[index++] = 0x01; // Количество слов (1)

    // Вычисление и добавление CRC
    uint16_t crc = calculateCRC(message, index);
    message[index++] = crc & 0xFF; // Младший байт CRC
    message[index++] = (crc >> 8) & 0xFF; // Старший байт CRC

    // Отправка сообщения
    Serial.write(message, index);
    Serial.println(); // Перевод строки для удобства
}

// Функция для отправки команды записи параметра
void sendWriteCommand(uint16_t parameterAddress, uint16_t value) {
    uint8_t message[8]; // Буфер для сообщения
    size_t index = 0;

    // Заполнение сообщения
    message[index++] = SLAVE_ADDRESS; // Адрес ведомого устройства
    message[index++] = WRITE_FUNCTION_CODE; // Код функции
    message[index++] = (parameterAddress >> 8) & 0xFF; // Старший байт адреса параметра
    message[index++] = parameterAddress & 0xFF; // Младший байт адреса параметра
    message[index++] = (value >> 8) & 0xFF; // Старший байт значения
    message[index++] = value & 0xFF; // Младший байт значения

    // Вычисление и добавление CRC
    uint16_t crc = calculateCRC(message, index);
    message[index++] = crc & 0xFF; // Младший байт CRC
    message[index++] = (crc >> 8) & 0xFF; // Старший байт CRC

    // Отправка сообщения
    Serial.write(message, index);
    Serial.println(); // Перевод строки для удобства
}

void setup() {
    Serial.begin(9600); // Настройка скорости последовательного порта
    delay(1000); // Задержка для стабилизации связи

    // Пример чтения параметра d-01 (установленная частота)
    sendReadCommand(0x7001); // Адрес для чтения параметра d-01

    // Пример записи параметра F0-08 (максимальная частота) на 20.00 Гц
    sendWriteCommand(0x0008, 0x07D0); // 20.00 Гц в шестнадцатеричном формате
}

void loop() {
    // Здесь можно добавить код для обработки ответов от устройства
}