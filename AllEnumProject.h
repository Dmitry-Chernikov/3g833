#pragma once

enum FunctionTypes : uint8_t {
  increase = 1,
  decrease = 2,
  edit = 3
};

enum DecIncrTypes : uint8_t {
  inc = 1,
  dec = 2
};

enum StartLevelSpeed : uint8_t {
    SPEED_1 = 0,  // Первая скорость
    SPEED_2 = 1,  // Вторая скорость
    SPEED_3 = 2   // Третья скорость
};
