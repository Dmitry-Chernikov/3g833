#include "SUSWE321.h"

/** @file SUSWE321.cpp
 * @brief Реализация методов класса SUSWE321, предоставляющего интерфейс API для взаимодействия с частотником SUSWE321.
 *
 * @author Dmitry Chernikov
 */

//#define DEBUG
//#define DEBUG_receiveData
//#define DEBUG_sendData

/**
 * @brief Конструктор класса SUSWE321.
 *
 * Инициализирует объект для взаимодействия с частотным преобразователем по протоколу Modbus RTU.
 *
 * @param slaveAddress Адрес устройства Modbus (частотника) в сети.
 * @param serialPort Указатель на объект HardwareSerial, используемый для связи с частотником.
 * @param serialDebug Указатель на объект HardwareSerial для вывода отладочной информации (может быть nullptr).
 * @param baud Скорость передачи данных (например, 9600, 19200, 115200).
 * @param transmitterModeContact Номер цифрового пина, управляющего направлением передачи RS485 (DE/RE).
 */
SUSWE321::SUSWE321(const uint8_t slaveAddress, HardwareSerial* serialPort, HardwareSerial* serialDebug, const unsigned long baud, const uint8_t transmitterModeContact)
                                                                :_slaveAddress(slaveAddress),
                                                                _serialPort(serialPort),
                                                                _serialDebug(serialDebug),
                                                                _baud(baud),
                                                                _transmitterModeContact(transmitterModeContact),
                                                                _totalTimeout(0),
                                                                _interCharTimeout(0){
}

/**
 * @brief Инициализация объекта и настройка аппаратных параметров.
 *
 * Настраивает последовательный порт, пин управления RS485 и таймауты для корректной работы с протоколом Modbus.
 * После успешной инициализации устанавливает флаг _initialized = true.
 */
void SUSWE321::begin() {
    // Проверка указателей
    if (_serialPort == nullptr) {
        if (_serialDebug != nullptr) {
            _serialDebug->println("Ошибка: _serialPort не должен быть nullptr");
        }
        _initialized = false;
        return;
    }

    if (_serialDebug == nullptr) {
        // Отладочный порт может быть nullptr — это допустимо
        // Можно работать без отладки
    } else {
        _serialDebug->begin(_baud, SERIAL_8N1);
    }

    // Инициализация порта, SERIAL_8N1 — 8 бит данных, без паритета, 1 стоп-бит по умолчанию в библиотеке Arduino
    _serialPort->begin(_baud);

    // Настройка пина режима RS485, управляющего направлением передачи данных (DE/RE) модуля RS485 на микросхеме MAX485
    pinMode(_transmitterModeContact, OUTPUT);
    // По умолчанию — приём (переводим модуль в режим приёма данных)
    digitalWrite(_transmitterModeContact, RS485Receive);

    // Вычисление тайм-аутов
    // Тайм-аут между кадрами MODBUS 2000 микросекунд (по умолчанию) 2 секунды
    _totalTimeout = static_cast<unsigned long>(2000);
    // Тайм-аут между байтами кадра MODBUS 3.5 символов
    // (1.0 / baud) это скорость передачи данных
    // 10 это количество бит в символе в кадре MODBUS
    // 1000000 это перевод в микросекунды
    _interCharTimeout = static_cast<unsigned long>(3.5 * 10 * 1000000 / _baud);

    // Установка флага инициализации и вывод отладочной информации
    _initialized = true;

    if (_serialDebug) {
        _serialDebug->println("SUSWE321: Инициализация завершена");
    }
}

/**
 * @brief Чтение одного или нескольких регистров Modbus.
 *
 * Отправляет запрос на чтение заданного количества регистров с указанного адреса.
 * Проверяет целостность ответа (адрес, функция, CRC).
 *
 * @param slaveAddress Адрес ведомого устройства Modbus.
 * @param startAddress Начальный адрес регистра для чтения.
 * @param arrayValues Указатель на массив для сохранения прочитанных значений (в Big-Endian).
 * @param numberRegisters Количество регистров для чтения (максимум — 125).
 * @return true, если чтение прошло успешно, иначе false.
 */
bool SUSWE321::readParameters(const uint8_t slaveAddress,
                            const uint16_t startAddress,
                            uint16_t* arrayValues,
                            const size_t numberRegisters) const {
#ifdef DEBUG
    _serialDebug->println("START readParameters !!!");
#endif
    // Проверки входных данных и корректности указателя на массив
    if (arrayValues == nullptr || numberRegisters == 0 ) {
        return false;
    }

    // Проверка на максимальное количество регистров (Modbus ограничение)
    if (numberRegisters > 125) {
        return false; // Modbus протокол ограничивает чтение 125 регистрами
    }

    uint8_t request[8];
    request[0] = slaveAddress;                                  // Адрес устройства
    request[1] = READ;                                          // Код функции для чтения
    request[2] = static_cast<uint8_t>(startAddress >> 8);       // Высокий байт адреса
    request[3] = static_cast<uint8_t>(startAddress & 0xFF);     // Низкий байт адреса
    request[4] = static_cast<uint8_t>(numberRegisters >> 8);    // Число параметров читаемых (по умолчанию 1)
    request[5] = static_cast<uint8_t>(numberRegisters & 0xFF);  // Число параметров читаемых (по умолчанию 1)

    // Вычисление и добавление CRC
    const uint16_t crc = calculateCRC(request, 6);
    request[6] = static_cast<uint8_t>(crc & 0xFF);          // Низкий байт CRC
    request[7] = static_cast<uint8_t>((crc >> 8) & 0xFF);   // Высокий байт CRC

#ifdef DEBUG
    _serialDebug->print("READ Request \"Запрос\": ");
    for (byte i = 0; i < sizeof(request); i++) {
        if (request[i] < 0x10) _serialDebug->print("0");
        _serialDebug->print(request[i], HEX);
        _serialDebug->print(" ");
    }
    _serialDebug->println();
#endif

    // Отправка запроса
    sendData(request, sizeof(request));

    // Расчет размера ответа
    // Ответ: [адрес][функция][байт данных][данные...][CRC]
    // байт данных = количество байт данных = numberRegisters * 2
    const size_t responseSize = 5 + (numberRegisters * 2); // 3 заголовка + данные + 2 CRC
    uint8_t response[responseSize]; // AVR поддерживает VLA (Variable Length Arrays)

    // Получение ответа
    if (!receiveData(response, responseSize)) {
#ifdef DEBUG
        _serialDebug->println("Ошибка приёма данных");
        _serialDebug->println("END readParameters !!!");
        _serialDebug->println();
#endif
        return false;
    }

#ifdef DEBUG
    _serialDebug->print("READ Response \"Ответ\": ");
    for (byte i = 0; i < responseSize; i++) {
        if (response[i] < 0x10) _serialDebug->print("0");
        _serialDebug->print(response[i], HEX);
        _serialDebug->print(" ");
    }
    _serialDebug->println();
#endif

    // Базовые проверки ответа
    if (response[0] != slaveAddress || response[1] != READ) {
#ifdef DEBUG
        _serialDebug->print("Неверный адрес или функция. Ожидалось: ");
        _serialDebug->print(slaveAddress, HEX);
        _serialDebug->print(" ");
        _serialDebug->print(READ, HEX);
        _serialDebug->print(", получено: ");
        _serialDebug->print(response[0], HEX);
        _serialDebug->print(" ");
        _serialDebug->println(response[1], HEX);
#endif
        return false;
    }

    // Проверка количества байт данных
    const uint8_t byteCount = response[2];
    if (byteCount != numberRegisters * 2) {
#ifdef DEBUG
        _serialDebug->print("Неверное количество байт данных. Ожидалось: ");
        _serialDebug->print(numberRegisters * 2);
        _serialDebug->print(", получено: ");
        _serialDebug->println(byteCount);
#endif
        return false;
    }

    // Проверка CRC ответа (исключая CRC байты)
    // Сохраняем CRC из ответа (последние 2 байта)
    const uint16_t receivedCRC = (response[responseSize - 1] << 8) | response[responseSize - 2];
    // Вычисляем CRC для ответа без CRC байтов
    const uint16_t calculatedCRC = calculateCRC(response, responseSize - 2);

    // Сравниваем CRC
    if (receivedCRC != calculatedCRC) {
#ifdef DEBUG
        _serialDebug->print("Ошибка CRC. Получено: 0x");
        _serialDebug->print(receivedCRC, HEX);
        _serialDebug->print(", рассчитано: 0x");
        _serialDebug->println(calculatedCRC, HEX);
#endif
        return false;
    }

    // Извлечение значений из ответа
    if (numberRegisters == 1) {
        // Для одного регистра
        arrayValues[0] = (static_cast<uint16_t>(response[3]) << 8) | response[4];
    } else {
        // Для нескольких регистров
        for (size_t i = 0; i < numberRegisters; i++) {
            const size_t dataIndex = 3 + (i * 2); // 3 - начало данных
            arrayValues[i] = (static_cast<uint16_t>(response[dataIndex]) << 8) | response[dataIndex + 1];
        }
    }

#ifdef DEBUG
    _serialDebug->print("Прочитано значений: ");
    for (size_t i = 0; i < numberRegisters; i++) {
        _serialDebug->print(arrayValues[i]);
        if (i < numberRegisters - 1) _serialDebug->print(", ");
    }
    _serialDebug->println();
    _serialDebug->println("END readParameters !!!");
    _serialDebug->println();
#endif

    return true;
}

/**
 * @brief Запись одного или нескольких регистров Modbus.
 *
 * Отправляет запрос на запись значений в регистры по заданному адресу.
 * Поддерживает функции Modbus 0x06 (один регистр) и 0x10 (диапазон регистров).
 *
 * @param slaveAddress Адрес ведомого устройства Modbus.
 * @param startAddress Начальный адрес регистра для записи.
 * @param arrayValues Указатель на массив значений, которые нужно записать.
 * @param numberRegisters Количество записываемых регистров (максимум — 123).
 * @return true, если запись прошла успешно, иначе false.
 */
bool SUSWE321::writeParameters(const uint8_t slaveAddress,
                            const uint16_t startAddress,
                            const uint16_t* arrayValues,
                            const size_t numberRegisters) const {

#ifdef DEBUG
    _serialDebug->println("START writeParameters !!!");
#endif

    // Проверка входных данных
    if (arrayValues == nullptr || numberRegisters == 0 ) {
#ifdef DEBUG
        _serialDebug->println("Ошибка: неверные входные данные");
#endif
        return false;
    }

    // Ограничение максимального количества регистров (Modbus ограничение)
    constexpr size_t MAX_MODBUS_REGISTERS = 123;
    if (numberRegisters > MAX_MODBUS_REGISTERS) {
#ifdef DEBUG
        _serialDebug->print("Ошибка: слишком много регистров: ");
        _serialDebug->println(numberRegisters);
#endif
        return false;
    }

    // Приводим void* к uint16_t* для работы с данными
    // const auto arrayRegisterValues = static_cast<const uint16_t*>(arrayValues);

    // Вычисляем размер запроса
    const size_t requestSize = (numberRegisters == 1) ? 8 : (9 + numberRegisters * 2);

    // Используем статический буфер с максимальным размером (более безопасно)
    constexpr size_t MAX_REQUEST_SIZE = 7 + (MAX_MODBUS_REGISTERS * 2);
    if (requestSize > MAX_REQUEST_SIZE) {
        return false;
    }

    uint8_t request[requestSize];  // VLA (Variable Length Array) - работает в GCC/Clang

    // Заполняем заголовок
    request[0] = slaveAddress;              // Адрес устройства

    if (numberRegisters == 1) {
        request[1] = WRITE_ONE;             // Код функции 0x06 для записи в один регистр
        request[2] = static_cast<uint8_t>(startAddress >> 8);   // Высокий байт адреса
        request[3] = static_cast<uint8_t>(startAddress & 0xFF); // Низкий байт адреса
        request[4] = static_cast<uint8_t>(arrayValues[0] >> 8); // Данные регистра старший байт
        request[5] = static_cast<uint8_t>(arrayValues[0] & 0xFF);   // Данные регистра младший байт
    } else if (numberRegisters > 1) {
        request[1] = WRITE_RANGE;           // Код функции 0x10 для записи в диапазон регистров
        request[2] = static_cast<uint8_t>(startAddress >> 8);
        request[3] = static_cast<uint8_t>(startAddress & 0xFF);
        request[4] = static_cast<uint8_t>(numberRegisters >> 8);    // Количество регистров старший байт
        request[5] = static_cast<uint8_t>(numberRegisters & 0xFF);  // Количество регистров младший байт
        request[6] = static_cast<uint8_t>(numberRegisters * 2); // Количество байт данных
        // Копируем, данные с преобразованием порядка байт
        for (size_t i = 0; i < numberRegisters; i++) {
            request[7 + (i * 2)] = static_cast<uint8_t>(arrayValues[i] >> 8);
            request[8 + (i * 2)] = static_cast<uint8_t>(arrayValues[i] & 0xFF);
        }
    }

    // Вычисление CRC (все байты, кроме последних 2, которые для CRC)
    const uint16_t crc = calculateCRC(request, requestSize - 2);
    // Добавляем CRC в конец запроса
    request[requestSize - 2] = static_cast<uint8_t>(crc & 0xFF); // Низкий байт CRC
    request[requestSize - 1] = static_cast<uint8_t>((crc >> 8) & 0xFF); // Высокий байт CRC

#ifdef DEBUG
    _serialDebug->print("Запрос Modbus (");
    _serialDebug->print(requestSize);
    _serialDebug->print(" байт): ");
    for (size_t i = 0; i < requestSize; i++) {
        if (request[i] < 0x10) _serialDebug->print("0");
        _serialDebug->print(request[i], HEX);
        _serialDebug->print(" ");
    }
    _serialDebug->println();
#endif

    // Отправка запроса
    sendData(request, requestSize);

    // Получение ответа
    // Размер ответа зависит от функции:
    // Для 0x06: 8 байт
    // Для 0x10: 8 байт
    constexpr size_t responseSize = 8;  // Modbus ответ всегда 8 байт для этих функций
    uint8_t response[responseSize];

    if (!receiveData(response, responseSize)) {
#ifdef DEBUG
        _serialDebug->println("Ошибка приема ответа");
        _serialDebug->println("END writeParameters !!!");
        _serialDebug->println();
        _serialDebug->println();
#endif
        return false;
    }
#ifdef DEBUG
    _serialDebug->println("END writeParameters !!!");
    _serialDebug->println();
    _serialDebug->println();
#endif
    // Проверка ответа
    return validateModbusResponse(response, responseSize, slaveAddress, request[1]);
}

/**
 * @brief Проверка корректности ответа Modbus.
 *
 * Проверяет, соответствует ли ответ ожидаемому:
 * - правильный адрес устройства,
 * - отсутствие кода исключения,
 * - корректный код функции,
 * - правильная контрольная сумма CRC.
 *
 * @param response Указатель на буфер с принятым ответом.
 * @param responseSize Размер ответа в байтах.
 * @param expectedAddress Ожидаемый адрес ведомого устройства.
 * @param expectedFunction Ожидаемый код функции.
 * @return true, если ответ корректен, иначе false.
 */
bool SUSWE321::validateModbusResponse(const uint8_t* response,
                                      const size_t responseSize,
                                      const uint8_t expectedAddress,
                                      const uint8_t expectedFunction) const {
    if (responseSize < 4) {  // Минимум: адрес + функция + CRC
        return false;
    }

    // Проверка адреса устройства
    if (response[0] != expectedAddress) {
#ifdef DEBUG
        _serialDebug->print("Неверный адрес в ответе: 0x");
        _serialDebug->print(response[0], HEX);
        _serialDebug->print(", ожидалось: 0x");
        _serialDebug->println(expectedAddress, HEX);
#endif
        return false;
    }

    // Проверка на исключение
    if (response[1] == (expectedFunction | 0x80)) {
#ifdef DEBUG
        _serialDebug->print("Исключение Modbus. Код ошибки: 0x");
        _serialDebug->println(response[2], HEX);
#endif
        return false;
    }

    // Проверка кода функции
    if (response[1] != expectedFunction) {
#ifdef DEBUG
        _serialDebug->print("Неверная функция в ответе: 0x");
        _serialDebug->print(response[1], HEX);
        _serialDebug->print(", ожидалось: 0x");
        _serialDebug->println(expectedFunction, HEX);
#endif
        return false;
    }

    // Проверка CRC
    const uint16_t calculatedCRC = calculateCRC(response, responseSize - 2);
    const uint16_t receivedCRC = static_cast<uint16_t>(response[responseSize - 1] << 8) | response[responseSize - 2];

    if (calculatedCRC != receivedCRC) {
#ifdef DEBUG
        _serialDebug->print("Ошибка CRC. Вычислено: 0x");
        _serialDebug->print(calculatedCRC, HEX);
        _serialDebug->print(", получено: 0x");
        _serialDebug->println(receivedCRC, HEX);
#endif
        return false;
    }

    return true;
}

/**
 * @brief Вычисление контрольной суммы CRC16-Modbus.
 *
 * Использует полином 0xA001 (обратный к 0x8005) для проверки целостности данных.
 *
 * @param data Указатель на массив данных.
 * @param length Длина данных в байтах.
 * @return Вычисленное значение CRC16.
 */
uint16_t SUSWE321::calculateCRC(const uint8_t *data, const uint8_t length) const {
    // Более строгая проверка
    if (data == nullptr || length == 0) {
        return 0xFFFF; // или другое значение ошибки
    }

    // Дополнительная проверка, что указатель валидный
    // (если есть возможность проверить диапазон адресов)

    uint16_t crc = 0xFFFF; // начальное значение CRC
    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001; // полином
            } else {
                crc = crc >> 1;
            }
        }
    }

#ifdef DEBUG
    _serialDebug->print("CRC as uint16_t: 0x");
    _serialDebug->println(crc, HEX);
#endif

    return crc;
}

/**
 * @brief Отправка данных через интерфейс RS485.
 *
 * Переводит RS485-трансивер в режим передачи, отправляет данные,
 * после чего немедленно возвращает его в режим приёма.
 *
 * @param data Указатель на массив данных для отправки.
 * @param length Количество байт для передачи.
 */
void SUSWE321::sendData(const uint8_t* data, const size_t length) const {
#ifdef DEBUG_sendData
    _serialDebug->println("\t START sendData !!!");
#endif

    // Переводим устройство в режим передатчика
    digitalWrite(_transmitterModeContact, RS485Transmit);
    //delay(1);  // Короткая задержка для стабилизации

        // Реализуйте отправку данных через последовательный порт
        _serialPort->write(data, length);
        _serialPort->flush();  // Ожидаем завершения передачи

    // Немедленно возвращаемся в режим приема
    digitalWrite(_transmitterModeContact, RS485Receive);
    //delay(1);  // Важно! Дать линии стабилизироваться перед приёмом

#ifdef DEBUG_sendData
    _serialDebug->println("\t END sendData !!!");
#endif
}

/**
 * @brief Приём данных по протоколу Modbus с тайм-аутами.
 *
 * Использует два тайм-аута:
 * - общий (TOTAL_TIMEOUT) — на весь пакет,
 * - межсимвольный (INTER_CHAR_TIMEOUT) — между байтами.
 *
 * @param buffer Буфер для сохранения принятых данных.
 * @param length Ожидаемое количество байт.
 * @return true, если все данные получены, иначе false.
 */
bool SUSWE321::receiveData(uint8_t* buffer, const size_t length) const {
#ifdef DEBUG
    _serialDebug->println("\t START receiveData !!!");
#endif

    if (buffer == nullptr || length == 0) {
#ifdef DEBUG
        _serialDebug->println("Ошибка: неверные входные данные");
        _serialDebug->println("\t END receiveData !!!");
#endif
        return false;
    }

    size_t bytesRead = 0;
    unsigned long lastByteTime = millis(); // Начало времени ожидания
    const unsigned long charTimeout = static_cast<unsigned long>(ceil(_interCharTimeout * length / 1000)); // Время ожидания между символами в мс

#ifdef DEBUG
    _serialDebug->print("Waiting for ");
    _serialDebug->print(length);
    _serialDebug->println(" bytes...");
#endif

    // Ждем данные с тайм-аутом
    while (bytesRead < length) {
        // Общий тайм-аут
        if (millis() - lastByteTime > _totalTimeout) {
#ifdef DEBUG
            _serialDebug->print("TOTAL TIMEOUT! Received ");
            _serialDebug->print(bytesRead);
            _serialDebug->print("/");
            _serialDebug->println(length);
#endif
            break;
        }

        // Чтение доступных данных
        while(_serialPort->available() > 0 && bytesRead < length) {
            buffer[bytesRead] = _serialPort->read();
            bytesRead++;
            lastByteTime = millis(); // Сброс таймера при получении данных
#ifdef DEBUG_receiveData
            _serialDebug->print("Got byte ");
            _serialDebug->print(bytesRead);
            _serialDebug->print(": 0x");
            if (buffer[bytesRead-1] < 0x10) _serialDebug->print("0");
            _serialDebug->print(buffer[bytesRead-1], HEX);
            _serialDebug->println();
#endif
        }


        // Проверка тайм-аута между символами только если нет доступных данных
        if (bytesRead < length) {
            if (_serialPort->available() == 0) {
                if (millis() - lastByteTime > charTimeout) {
#ifdef DEBUG
                    _serialDebug->print("INTER-CHAR TIMEOUT! Received ");
                    _serialDebug->print(bytesRead);
                    _serialDebug->print("/");
                    _serialDebug->println(length);
#endif
                    break;
                }
            }
        }
    }

#ifdef DEBUG
    if (bytesRead > 0) {
        _serialDebug->print("Received ");
        _serialDebug->print(bytesRead);
        _serialDebug->print("/");
        _serialDebug->print(length);
        _serialDebug->print(" bytes: ");
        for (size_t i = 0; i < bytesRead; i++) {
            if (buffer[i] < 0x10) _serialDebug->print("0");
            _serialDebug->print(buffer[i], HEX);
            _serialDebug->print(" ");
        }
        _serialDebug->println("");
    } else {
        _serialDebug->print("NO DATA RECEIVED");
        _serialDebug->println("");
    }

    _serialDebug->println("\t END receiveData !!!");
#endif

    return (bytesRead == length);
}


/**
 * @brief Чтение кода текущей ошибки частотного преобразователя.
 *
 * Обращается к регистру 0x8000, где хранится код последней неисправности.
 *
 * @param faultCode Указатель на переменную для сохранения кода ошибки.
 * @return true при успехе, иначе false.
 */
bool SUSWE321::readFaultDescription(uint16_t* faultCode) const {
    if (!isInitialized() || faultCode == nullptr) {
        return false;
    }
    return readSingleParameter(0x8000, faultCode);
}

/**
 * @brief Чтение состояния двигателя (вращается / остановлен).
 *
 * Читает регистр 0x3000, содержащий текущее состояние работы.
 *
 * @param state Указатель на переменную для сохранения состояния.
 * @return true при успехе, иначе false.
 */
bool SUSWE321::readRunningState(uint16_t* state) const {
    if (!isInitialized() || state == nullptr) {
        return false;
    }
    return readParameters(_slaveAddress, 0x3000, state, 1);
}

/**
 * @brief Отправка команды управления двигателем.
 *
 * Передаёт одну из команд (пуск вперёд, стоп, сброс аварии и т.д.) в регистр 0x2000.
 *
 * @param command Команда из перечисления ControlCommand.
 * @return true при успехе, иначе false.
 */
bool SUSWE321::writeControlCommand(const ControlCommand command) const {
    if (!isInitialized()) {
        return false;
    }
    return writeSingleParameter(0x2000,  static_cast<uint16_t>(command));
}


/**
 * @brief Чтение одного регистра Modbus.
 *
 * Упрощённая обёртка для readParameters при чтении одного значения.
 *
 * @param address Адрес регистра.
 * @param value Указатель на переменную для сохранения значения.
 * @return true при успехе, иначе false.
 */
bool SUSWE321::readSingleParameter(const uint16_t address, uint16_t* value) const {
    return readParameters(_slaveAddress, address, value, 1);
}

/**
 * @brief Чтение нескольких параметров из указанной группы.
 *
 * Вычисляет начальный адрес по группе и номеру, затем читает заданное количество регистров.
 *
 * @param group Группа параметров (например, GROUP_F1).
 * @param numberGroup Номер начального параметра в группе.
 * @param arrayValues Буфер для сохранения значений.
 * @param count Количество регистров для чтения.
 * @return true при успехе, иначе false.
 */
bool SUSWE321::readParametersInGroups(const GroupsParameter group, const uint8_t numberGroup, uint16_t* arrayValues, const size_t count) const {
    if (!isInitialized() || arrayValues == nullptr || count == 0) {
        return false;
    }
    const uint16_t startAddress = buildParameterAddress(group, numberGroup);
    return readParameters(_slaveAddress, startAddress, arrayValues, count);
}

/**
 * @brief Чтение одного параметра из указанной группы.
 *
 * Обёртка для чтения одного регистра по группе и номеру.
 *
 * @param group Группа параметров.
 * @param numberGroup Номер параметра.
 * @param value Указатель на переменную для сохранения значения.
 * @return true при успехе, иначе false.
 */
bool SUSWE321::readSingleGroupParameter(const GroupsParameter group, const uint8_t numberGroup, uint16_t* value) const {
    if (!isInitialized() || value == nullptr) {
        return false;
    }
    const uint16_t address = buildParameterAddress(group, numberGroup);
    return readSingleParameter(address, value);
}


/**
 * @brief Запись значения в один регистр Modbus.
 *
 * Упрощённая функция для записи одного значения.
 *
 * @param address Адрес регистра.
 * @param value Значение для записи.
 * @return true при успехе, иначе false.
 */
bool SUSWE321::writeSingleParameter(const uint16_t address, const uint16_t value) const {
    return writeParameters(_slaveAddress, address, &value, 1);
}

/**
 * @brief Запись нескольких значений в параметры указанной группы.
 *
 * Вычисляет начальный адрес и выполняет запись массива значений.
 *
 * @param group Группа параметров.
 * @param numberGroup Начальный номер параметра.
 * @param arrayData Массив значений для записи.
 * @param dataCount Количество значений.
 * @return true при успехе, иначе false.
 */
bool SUSWE321::writeParametersInGroups(const GroupsParameter group, const uint8_t numberGroup, const uint16_t* arrayData, const size_t dataCount) const {
    if (!isInitialized() || arrayData == nullptr || dataCount == 0) {
        return false;
    }
    uint16_t startAddress = buildParameterAddress(group, numberGroup);
    return writeParameters(_slaveAddress, startAddress, arrayData, dataCount );
}

/**
 * @brief Запись значения в один параметр группы.
 *
 * Удобная обёртка для записи одного значения по группе и номеру.
 *
 * @param group Группа параметров.
 * @param numberGroup Номер параметра.
 * @param value Значение для записи.
 * @return true при успехе, иначе false.
 */
bool SUSWE321::writeSingleGroupParameter(const GroupsParameter group, const uint8_t numberGroup, const uint16_t value) const {
    if (!isInitialized()) {
        return false;
    }
    uint16_t address = buildParameterAddress(group, numberGroup);
    return writeSingleParameter(address, value);
}

/**
 * @brief Проверка текущих настроек связи с частотником.
 *
 * Читает ключевые параметры из группы FC (настройки RS485) и выводит их в отладочный порт.
 * Используется для диагностики подключения.
 *
 * @return true, если чтение прошло успешно, иначе false.
 */
bool SUSWE321::checkCommunicationSettings() const {
    if (!isInitialized()) {
        return false;
    }
    constexpr size_t requestSize = 5;
    uint16_t arrayValues[requestSize];
    if ( readParametersInGroups(GROUP_FC, 0,  arrayValues, requestSize) ) {
        // Проверка FC.00 - скорость (должен быть 3 = 9600)
        _serialDebug->print("FC.00 (Baud rate): ");
        _serialDebug->println(arrayValues [0]);

        // Проверка FC.01 - формат данных (должен быть 0 = 8N1)
        _serialDebug->print("FC.01 (Data format): ");
        _serialDebug->println(arrayValues [1]);

        // Проверка FC.02 - адрес (должен быть 1 или 2)
        _serialDebug->print("FC.02 (Address): ");
        _serialDebug->println(arrayValues[2]);

        // Проверка FC.03 - тайм-аут связи (должен быть 10 с)
        _serialDebug->print("FC.03 (Timeout Communication): ");
        _serialDebug->println(arrayValues[3]);

        // Проверка FC.05 - тип обработчика ошибки связи (должен быть 1 "бездействие")
        _serialDebug->print("FC.05 (Error Communication): ");
        _serialDebug->println(arrayValues[4]);
        return true;
    }
    return false;
}