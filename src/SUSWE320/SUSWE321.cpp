#include "SUSWE321.h"

//#define DEBUG
//#define DEBUG_DETAILED
//#define  DEBUG_sendData

SUSWE321::SUSWE321(const uint8_t slaveAddress, HardwareSerial* serialPort, HardwareSerial* serialDebug, const unsigned long baud, const uint8_t transmitterModeContact):
                                                                _slaveAddress(slaveAddress),
                                                                _serialPort(serialPort),
                                                                _serialDebug(serialDebug),
                                                                _baud(baud),
                                                                _transmitterModeContact(transmitterModeContact) {
    TOTAL_TIMEOUT = static_cast<unsigned long>(2000);
    // Тайм-аут между байтами кадра MODBUS 3.5 символов
    // (1.0 / baud) это скорость передачи данных
    // 10 это количество бит в символе в кадре MODBUS
    // 1000000 это перевод в микросекунды
    INTER_CHAR_TIMEOUT = static_cast<unsigned long>(3.5 * 10 * 1000000 / _baud);
}

void SUSWE321::begin() const {
    _serialPort->begin(_baud);
    _serialDebug->begin(_baud, SERIAL_8N1);
}

// Функция чтения параметров
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
    const uint16_t receivedCRC = (response[responseSize - 1] << 8) | response[responseSize - 2];
    const uint16_t calculatedCRC = calculateCRC(response, responseSize - 2);

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

// Функция записи параметров
bool SUSWE321::writeParameters(const uint8_t slaveAddress,
                            const uint16_t startAddress,
                            const void* arrayValues,
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
    const auto arrayRegisterValues = static_cast<const uint16_t*>(arrayValues);

    // Вычисляем размер запроса
    const size_t requestSize = (numberRegisters == 1) ? 6 : (7 + numberRegisters * 2);

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
        request[4] = static_cast<uint8_t>(arrayRegisterValues[0] >> 8); // Данные регистра старший байт
        request[5] = static_cast<uint8_t>(arrayRegisterValues[0] & 0xFF);   // Данные регистра младший байт
    } else if (numberRegisters > 1) {
        request[1] = WRITE_RANGE;           // Код функции 0x10 для записи в диапазон регистров
        request[2] = static_cast<uint8_t>(startAddress >> 8);
        request[3] = static_cast<uint8_t>(startAddress & 0xFF);
        request[4] = static_cast<uint8_t>(numberRegisters >> 8);    // Количество регистров старший байт
        request[5] = static_cast<uint8_t>(numberRegisters & 0xFF);  // Количество регистров младший байт
        request[6] = static_cast<uint8_t>(numberRegisters * 2); // Количество байт данных
        // Копируем, данные с преобразованием порядка байт
        for (size_t i = 0; i < numberRegisters; i++) {
            request[7 + (i * 2)] = static_cast<uint8_t>(arrayRegisterValues[i] >> 8);
            request[8 + (i * 2)] = static_cast<uint8_t>(arrayRegisterValues[i] & 0xFF);
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

// Чтения кода неисправности частотного преобразователя
bool SUSWE321::readFaultDescription(uint16_t* faultCode) const {
    return readSingleParameter(0x8000, faultCode);
}

// Чтения состояния частотного преобразователя в каком состоянии находится двигатель
bool SUSWE321::readRunningState(uint16_t* state) const {
    return readParameters(_slaveAddress, 0x3000, state, 1);
}

// Записи команды управления двигателем: вперёд, назад, толчок вперёд, толчок назад,
// свободная остановка, замедление остановки, сброс неисправности.
bool SUSWE321::writeControlCommand(const ControlCommand command) const {
    return writeSingleParameter(0x2000, command);
}

bool SUSWE321::readSingleParameter(const uint16_t address, uint16_t* value) const {
    return readParameters(_slaveAddress, address, value, 1);
}

bool SUSWE321::readParameterInGroups(const GroupsParameter group, const uint8_t numberGroup, uint16_t* arrayValues, const size_t count) const {
    return readParameters(_slaveAddress,buildParameterAddress(group, numberGroup), arrayValues, count);
}

bool SUSWE321::readSingleGroupParameter(const GroupsParameter group, const uint8_t numberGroup, uint16_t* value) const {
    return readSingleParameter(buildParameterAddress(group, numberGroup), value);

}

bool SUSWE321::writeSingleParameter(const uint16_t address, const uint16_t value) const {
    return writeParameters(_slaveAddress, address, &value, 1);
}

bool SUSWE321::writeParameterInGroups(const GroupsParameter group, const uint8_t numberGroup, const uint16_t* arrayData, const size_t dataCount) const {
    return writeParameters(_slaveAddress, buildParameterAddress(group, numberGroup), arrayData, dataCount );
}

bool SUSWE321::writeSingleGroupParameter(const GroupsParameter group, const uint8_t numberGroup, const uint16_t value) const {
    return writeSingleParameter(buildParameterAddress(group, numberGroup), value);
}

// Реализация функции CRC
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

// Генерация таблицы
void SUSWE321::generate_crc16_table() {
    for (uint16_t i = 0; i < 256; i++) {
        uint16_t crc = (i << 8); // Начальное значение
        for (uint8_t j = 0; j < 8; j++) {
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
        }
        //crc16_table[i] = crc;
    }
}

// Реализация функций для отправки
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

// Реализация функций для получения данных
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
    const unsigned long charTimeout = INTER_CHAR_TIMEOUT * length * 1000; // Время ожидания между символами в мс

#ifdef DEBUG
    _serialDebug->print("Waiting for ");
    _serialDebug->print(length);
    _serialDebug->println(" bytes...");
#endif

    // Ждем данные с тайм-аутом
    while (bytesRead < length) {
        // Общий тайм-аут
        if (millis() - lastByteTime > TOTAL_TIMEOUT) {
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
#ifdef DEBUG_DETAILED
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
                    _serialDebug->println("ЛОХ ПИДОР");
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

bool SUSWE321::checkCommunicationSettings() const {
    constexpr size_t requestSize = 5;
    uint16_t arrayValues[requestSize];
    if ( readParameterInGroups(GROUP_FC, 0,  arrayValues, requestSize) ) {
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

// Реализация функций для чтения и записи параметров
uint16_t SUSWE321::buildParameterAddress(const GroupsParameter group, const uint8_t subAddress) {
    return ((static_cast<uint16_t>(group) << 8) | subAddress);
}

// Вспомогательная функция для проверки ответа
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

/* // Реализация функции отправки данных
void SUSWE321::sendData(const uint8_t* data, size_t length) {
    _serialPort->write(data, length);
    _serialPort->flush(); // Дождаться завершения передачи
}

// Реализация функции получения данных
bool SUSWE321::receiveData(uint8_t* buffer, size_t length) {
    unsigned long startMillis = millis(); // Начало времени ожидания
    while (_serialPort->available() < length) {
        // Проверка на время ожидания
        if (millis() - startMillis > 1000) { // Тайм-аут 1 секунда
            return false; // Время ожидания истекло
        }
    }
    _serialPort->readBytes(buffer, length); // Чтение данных
    // Проверка на наличие конца строки
    if (_serialPort->find('\n')) {
        return true; // Данные успешно получены
    }
    return false; // Не удалось найти конец строки
}

// Реализация функции отправки данных
void SUSWE321::sendData(const uint8_t* data, size_t length) {
    _serialPort->write(data, length);
    unsigned long startMillis = millis(); // Запоминаем время начала
    while (_serialPort->availableForWrite() < length) { // Проверяем, доступно ли место для записи
        if (millis() - startMillis > 1000) { // Тайм-аут 1 секунда
            _serialDebug->println("Ошибка: Тайм-аут при отправке данных");
            return; // Выход из функции при истечении времени
        }
    }
}

volatile bool sendingData = false;
// Реализация функции отправки данных
void SUSWE321::sendData(const uint8_t* data, size_t length) {
    sendingData = true; // Устанавливаем флаг
    _serialPort->write(data, length);
    _serialPort->flush(); // Дождаться завершения передачи
    sendingData = false; // Сбрасываем флаг
} */