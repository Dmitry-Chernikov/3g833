#pragma once

#include "MemoryEeprom.h"
#include <AS5048A.h>
#include <Arduino.h>

// Переменные энкодера
const uint8_t _NormalModule = 3;     // Модуль нормальны
const uint8_t _NumberGearTeeth = 17; // Число зубьев колеса или число заходов червяка

extern AS5048A angleSensor; // выход на Arduino SS = PIN_SPI_SS (53), MOSI = PIN_SPI_MOSI (51), MISO = PIN_SPI_MISO (50), SCK = PIN_SPI_SCK (52)

void initEncoder();
float getAngle();
float getLinearMotion();