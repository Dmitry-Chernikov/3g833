#include "SUSWE321.h"

//#define DEBUG

SUSWE321::SUSWE321(const uint8_t slaveAddress, HardwareSerial* serialPort, HardwareSerial* serialDebug, const uint8_t transmitterModeContact):
                                                                _slaveAddress(slaveAddress),
                                                                _serialPort(serialPort),
                                                                _serialDebug(serialDebug),
                                                                _transmitterModeContact(transmitterModeContact) {
}

// Функция чтения параметров
bool SUSWE321::readParameters(const uint8_t slaveAddress, uint16_t* arrayValues, const uint16_t startAddress, const size_t numberRegisters) const {

    if (arrayValues == nullptr || numberRegisters == 0 ) {
        return false;
    }

    uint8_t request[8];
    request[0] = slaveAddress;              // Адрес устройства
    request[1] = READ;                      // Код функции для чтения
    request[2] = startAddress >> 8;         // Высокий байт адреса
    request[3] = startAddress & 0xFF;       // Низкий байт адреса
    request[4] = numberRegisters >> 8;       // Число параметров читаемых (по умолчанию 1)
    request[5] = numberRegisters & 0xFF;     // Число параметров читаемых (по умолчанию 1)

    // Вычисление и добавление CRC
    const uint16_t crc = calculateCRC(request, 6);

#ifdef DEBUG
    _serialDebug->print("Calculated CRC: 0x");
    _serialDebug->println(crc, HEX);
    _serialDebug->print("Low byte: 0x");
    _serialDebug->print(crc & 0xFF, HEX);
    _serialDebug->print(", High byte: 0x");
    _serialDebug->println((crc >> 8) & 0xFF, HEX);
#endif

    request[6] = crc & 0xFF; // Низкий байт CRC
    request[7] = (crc >> 8) & 0xFF; // Высокий байт CRC

#ifdef DEBUG
    _serialDebug->print("Запрос: ");
    //_serialDebug->write(request, sizeof(request));
    for (byte i = 0; i < sizeof(request); i++) {
        _serialDebug->print(request[i], HEX);
        _serialDebug->print(" ");
    }
    _serialDebug->println();
#endif

    // Отправка запроса
    sendData(request, sizeof(request));

    // Получение ответа
    uint8_t response[5 + (numberRegisters * 2)]; // Ожидаем 7 байт ответа если читаем один регистр, если несколько 6 + numberRegisters
    receiveData(response, sizeof(response));


    // Проверка CRC ответа
    if (response[0] != slaveAddress || response[1] != 0x03) {
#ifdef DEBUG
        if (response[0] == 0x00) {
            _serialDebug->println("Ошибка CRC");
        } else {
            _serialDebug->print("Ошибка в ответе: ");
            for (byte i = 0; i < sizeof(response); i++) {
                _serialDebug->print(response[i], HEX);
                _serialDebug->print(" ");
            }
        }
        
        _serialDebug->println("");
#endif
        return false; // Ошибка в ответе
    }

    // Извлечение значения
    if (numberRegisters == 1) {
        *arrayValues = (response[3] << 8) | response[4];
    } else if (numberRegisters > 1) {
        for (size_t i = 0; i < numberRegisters; i++) {
            arrayValues[i] = (response[3] << 8) | response[4]; // Объединение двух байтов в одно значение
        }
    }

#ifdef DEBUG
    _serialDebug->print("Ответ: ");
    for (byte i = 0; i < sizeof(response); i++) {
        _serialDebug->print(response[i], HEX);
        _serialDebug->print(" ");
    }
    _serialDebug->println();
#endif

    return true;
}

// Функция записи параметров
bool SUSWE321::writeParameters(const uint8_t slaveAddress,
                            const uint16_t startAddress,
                            const void* arrayValues,
                            const size_t numberRegisters) const {
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
        // Копируем данные с преобразованием порядка байт
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
#endif
        return false;
    }

    // Проверка ответа
    return validateModbusResponse(response, responseSize, slaveAddress, request[1]);
}

// Чтения кода неисправности частотного преобразователя
bool SUSWE321::readFaultDescription(uint16_t* faultCode) const {
    return readParameters(_slaveAddress, faultCode, 0x8000, 1);
}
// Чтения состояния частотного преобразователя в каком состоянии находится двигатель
bool SUSWE321::readRunningState(uint16_t* state) const {
    return readParameters(_slaveAddress, state, 0x3000, 1);
}

// Записи команды управления двигателем: вперёд, назад, толчок вперёд, толчок назад, свободная остановка, замедление остановки, сброс неисправности.
bool SUSWE321::writeControlCommand(const ControlCommand command) const {
    const uint16_t singleData = static_cast<uint16_t>(command);
    return writeParameters(_slaveAddress ,0x2000, &singleData, sizeof(singleData));
}

bool SUSWE321::readParameterInGroups(const GroupsParameter group, const uint8_t number, uint16_t* valueRead) const {
    return readParameters(_slaveAddress, valueRead, buildParameterAddress(group, number), 1);
}

bool SUSWE321::writeParameterInGroups(const GroupsParameter group, const uint8_t numberGroup, const uint16_t* arrayData, const size_t dataCount) const {
    return writeParameters(_slaveAddress, buildParameterAddress(group, numberGroup), arrayData, dataCount );
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
    _serialDebug->println(crc, DEC);
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
    // Переводим устройство в режим передатчика
    digitalWrite(_transmitterModeContact, RS485Transmit); 
    delay(1);  // Короткая задержка для стабилизации

    // Реализуйте отправку данных через последовательный порт
    _serialPort->write(data, length);
    _serialPort->flush();  // Ожидаем завершения передачи
    // Немедленно возвращаемся в режим приема
    digitalWrite(_transmitterModeContact, RS485Receive);
    delay(1);  // Важно! Дать линии стабилизироваться перед приёмом
}

// Реализация функций для получения данных
bool SUSWE321::receiveData(uint8_t* buffer, const size_t length) const {
    if (buffer == nullptr || length == 0) {
        return false;
    }

    size_t bytesRead = 0;
    unsigned long startTime = millis(); // Начало времени ожидания
    constexpr unsigned long TOTAL_TIMEOUT = 1000; // Общий тайм-аут 2 сек
    constexpr unsigned long INTER_CHAR_TIMEOUT = 50; // Тайм-аут между символами 50 мс

#ifdef DEBUG
    _serialDebug->print("Waiting for ");
    _serialDebug->print(length);
    _serialDebug->println(" bytes...");
#endif

    // Ждем данные с тайм-аутом
    while (bytesRead < length) {
        // Общий тайм-аут
        if (millis() - startTime > TOTAL_TIMEOUT) {
#ifdef DEBUG
            _serialDebug->print("TOTAL TIMEOUT! Received ");
            _serialDebug->print(bytesRead);
            _serialDebug->print("/");
            _serialDebug->println(length);
#endif
            break;
        }

        // Чтение доступных данных
        while (_serialPort->available() > 0 && bytesRead < length) {
            buffer[bytesRead] = _serialPort->read();
            bytesRead++;
            startTime = millis(); // Сброс таймера при получении данных
        }

        if (bytesRead < length) {
            // Проверка тайм-аута между символами
            if (_serialPort->available() == 0) {
                if (millis() - startTime > INTER_CHAR_TIMEOUT) {
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
        _serialDebug->print("SUCCESS: Received ");
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
#endif
    return (bytesRead == length);
}

bool SUSWE321::checkCommunicationSettings() const {
    uint16_t value = 0;
    bool result = false;

    // Проверка FC.00 - скорость (должен быть 3 = 9600)

    if ( (result = readParameterInGroups(GROUP_FC, 0, &value)) ) {
        _serialDebug->print("FC.00 (Baud rate): ");
        _serialDebug->println(value);
    }
    
    // Проверка FC.01 - формат данных (должен быть 0 = 8N1)
    if ( (result = readParameterInGroups(GROUP_FC, 1, &value)) ) {
        _serialDebug->print("FC.01 (Data format): ");
        _serialDebug->println(value);
    }
    
    // Проверка FC.02 - адрес (должен быть 1 или 2)
    if ( (result = readParameterInGroups(GROUP_FC, 2, &value)) ) {
        _serialDebug->print("FC.02 (Address): ");
        _serialDebug->println(value);
    }

    // Проверка FC.03 - тайм-аут связи (должен быть 10 с)
    if ( (result = readParameterInGroups(GROUP_FC, 3, &value)) ) {
        _serialDebug->print("FC.03 (Timeout Communication): ");
        _serialDebug->println(value);
    }

    // Проверка FC.05 - тип обработчика ошибки связи (должен быть 1 "бездействие")
    if ( (result = readParameterInGroups(GROUP_FC, 4, &value)) ) {
        _serialDebug->print("FC.05 (Error Communication): ");
        _serialDebug->println(value);
    }

    _serialDebug->println();

    return result;
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