#pragma once

/***Основные технические харктеристики станка 3Г833***/
/**Диаметр хонингуемого отверстия в мм **/
#define smallestDiameter 30.00 // наименьший диаметр
#define largestDiameter 125.00 // наибольший диаметр
#define permissibleDiameter 165.00 // допустимый диаметр
/**Длина хонингования в мм**/
#define smallestLength 30.00 // наименьший длина
#define largestLength 450.00 // наибольшая длина
/**Вылет шпинделя в мм**/
#define spindleDeparture 300.00
/**Расстояние от нижнего торца хоны до поверхности плиты в мм **/
#define smallestDistance 50.00 // наименьшая дистанция
#define largestDistance 550.00 // наибольшая дистанция
/**Количество скоростей вращательного движения в об/мин **/
#define quantityRotationalSpeed 3
#define rotationalSpeedOne 155.00
#define rotationalSpeedTwo 280.00
#define rotationalSpeedThree 400.00
/**Количество скоростей возвратно-поступательного движения в м/мин **/
#define quantityReturnsSpeed 4
#define returnsSpeedOne 5.00
#define returnsSpeedTwo 8.00
#define returnsSpeedThree 11.00
#define returnsSpeedFour 18.00
/**Наибольшее вертикальное перемещение шпинделя в мм **/
#define maxVerticalMovementSpindle 500.00
/**Характеристи электро маторов об/мин **/
#define motorSpindleRPM 2800.00
#define motorReturnsRPM 920.00
/**Допустимый угол сетки при хонингании цилиндра двигателя обычно находится в диапазоне от 30° до 90° **/
#define maximumScrubbingAngle 90
#define minimalScrubbingAngle 30