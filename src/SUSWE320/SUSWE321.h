#pragma once

/** @file SUSWE321.h
 * @brief Заголовочный файл для библиотеки (API) управления частотным преобразователем SUSWE321.
 *
 * @author Dmitry Chernikov
 */

#include <Arduino.h>

/**
 * @def RS485Transmit
 * @brief Уровень сигнала для режима передачи данных по RS485 (высокий уровень).
 */
#define RS485Transmit HIGH

/**
 * @def RS485Receive
 * @brief Уровень сигнала для режима приёма данных по RS485 (низкий уровень).
 */
#define RS485Receive LOW

/**
 * @enum Model
 * @brief Перечисление доступных моделей частотного преобразователя.
 *
 * Каждый элемент соответствует определённой модели по мощности.
 * MODEL_COUNT — служебный элемент для определения количества моделей.
 */
enum class Model {
    MODEL_0_4,     ///< Модель 0.4 кВт
    MODEL_0_75,    ///< Модель 0.75 кВт
    MODEL_1_5,     ///< Модель 1.5 кВт
    MODEL_2_2,     ///< Модель 2.2 кВт
    MODEL_3_0,     ///< Модель 3.0 кВт
    MODEL_4_0,     ///< Модель 4.0 кВт
    MODEL_5_5,     ///< Модель 5.5 кВт
    MODEL_7_5,     ///< Модель 7.5 кВт
    MODEL_11_0,    ///< Модель 11.0 кВт
    MODEL_COUNT    ///< Количество моделей (используется для размера массива)
};

/**
 * @var modelPowers[]
 * @brief Массив мощностей в ваттах, соответствующих каждой модели.
 *
 * Порядок значений соответствует порядку в enum Model.
 * Используется для получения мощности модели по её индексу.
 */
constexpr int modelPowers[] = {
    400,   ///< MODEL_0_4
    750,   ///< MODEL_0_75
    1500,  ///< MODEL_1_5
    2200,  ///< MODEL_2_2
    3000,  ///< MODEL_3_0
    4000,  ///< MODEL_4_0
    5500,  ///< MODEL_5_5
    7500,  ///< MODEL_7_5
    11000  ///< MODEL_11_0
};

/**
 * @enum GroupsParameter
 * @brief Группы параметров частотного преобразователя SUSWE321.
 *
 * Каждая группа объединяет параметры по функциональному назначению.
 * Адресация параметров производится по формуле: (GROUP << 8) | subAddress.
 */
enum GroupsParameter : uint16_t {
    GROUP_F0 = 0,     ///< Основные рабочие параметры
    GROUP_F1,         ///< Параметры управления V/F
    GROUP_F2,         ///< Параметры векторного управления V
    GROUP_F3,         ///< Вспомогательные эксплуатационные параметры 1
    GROUP_F4,         ///< Вспомогательные эксплуатационные параметры 2
    GROUP_F5,         ///< Параметры цифровых входов/выходов
    GROUP_F6,         ///< Аналоговые входные и выходные функции
    GROUP_F7,         ///< Параметры запуска программы (ПЛК)
    GROUP_F8,         ///< Параметры PID-регулятора
    GROUP_F9,         ///< Параметры электродвигателя
    GROUP_FA,         ///< Параметры защиты
    GROUP_FB,         ///< Параметры дисплея и специальные функции
    GROUP_FC,         ///< Параметры связи по RS485
    GROUP_FP,         ///< Заводские параметры
    GROUP_d = 112,    ///< Параметры мониторинга (данные в реальном времени)
    GROUP_COUNT = GROUP_d - 97  ///< Общее количество групп параметров
};

/**
 * @enum ControlCommand
 * @brief Команды управления работой двигателя.
 *
 * Используются для отправки управляющих команд частотнику.
 */
enum ControlCommand : uint16_t {
    FORWARD_RUN_COMMAND,           ///< Запуск двигателя вперёд
    REVERSE_RUN_COMMAND,           ///< Запуск двигателя назад
    FORWARD_RUN_JOG_COMMAND,       ///< Импульсный запуск вперёд
    REVERSE_RUN_JOG_COMMAND,       ///< Импульсный запуск назад
    FREE_STOP_COMMAND,             ///< Свободная остановка
    DECELERATE_STOP_COMMAND,       ///< Остановка с торможением
    FAULT_RESET_COMMAND            ///< Сброс аварии
};

/**
 * @struct FaultInfo
 * @brief Структура для хранения информации об ошибках частотного преобразователя.
 *
 * Содержит описание, причины и рекомендуемые действия для устранения ошибки.
 */
struct FaultInfo {
    const char* name;       ///< Название ошибки (например, "Перегрузка по току")
    const char* causes;     ///< Возможные причины возникновения ошибки
    const char* solution;   ///< Рекомендованные действия по устранению
};

/**
 * @enum ParameterType
 * @brief Тип данных параметра.
 *
 * Используется для определения типа значения в структуре Parameter.
 */
enum ParameterType {
    FLOAT,   ///< Значение типа float
    INT,     ///< Значение типа int
    STRING   ///< Значение типа строка (const char*)
};

/**
 * @union ParameterValue
 * @brief Объединение для хранения значений разных типов.
 *
 * Позволяет хранить значения параметров разного типа в единой структуре.
 * Активный тип должен отслеживаться отдельно.
 */
union ParameterValue {
    float floatValue;        ///< Хранение значения с плавающей точкой
    int intValue;            ///< Хранение целочисленного значения
    const char* stringValue; ///< Хранение строкового значения
};

/**
 * @struct Parameter
 * @brief Описание параметра частотного преобразователя.
 *
 * Содержит полную метаинформацию о параметре: название, диапазон, единицы измерения и т.д.
 */
struct Parameter {
    const char* name;              ///< Название параметра (например, "Частота задания")
    ParameterValue factoryDefault; ///< Значение по умолчанию
    const char* unit;              ///< Единица измерения (например, "Гц", "В", "%")
    ParameterValue minSetting;     ///< Минимально допустимое значение
    ParameterValue maxSetting;     ///< Максимально допустимое значение
    const char* description;       ///< Подробное описание параметра
    ParameterType type;            ///< Тип данных значения
};

/**
 * @class SUSWE321
 * @brief Класс для управления частотным преобразователем SUSWE321 по протоколу Modbus RTU.
 *
 * Реализует взаимодействие с частотником через интерфейс RS485.
 * Поддерживает чтение/запись параметров, отправку команд управления и диагностику.
 */
class SUSWE321 {
public:
    /**
     * @brief Конструктор класса.
     * @param slaveAddress Адрес ведомого устройства (частотника) в сети Modbus.
     * @param serialPort Указатель на объект HardwareSerial для связи с частотником.
     * @param serialDebug Указатель на объект HardwareSerial для вывода отладочной информации.
     * @param baud Скорость передачи данных (обычно 9600, 19200, 115200).
     * @param transmitterModeContact Номер цифрового пина для управления направлением RS485 (DE/RE).
     */
    SUSWE321(uint8_t slaveAddress, HardwareSerial* serialPort, HardwareSerial* serialDebug, unsigned long baud, uint8_t transmitterModeContact);

    /**
     * @brief Инициализация класса и настройка аппаратных параметров.
     *
     * Настройка последовательного порта, пина управления RS485 и таймаутов.
     */
    void begin();

    /**
     * @brief Деструктор по умолчанию.
     */
    ~SUSWE321() = default;

    /**
     * @brief Проверяет, была ли успешная инициализация класса.
     * @return true, если инициализация прошла успешно, иначе false.
     */
    bool isInitialized() const { return _initialized; }

    /**
     * @brief Чтение кода текущей ошибки.
     * @param faultCode Указатель на переменную для записи кода ошибки.
     * @return true в случае успеха, иначе false.
     */
    bool readFaultDescription(uint16_t* faultCode) const;

    /**
     * @brief Чтение текущего состояния двигателя (работает/остановлен).
     * @param state Указатель на переменную для записи состояния.
     * @return true в случае успеха, иначе false.
     */
    bool readRunningState(uint16_t* state) const;

    /**
     * @brief Отправка команды управления двигателем.
     * @param command Команда из перечисления ControlCommand.
     * @return true в случае успеха, иначе false.
     */
    bool writeControlCommand(ControlCommand command) const;

    /**
     * @brief Чтение одного параметра из указанной группы.
     * @param group Группа параметра.
     * @param numberGroup Номер параметра в группе.
     * @param value Указатель на переменную для записи значения.
     * @return true в случае успеха, иначе false.
     */
    bool readSingleGroupParameter(GroupsParameter group, uint8_t numberGroup, uint16_t* value) const;

    /**
     * @brief Чтение нескольких параметров из одной группы.
     * @param group Группа параметров.
     * @param numberGroup Начальный номер параметра.
     * @param arrayValues Массив для записи значений.
     * @param count Количество параметров для чтения.
     * @return true в случае успеха, иначе false.
     */
    bool readParametersInGroups(GroupsParameter group, uint8_t numberGroup, uint16_t* arrayValues, size_t count) const;

    /**
     * @brief Запись значения в один параметр указанной группы.
     * @param group Группа параметра.
     * @param numberGroup Номер параметра в группе.
     * @param value Значение для записи.
     * @return true в случае успеха, иначе false.
     */
    bool writeSingleGroupParameter(GroupsParameter group, uint8_t numberGroup, uint16_t value) const;

    /**
     * @brief Запись нескольких значений в параметры одной группы.
     * @param group Группа параметров.
     * @param numberGroup Начальный номер параметра.
     * @param arrayData Массив значений для записи.
     * @param dataCount Количество записываемых значений.
     * @return true в случае успеха, иначе false.
     */
    bool writeParametersInGroups(GroupsParameter group, uint8_t numberGroup, const uint16_t* arrayData, size_t dataCount) const;

    /**
     * @brief Проверка корректности настроек связи.
     * @return true, если связь работает, иначе false.
     */
    bool checkCommunicationSettings() const;

private:
    bool _initialized = false;               ///< Флаг успешной инициализации
    uint8_t _slaveAddress;                   ///< Адрес Modbus-устройства
    HardwareSerial* _serialPort;             ///< Порт для связи с частотником
    HardwareSerial* _serialDebug;            ///< Порт для отладочного вывода
    unsigned long _baud;                     ///< Скорость передачи данных
    uint8_t _transmitterModeContact;         ///< Пин управления направлением RS485
    unsigned long _totalTimeout;             ///< Общий таймаут ожидания ответа (мс)
    unsigned long _interCharTimeout;         ///< Таймаут между символами (мс)

    /**
     * @enum CodeFunction
     * @brief Коды функций Modbus, используемые в классе.
     */
    enum CodeFunction : uint8_t {
        READ = 0x03,       ///< Функция Modbus 0x03 — чтение регистров
        WRITE_ONE = 0x06,  ///< Функция Modbus 0x06 — запись одного регистра
        WRITE_RANGE = 0x10 ///< Функция Modbus 0x10 — запись нескольких регистров
    };

    /**
     * @brief Построение полного адреса параметра.
     * @param group Группа параметра.
     * @param subAddress Порядковый номер параметра в группе.
     * @return Полный 16-битный адрес регистра Modbus.
     */
    static constexpr uint16_t buildParameterAddress(const GroupsParameter group, const uint8_t subAddress) {
        return ((static_cast<uint16_t>(group) << 8) | subAddress);
    }

    /**
     * @brief Чтение одного регистра Modbus.
     * @param address Адрес регистра.
     * @param value Указатель на переменную для записи значения.
     * @return true при успехе, иначе false.
     */
    bool readSingleParameter(uint16_t address, uint16_t* value) const;

    /**
     * @brief Запись одного регистра Modbus.
     * @param address Адрес регистра.
     * @param value Значение для записи.
     * @return true при успехе, иначе false.
     */
    bool writeSingleParameter(uint16_t address, uint16_t value) const;

    /**
     * @brief Чтение нескольких регистров Modbus.
     * @param slaveAddress Адрес ведомого.
     * @param startAddress Адрес первого регистра.
     * @param arrayValues Массив для хранения прочитанных значений.
     * @param numberRegisters Количество регистров.
     * @return true при успехе, иначе false.
     */
    bool readParameters(uint8_t slaveAddress, uint16_t startAddress, uint16_t* arrayValues, size_t numberRegisters = 1) const;

    /**
     * @brief Запись нескольких регистров Modbus.
     * @param slaveAddress Адрес ведомого.
     * @param startAddress Адрес первого регистра.
     * @param arrayValues Массив значений для записи.
     * @param numberRegisters Количество регистров.
     * @return true при успехе, иначе false.
     */
    bool writeParameters(uint8_t slaveAddress, uint16_t startAddress, const uint16_t* arrayValues, size_t numberRegisters) const;

    /**
     * @brief Проверка корректности ответа Modbus.
     * @param response Указатель на буфер с ответом.
     * @param responseSize Размер ответа.
     * @param expectedAddress Ожидаемый адрес устройства.
     * @param expectedFunction Ожидаемая функция.
     * @return true, если ответ корректен, иначе false.
     */
    bool validateModbusResponse(const uint8_t* response, size_t responseSize, uint8_t expectedAddress, uint8_t expectedFunction) const;

    /**
     * @brief Вычисление CRC16 для пакета Modbus.
     * @param data Указатель на данные.
     * @param length Длина данных.
     * @return Рассчитанное значение CRC16.
     */
    uint16_t calculateCRC(const uint8_t* data, uint8_t length) const;

    /**
     * @brief Отправка данных через последовательный порт.
     * @param data Указатель на массив данных.
     * @param length Количество байт для отправки.
     */
    void sendData(const uint8_t* data, size_t length) const;

    /**
     * @brief Приём данных с ожиданием.
     * @param buffer Буфер для записи принятых данных.
     * @param length Ожидаемое количество байт.
     * @return true при успешном приёме, иначе false.
     */
    bool receiveData(uint8_t* buffer, size_t length) const;
};