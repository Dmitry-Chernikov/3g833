#include "SUSWE320.h"


SUSWE320::SUSWE320(HardwareSerial* serialPort, const uint8_t transmitterModeContact):   _serialPort(serialPort),
                                                                                        _transmitterModeContact(transmitterModeContact) {
}

SUSWE320::~SUSWE320(){
}

// Функция чтения параметров
bool SUSWE320::readParameter(const uint8_t slaveAddress, const uint16_t parameterAddress, uint16_t* value) const {
    uint8_t request[8];
    request[0] = slaveAddress;              // Адрес устройства
    request[1] = 0x0003;                    // Код функции для чтения
    request[2] = parameterAddress >> 8;     // Высокий байт адреса
    request[3] = parameterAddress & 0xFF;   // Низкий байт адреса
    request[4] = 0x0000;                    // Число параметров (1)
    request[5] = 0x0001;                    // Число параметров (1)

    // Вычисление и добавление CRC
    const uint16_t crc = calculateCRC(request, 6);
    //uint16_t crc = 0xF781;
    Serial.print("Calculated CRC: 0x");
    Serial.println(crc, HEX);
    Serial.print("Low byte: 0x");
    Serial.print(crc & 0xFF, HEX);
    Serial.print(", High byte: 0x");
    Serial.println((crc >> 8) & 0xFF, HEX);
    
    request[6] = crc & 0xFF; // Низкий байт CRC
    request[7] = (crc >> 8) & 0xFF; // Высокий байт CRC

    Serial.print("Запрос: ");
    //Serial.write(request, sizeof(request));
    for (byte i = 0; i < sizeof(request); i++) {
        Serial.print(request[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    // Отправка запроса
    sendData(request, sizeof(request));

    // Получение ответа
    uint8_t response[7]; // Ожидаем 7 байт ответа
    receiveData(response, sizeof(response));

    // Проверка CRC ответа
    if (response[0] != slaveAddress || response[1] != 0x03) {
        if (response[0] == 0x00) {
            Serial.println("Ошибка CRC");
        } else {
            Serial.print("Ошибка в ответе: ");
            for (byte i = 0; i < sizeof(response); i++) {
                Serial.print(response[i], HEX);
                Serial.print(" ");
            }
        }
        
        Serial.println("");

        return false; // Ошибка в ответе
    }

    // Извлечение значения
    *value = (response[3] << 8) | response[4]; // Объединение двух байтов в одно значение
    Serial.print("Ответ: ");
    for (byte i = 0; i < sizeof(response); i++) {
        Serial.print(response[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    return true;
}

// Функция записи параметров
bool SUSWE320::writeParameter(const uint8_t slaveAddress, const uint16_t parameterAddress, const uint16_t value) const {
    uint8_t request[8];
    request[0] = slaveAddress;              // Адрес устройства
    request[1] = 0x0006;                    // Код функции для записи
    request[2] = parameterAddress >> 8;     // Высокий байт адреса
    request[3] = parameterAddress & 0xFF;   // Низкий байт адреса
    request[4] = value >> 8;                // Высокий байт значения
    request[5] = value & 0xFF;              // Низкий байт значения
    
    // Вычисление и добавление CRC
    const uint16_t crc = calculateCRC(request, 6);
    Serial.print("Calculated CRC: 0x");
    Serial.println(crc, HEX);
    Serial.print("Low byte: 0x");
    Serial.print(crc & 0xFF, HEX);
    Serial.print(", High byte: 0x");
    Serial.println((crc >> 8) & 0xFF, HEX);

    request[6] = crc & 0xFF; // Низкий байт CRC
    request[7] = (crc >> 8) & 0xFF; // Высокий байт CRC
    
    Serial.print("Запрос: ");
    //Serial.write(request, sizeof(request));
    for (byte i = 0; i < sizeof(request); i++) {
        Serial.print(request[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    // Отправка запроса
    sendData(request, sizeof(request));

    // Получение ответа
    uint8_t response[8]; // Ожидаем 8 байт ответа
    receiveData(response, sizeof(response));
    // Проверка ответа
    if (response[0] != slaveAddress || response[1] != 0x06) {
        return false; // Ошибка в ответе
    }
    return true;
}

// Реализация функции чтения описания ошибок
bool SUSWE320::readFaultDescription(const uint8_t slaveAddress, uint16_t* faultCode) const {
    return readParameter(slaveAddress, 0x8000, faultCode);
}
// Реализация функции чтения состояния
bool SUSWE320::readRunningState(const uint8_t slaveAddress, uint16_t* state) const {
    return readParameter(slaveAddress, 0x3000, state);
}

// Чтение версии прошивки
bool SUSWE320::readSoftwareVersion(const uint8_t slaveAddress, uint16_t* state) const {
    //return readParameter(slaveAddress, 0x0003, state);
    return readParameter(slaveAddress, buildParameterAddress(GroupsParameter::GROUP_d, 02), state);
}

// Реализация функции записи команды управления
bool SUSWE320::writeControlCommand(const uint8_t slaveAddress, const uint16_t command) const {
    return writeParameter(slaveAddress, 0x2000, command);
}

// Реализация функции CRC
uint16_t SUSWE320::calculateCRC(const uint8_t *data, const uint8_t length) {
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

    Serial.print("CRC as uint16_t: 0x");
    Serial.println(crc, DEC);
    return crc;
}

// Генерация таблицы
void SUSWE320::generate_crc16_table() {
    for (uint16_t i = 0; i < 256; i++) {
        uint16_t crc = (i << 8); // Начальное значение
        for (uint8_t j = 0; j < 8; j++) {
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
        }
        //crc16_table[i] = crc;
    }
}

// Реализация функций для отправки
void SUSWE320::sendData(const uint8_t* data, const size_t length) const {
    // Переводим устройство в режим передатчика
    digitalWrite(_transmitterModeContact, RS485Transmit); 
    delay(2);  // Короткая задержка для стабилизации

    // Реализуйте отправку данных через последовательный порт
    _serialPort->write(data, length);
    _serialPort->flush();  // Ожидаем завершения передачи
    // Немедленно возвращаемся в режим приема
    digitalWrite(_transmitterModeContact, RS485Receive);
}

// Реализация функций для получения данных
bool SUSWE320::receiveData(uint8_t* buffer, const size_t length) const {
    unsigned long startMillis = millis(); // Начало времени ожидания
    size_t bytesRead = 0;

    Serial.println("Waiting for response...");

    // Ждем данные с тайм-аутом
    while (bytesRead < length) {
        const int available = _serialPort->available(); // Проверяем наличие данных
        if (available > 0) {
            Serial.print("Found ");
            Serial.print(available);
            Serial.println(" bytes in buffer");

           for (int i = 0; i < available && bytesRead < length; i++) {
                buffer[bytesRead] = _serialPort->read();
                bytesRead++;
            }
            startMillis = millis(); // Сброс таймаута
        }

        // Проверка на время ожидания
        if (millis() - startMillis > 1000) { // Таймаут 1 секунда
            Serial.println("=== TIMEOUT ===");
            break;
        }
        delay(10);
    }

    if (bytesRead > 0) {
        Serial.print("SUCCESS: Received ");
        Serial.print(bytesRead);
        Serial.print("/");
        Serial.print(length);
        Serial.print(" bytes: ");
        for (size_t i = 0; i < bytesRead; i++) {
            if (buffer[i] < 0x10) Serial.print("0");
            Serial.print(buffer[i], HEX);
            Serial.print(" ");
        }
        Serial.println("");
    } else {
        Serial.print("NO DATA RECEIVED");
        Serial.println("");
    }
    
    return (bytesRead == length);
}

bool SUSWE320::checkCommunicationSettings(const uint8_t slaveAddress) const {
    uint16_t value = 0;
    
    // Проверка FC.00 - скорость (должен быть 3 = 9600)
    if (readParameter(slaveAddress, 0x0000, &value)) {
        Serial.print("FC.00 (Baud rate): ");
        Serial.println(value);
    }
    
    // Проверка FC.01 - формат данных (должен быть 0 = 8N1)  
    if (readParameter(slaveAddress, 0x0001, &value)) {
        Serial.print("FC.01 (Data format): ");
        Serial.println(value);
    }
    
    // Проверка FC.02 - адрес (должен быть 1)
    if (readParameter(slaveAddress, 0x0002, &value)) {
        Serial.print("FC.02 (Address): ");
        Serial.println(value);
    }
    return value != 0 ? true : false;
}


// Реализация функций для чтения и записи параметров
uint16_t SUSWE320::buildParameterAddress(const GroupsParameter group, const uint8_t subAddress) {
    return ((static_cast<uint16_t>(group) << 8) | subAddress);
}

/* // Реализация функции отправки данных
void SUSWE320::sendData(const uint8_t* data, size_t length) {
    _serialPort->write(data, length);
    _serialPort->flush(); // Дождаться завершения передачи
}

// Реализация функции получения данных
bool SUSWE320::receiveData(uint8_t* buffer, size_t length) {
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
void SUSWE320::sendData(const uint8_t* data, size_t length) {
    _serialPort->write(data, length);
    unsigned long startMillis = millis(); // Запоминаем время начала
    while (_serialPort->availableForWrite() < length) { // Проверяем, доступно ли место для записи
        if (millis() - startMillis > 1000) { // Тайм-аут 1 секунда
            Serial.println("Ошибка: Тайм-аут при отправке данных");
            return; // Выход из функции при истечении времени
        }
    }
}

volatile bool sendingData = false;
// Реализация функции отправки данных
void SUSWE320::sendData(const uint8_t* data, size_t length) {
    sendingData = true; // Устанавливаем флаг
    _serialPort->write(data, length);
    _serialPort->flush(); // Дождаться завершения передачи
    sendingData = false; // Сбрасываем флаг
} */