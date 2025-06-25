#pragma once

#include <Arduino.h>

enum FunctionTypes : uint8_t {
  Increase = 1, // Режим увелечения перемнной
  Decrease = 2, // Режим уменьшения переменной
  Edit = 3      // Режим редактирования переменной
};

enum DecIncrTypes : uint8_t { Inc = 1, Dec = 2 };

enum StartLevelSpeed : uint8_t {
  Speed_1 = 0, // Первая скорость
  Speed_2 = 1, // Вторая скорость
  Speed_3 = 2  // Третья скорость
};
