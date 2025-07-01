#include "SUSWE320.h"


SUSWE320::SUSWE320(HardwareSerial* serialPort, uint8_t transmitterModeContact): _serialPort(serialPort), 
                                                                                _transmitterModeContact(transmitterModeContact) {
}

SUSWE320::~SUSWE320(){
}

// Функция чтения параметров
bool SUSWE320::readParameter(uint8_t slaveAddress, uint16_t parameterAddress, uint16_t* value) {
    uint8_t request[8];
    request[0] = slaveAddress; // Адрес устройства
    request[1] = 0x03; // Код функции для чтения
    request[2] = parameterAddress >> 8; // Высокий байт адреса
    request[3] = parameterAddress & 0xFF; // Низкий байт адреса
    request[4] = 0x00; // Число параметров (1)
    request[5] = 0x01; // Число параметров (1)    
    // Вычисление и добавление CRC
    unsigned int crc = crc_chk_value(request, 6);
    request[6] = crc & 0xFF; // Низкий байт CRC
    request[7] = (crc >> 8) & 0xFF; // Высокий байт CRC
    // Отправка запроса
    sendData(request, sizeof(request));
    // Получение ответа
    uint8_t response[7]; // Ожидаем 7 байт ответа
    receiveData(response, sizeof(response));
    // Проверка CRC ответа
    if (response[0] != slaveAddress || response[1] != 0x03) {
        return false; // Ошибка в ответе
    }
    // Извлечение значения
    *value = (response[3] << 8) | response[4]; // Объединение двух байтов в одно значение
    return true;
}

// Функция записи параметров
bool SUSWE320::writeParameter(uint8_t slaveAddress, uint16_t parameterAddress, uint16_t value) {
    uint8_t request[8];
    request[0] = slaveAddress; // Адрес устройства
    request[1] = 0x06; // Код функции для записи
    request[2] = parameterAddress >> 8; // Высокий байт адреса
    request[3] = parameterAddress & 0xFF; // Низкий байт адреса
    request[4] = value >> 8; // Высокий байт значения
    request[5] = value & 0xFF; // Низкий байт значения
    // Вычисление и добавление CRC
    unsigned int crc = crc_chk_value(request, 6);
    request[6] = crc & 0xFF; // Низкий байт CRC
    request[7] = (crc >> 8) & 0xFF; // Высокий байт CRC
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
bool SUSWE320::readFaultDescription(uint8_t slaveAddress, uint16_t* faultCode) {
    return readParameter(slaveAddress, 0x8000, faultCode);
}
// Реализация функции чтения состояния
bool SUSWE320::readRunningState(uint8_t slaveAddress, uint16_t* state) {
    return readParameter(slaveAddress, 0x3000, state);
}
// Реализация функции записи команды управления
bool SUSWE320::writeControlCommand(uint8_t slaveAddress, uint16_t command) {
    return writeParameter(slaveAddress, 0x2000, command);
}

// Реализация функции CRC
unsigned int SUSWE320::crc_chk_value(unsigned char *data_value, unsigned char length) {
    unsigned int crc_value = 0xFFFF;
    int i;
    while (length--) {
        crc_value ^= *data_value++;
        for (i = 0; i < 8; i++) {
            if (crc_value & 0x0001) {
                crc_value = (crc_value >> 1) ^ 0xA001;
            } else {
                crc_value = crc_value >> 1;
            }
        }
    }
    return crc_value;
}

// Реализация функций для отправки
void SUSWE320::sendData(const uint8_t* data, size_t length) {
    // Переводим устройство в режим передатчика
    digitalWrite(_transmitterModeContact, RS485Transmit); 

    // Реализуйте отправку данных через последовательный порт
    _serialPort->write(data, length);
}

// Реализация функций для получения данных
bool SUSWE320::receiveData(uint8_t* buffer, size_t length) {
    // Переводим устройство в режим приёмника
    digitalWrite(_transmitterModeContact, RS485Receive);
    unsigned long startMillis = millis(); // Начало времени ожидания
    while (_serialPort->available() < length) {
        // Проверка на время ожидания
        if (millis() - startMillis > 1000) { // Тайм-аут 1 секунда
            return false; // Время ожидания истекло
        }
    }

    // Чтение данных полученных через последовательный порт
    _serialPort->readBytes(buffer, length);

    // Проверка CRC (предполагается, что CRC находится в последних 2 байтах)
    unsigned int crcReceived = (buffer[length - 1] << 8) | buffer[length - 2]; // Получаем CRC из последних двух байтов
    unsigned int crcCalculated = crc_chk_value(buffer, length - 2); // Вычисляем CRC для полученных данных
    
    // Проверка соответствия полученного и вычисленного CRC
    if (crcReceived == crcCalculated) {
        return true; // Данные успешно получены и проверены
    }
    return false; // Ошибка в полученных данных (неверный CRC)
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