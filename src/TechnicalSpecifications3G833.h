#pragma once

#include <Arduino.h>

namespace TechnicalSpecifications3G833 {

    /*** Основные технические характеристики станка 3Г833 ***/

    /** Диаметр обрабатываемого отверстия в мм **/
    constexpr double smallestDiameter = 30.00;    // наименьший диаметр
    constexpr double largestDiameter = 125.00;    // наибольший диаметр
    constexpr double permissibleDiameter = 165.00; // допустимый диаметр

    /** Длина хонингования в мм **/
    constexpr double smallestLength = 30.00;      // наименьшая длина
    constexpr double largestLength = 450.00;      // наибольшая длина

    /** Вылет шпинделя в мм **/
    constexpr double spindleDeparture = 300.00;

    /** Расстояние от нижнего торца хоны до поверхности плиты в мм **/
    constexpr double smallestDistance = 50.00;    // наименьшая дистанция
    constexpr double largestDistance = 550.00;    // наибольшая дистанция

    /** Количество скоростей вращательного движения в об/мин **/
    constexpr int quantityRotationalSpeed = 3;    // Количество скоростей вращательного движения
    constexpr double rotationalSpeedOne = 155.00;  // Первая скорость вращательного движения
    constexpr double rotationalSpeedTwo = 280.00;  // Вторая скорость вращательного движения
    constexpr double rotationalSpeedThree = 400.00; // Третья скорость вращательного движения

    /** Количество скоростей возвратно-поступательного движения в м/мин **/
    constexpr int quantityReturnsSpeed = 3;       // Количество скоростей возвратно-поступательного движения
    constexpr double returnsSpeedOne = 8.00;      // Первая скорость возвратно-поступательного движения
    constexpr double returnsSpeedTwo = 11.50;      // Вторая скорость возвратно-поступательного движения
    constexpr double returnsSpeedThree = 16.927; // Третья скорость возвратно-поступательного движения (расчётная)

    /** Наибольшее вертикальное перемещение шпинделя в мм **/
    constexpr double maxVerticalMovementSpindle = 500.00;

    /** Характеристики электромоторов об/мин **/
    constexpr double motorSpindleRPM = 1430.00;   // Максимальная скорость вращения мотора шпинделя
    constexpr double motorReturnsRPM = 930.00;    // Максимальная скорость вращения мотора возвратно-поступательного движения

    /** Мощность моторов в Вт **/
    constexpr double motorSpindlePower = 3000.00; // Максимальная мощность мотора шпинделя
    constexpr double motorReturnsPower = 1100.00; // Максимальная мощность мотора возвратно-поступательного движения

    /** Допустимый угол сетки при хонинговании цилиндра двигателя **/
    constexpr int maximumScrubbingAngle = 90;     // Максимальный угол хонингования
    constexpr int minimalScrubbingAngle = 30;     // Минимальный угол хонингования

    /*** Расчёт скоростей возвратно-поступательного движения из кинематики ***/
    namespace KinematicsReturns {
        // Параметры скоростных шкивов
        constexpr int GEAR_PULLEY_SPEED_ONE1 = 70;      // Диаметр шкива 1 первой передачи
        constexpr int GEAR_PULLEY_SPEED_ONE2 = 190;     // Диаметр шкива 2 первой передачи

        constexpr int GEAR_PULLEY_SPEED_TWO1 = 84;      // Диаметр шкива 1 второй передачи
        constexpr int GEAR_PULLEY_SPEED_TWO2 = 175;     // Диаметр шкива 2 второй передачи

        constexpr int GEAR_PULLEY_SPEED_THREE1 = 110;   // Диаметр шкива 1 второй передачи
        constexpr int GEAR_PULLEY_SPEED_THREE2 = 155;   // Диаметр шкива 2 второй передачи

        // Коэффициенты редукции шкива от двигателя к входному валу
        constexpr double PULLEY_RATIO_1 = static_cast<double>(GEAR_PULLEY_SPEED_ONE1) / GEAR_PULLEY_SPEED_ONE2;      // Первая передача 70/190 = 0.36
        constexpr double PULLEY_RATIO_2 = static_cast<double>(GEAR_PULLEY_SPEED_TWO1) / GEAR_PULLEY_SPEED_TWO2;      // Вторая передача 84/175 = 0.48
        constexpr double PULLEY_RATIO_3 = static_cast<double>(GEAR_PULLEY_SPEED_THREE1) / GEAR_PULLEY_SPEED_THREE2;  // Третья передача 110/155 = 0.71

        // Параметры зубчатых передач в редукторе
        constexpr int GEAR_Z1 = 18;      // Число зубьев шестерни на валу 1
        constexpr int GEAR_Z2 = 45;      // Число зубьев шестерни на валу 2 (первая пара)
        constexpr int GEAR_Z3 = 22;      // Число зубьев шестерни на валу 2 (вторая пара)
        constexpr int GEAR_Z4 = 55;      // Число зубьев шестерни на валу 3 (вторая пара)

        // Выходная шестерня на валу 3
        constexpr int GEAR_Z5 = 17;      // Число зубьев выходной шестерни
        constexpr double MODULE_MM = 3.0; // Модуль зубьев, мм

        // Расчёт передаточных отношений редуктора
        constexpr double GEAR_RATIO_1 = static_cast<double>(GEAR_Z1) / GEAR_Z2;      // 18/45 = 0.4
        constexpr double GEAR_RATIO_2 = static_cast<double>(GEAR_Z3) / GEAR_Z4;      // 22/55 = 0.4
        constexpr double GEARBOX_RATIO = GEAR_RATIO_1 * GEAR_RATIO_2;                // 0.4*0.4 = 0.16

        // Диаметр делительной окружности выходной шестерни, мм
        constexpr double PITCH_DIAMETER_MM = MODULE_MM * GEAR_Z5;                    // 3*17 = 51 мм

        // Перемещение рейки за один оборот шестерни, мм
        constexpr double TRAVEL_PER_REVOLUTION_MM = PI * PITCH_DIAMETER_MM; // π*51 ≈ 160.221 мм

        // Константа для расчёта скорости рейки
        constexpr double SPEED_CONSTANT = (motorReturnsRPM * TRAVEL_PER_REVOLUTION_MM * GEARBOX_RATIO) / 1000.0; // (930*160.221*0.16) ≈ 23.840 м/мин

        // Финальные скорости возвратно-поступательного движения, м/мин
        constexpr double CALC_RETURNS_SPEED_1 = SPEED_CONSTANT * PULLEY_RATIO_1; // (23.840*0.36) ≈ 8.582 м/мин
        constexpr double CALC_RETURNS_SPEED_2 = SPEED_CONSTANT * PULLEY_RATIO_2; // (23.840*0.48) ≈ 11.443 м/мин
        constexpr double CALC_RETURNS_SPEED_3 = SPEED_CONSTANT * PULLEY_RATIO_3; // (23.840*0.71) ≈ 16.927 м/мин
    } // namespace KinematicsReturns

    namespace KinematicsRotational {
        // Параметры скоростных шкивов
        constexpr int GEAR_PULLEY_SPEED_ONE1 = 140;     // Диаметр шкива 1 первой передачи
        constexpr int GEAR_PULLEY_SPEED_ONE2 = 260;     // Диаметр шкива 2 первой передачи

        constexpr int GEAR_PULLEY_SPEED_TWO1 = 200;     // Диаметр шкива 1 второй передачи
        constexpr int GEAR_PULLEY_SPEED_TWO2 = 215;     // Диаметр шкива 2 второй передачи

        constexpr int GEAR_PULLEY_SPEED_THREE1 = 235;   // Диаметр шкива 1 второй передачи
        constexpr int GEAR_PULLEY_SPEED_THREE2 = 180;   // Диаметр шкива 2 второй передачи

        // Коэффициенты редукции шкива от двигателя к входному валу
        constexpr double PULLEY_RATIO_1 = static_cast<double>(GEAR_PULLEY_SPEED_ONE1) / GEAR_PULLEY_SPEED_ONE2;      // Первая передача 140/260 = 0.54
        constexpr double PULLEY_RATIO_2 = static_cast<double>(GEAR_PULLEY_SPEED_TWO1) / GEAR_PULLEY_SPEED_TWO2;      // Вторая передача 200/215 = 0.93
        constexpr double PULLEY_RATIO_3 = static_cast<double>(GEAR_PULLEY_SPEED_THREE1) / GEAR_PULLEY_SPEED_THREE2;  // Третья передача 235/180 = 1.30

        // Параметры зубчатых передач в редукторе
        constexpr int GEAR_Z1 = 18;      // Число зубьев шестерни на валу 1
        constexpr int GEAR_Z2 = 45;      // Число зубьев шестерни на валу 2 (первая пара)
        constexpr int GEAR_Z3 = 19;      // Число зубьев шестерни на валу 2 (вторая пара)
        constexpr int GEAR_Z4 = 47;      // Число зубьев шестерни на промежуточном валу 3
        constexpr int GEAR_Z5 = 35;      // Число зубьев шестерни на валу 4 шпинделя

        // Расчёт передаточных отношений редуктора
        constexpr double GEAR_RATIO_1 = static_cast<double>(GEAR_Z1) / GEAR_Z2;      // 18/45 = 0.4
        constexpr double GEAR_RATIO_2 = static_cast<double>(GEAR_Z3) / GEAR_Z5;      // 19/35 = 0.54
        constexpr double GEARBOX_RATIO = GEAR_RATIO_1 * GEAR_RATIO_2;                // 0.4*0.54 = 0.217

        // Финальные обороты шпинделя, об/мин
        constexpr double CALC_RETURNS_SPEED_1 = motorSpindleRPM * GEARBOX_RATIO * PULLEY_RATIO_1; // (1430*0.217*0.54) ≈ 155.00 об/мин
        constexpr double CALC_RETURNS_SPEED_2 = motorSpindleRPM * GEARBOX_RATIO * PULLEY_RATIO_2; // (1430*0.217*0.93) ≈ 280.00 об/мин
        constexpr double CALC_RETURNS_SPEED_3 = motorSpindleRPM * GEARBOX_RATIO * PULLEY_RATIO_3; // (1430*0.217*1.30) ≈ 400.00 об/мин

        // Функция для расчёта окружной скорости (V, м/мин) инструмента на шпинделе в зависимости от диаметра инструмента (D, мм)
        constexpr double calcRotationalPeripheralSpeed(const double diameter = 82.00 , const byte speed = 1) {
            return  speed == 1 ? PI * (diameter / 1000.0) * CALC_RETURNS_SPEED_1 :
                    speed == 2 ? PI * (diameter / 1000.0) * CALC_RETURNS_SPEED_2 :
                    speed == 3 ? PI * (diameter / 1000.0) * CALC_RETURNS_SPEED_3 : -1 ;
        }

    } // namespace KinematicsRotational

    // Альтернативные (расчётные) значения скоростей возвратно-поступательного движения
    namespace CalculatedSpeeds {
        constexpr int quantityReturnsSpeed = 3;    // Количество скоростей (по числу шкивов)
        constexpr double returnsSpeedOne = KinematicsReturns::CALC_RETURNS_SPEED_1;    // ≈7.152 м/мин
        constexpr double returnsSpeedTwo = KinematicsReturns::CALC_RETURNS_SPEED_2;    // ≈8.344 м/мин
        constexpr double returnsSpeedThree = KinematicsReturns::CALC_RETURNS_SPEED_3;  // ≈11.919 м/мин
        constexpr double returnsSpeedFour = 0.00;  // Четвертая скорость не используется
    } // namespace CalculatedSpeeds

} // namespace TechnicalSpecifications3G833

// using namespace TechnicalSpecifications3G833;
// Или более явно:
// double speed1 = TechnicalSpecifications3G833::CalculatedSpeeds::returnsSpeedOne;