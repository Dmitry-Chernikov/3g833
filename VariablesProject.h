#pragma once

#include <Arduino.h>

enum FunctionTypes : uint8_t {
  Increase = 1,
  Decrease = 2,
  Edit = 3
};

enum DecIncrTypes : uint8_t {
  Inc = 1,
  Dec = 2
};

enum StartLevelSpeed : uint8_t {
  Speed_1 = 0,  // Первая скорость
  Speed_2 = 1,  // Вторая скорость
  Speed_3 = 2   // Третья скорость
};


