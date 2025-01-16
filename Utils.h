////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
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
