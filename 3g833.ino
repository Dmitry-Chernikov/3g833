#include <Arduino.h>
#include <AS5048A.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <LiquidMenu.h>
#include <EEPROM.h>

//#include <avr/pgmspace.h>
//#include <util/delay.h>


AS5048A angleSensor(SS);  //выход на Arduino SS = PIN_SPI_SS (53), MOSI = PIN_SPI_MOSI (51), MISO = PIN_SPI_MISO (50), SCK = PIN_SPI_SCK (52)

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

#define OFF 0x0
#define RED 0x1
#define GREEN 0x2
#define YELLOW 0x3
#define BLUE 0x4
#define VIOLET 0x5
#define TEAL 0x6
#define WHITE 0x7

#define ENABLE_KYPAD   // Это для моей клавиатуры кода включаю
#define ENABLE_SWITCH  // Включение кода механического переключателя
#define ENABLE_EEPROM  // Включить код поддкржки использовангия EEPROM и переменно структуры Data и переменной data
//#define CLEAR_EEPROM // Запуск кода очистки всей памяти EEPROM для отладки

//Входные сигналы
#ifdef ENABLE_KYPAD
#define interruptRemote 12  // Вывод прерывания PCINT6 пульта управления
#endif

#define buttonEndCycle 22         // Вывод кнопки Конец Цикла
#define buttonStartFeed 23        // Вывод кнопки Подача-Пуск, подача электроэнэргии на двигатели возвратно поступательного движения
#define buttonSpindleStart 24     // Вывод кнопки Старт Шпиндель
#define buttonSpindleStop 25      // Вывод кнопки Шпиндель Стоп
#define buttonPush 26             // Вывод кнопки Толчковое опускание хонинговальной головки
#define buttonGeneralStop 27      // Вывод кнопки Общий Стоп, отключение питания на все миханизмы станка
#define switchAutoCycleManual 28  // Вывод переключателя автоматического цикла или ручного управления
#define switchTopSlider 29        // Вывод концевика верхнего положения ползуна
#define endSwitchTop 37           // Вывод концевика верхнего концевика цикла
#define endSwitchBottom 38        // Вывод концевика нижнего концевика цикла

//Выходные сигналы
#define electromagnetTop 30     // Вывод электро-муфты ЭТМ0921Н перемещения вверх ползуна
#define electromagnetBottom 31  // Вывод электро-муфты ЭТМ0921Н перемещения вниз ползуна
#define electromagnetManual 32  // Вывод электро-муфты ЭТМ0721Н ручное управления ползуна
#define electromagnetBrake 33   // Вывод электромагнита растормаживания МИС5100М перемещения ползуна
#define motorSpindle 34         // Вывод включения мотора вращения шпинделя
#define motorStartFeed 35       // Вывод включения мотора возвратно поступательного движения ползун
#define motorSelfCoolant 36     // Вывод включения смазочно охлаждающей жидкости

//Состояние режимов
volatile bool stateEndCycle = false;         // Состояние Конец-Цикла
volatile bool statePush = false;             // Состояние Толчёк-Ползун
volatile bool stateStartFeed = false;        // Состояние Подача-Пуск
volatile bool stateSpindle = false;          // Состояние Шпиндель-Старт\Стоп
volatile bool stateAutoCycleManual = false;  // Состояние Цикл-Ручной
volatile bool stateTopSlider = false;        // Состояние концевика верхнего положения ползуна
volatile bool stateStartCycle = false;       // Состояние Цикла
volatile bool stateGeneralStop = true;       // Состояние Общий Стоп

//Переменные энкодера
float AngleCurrent, AnglePrevious, AbsoluteAngle = 0;
float NormalModule = 3;      //Модуль нормальны
float NumberGearTeeth = 17;  //Число зубьев колеса или число заходов червяка

unsigned long previousMillisMenu = 0;
bool startMenu = false;
unsigned long intervalMenu = 2000;
unsigned long updateMenu = 500;
uint8_t buttons, second = 0;

//Переменные для инициализации меню, текст, используемый для Меню индикации для линий сохранения.
volatile bool IncDecMode = false;

enum FunctionTypes {
  increase = 1,
  decrease = 2,
  edit = 3
};

#if defined ENABLE_EEPROM || defined CLEAR_EEPROM
struct Data {
  char InitData;
  float LinearMove;
  float LimitTop;
  float LimitBottom;
  float CylinderDiametr;
  float CylinderAngle;
};

volatile Data data;
#endif

char input_saved[3];
char output_saved[3];

char string_saved[] = " *";
char string_notSaved[] = "  ";
char degree = (char)223;

unsigned long previousMillisSped1, previousMillisSped2, previousMillisSped3 = 0;
unsigned long interval1 = 3000;
unsigned long interval2 = 3000;
unsigned long interval3 = 3000;
bool oneBool, twoBool, threeBool = false;

//Объекты Liquidline может быть использован больше, чем один раз.
LiquidLine back_line(11, 1, "/BACK");


LiquidLine welcome_line1(0, 0, "Properties Menu");
LiquidLine welcome_line2(1, 1, "Machine 3G833");
LiquidScreen welcome_screen(welcome_line1, welcome_line2);

//Эти строки направлены на другие меню.
LiquidLine limits_line(1, 0, "Limits position");
LiquidLine cylinder_line(1, 1, "Cylinder size");
LiquidScreen settings_screen(limits_line, cylinder_line);

//Это первое меню.
LiquidMenu main_menu(lcd, welcome_screen, settings_screen, 1);


LiquidLine linear_move_line(0, 0, "Current ", data.LinearMove, "mm");
LiquidLine limit_top_line(1, 1, "Top:", data.LimitTop, "mm");
LiquidLine limit_bootom_line(1, 1, "Bottom:", data.LimitBottom, "mm");

LiquidScreen top_screen(linear_move_line, limit_top_line);
LiquidScreen bootom_screen(linear_move_line, limit_bootom_line);

LiquidLine oSave_line(0, 0, "Save");
LiquidScreen oSecondary_screen(oSave_line, back_line);

//Это второе меню.
LiquidMenu limit_menu(lcd, bootom_screen, top_screen, oSecondary_screen, 1);


LiquidLine diametr_title_line(0, 0, "Diameter");
LiquidLine diametr_value_line(1, 1, "Set ", data.CylinderDiametr, "mm");
LiquidScreen diametr_screen(diametr_title_line, diametr_value_line);

LiquidLine angle_title_line(0, 0, "Grid Angle");
LiquidLine angle_value_line(1, 1, "Set ", data.CylinderAngle, degree);
LiquidScreen angle_screen(angle_title_line, angle_value_line);

// И это последнее третье меню.
LiquidMenu cylinder_menu(lcd, diametr_screen, angle_screen, oSecondary_screen);

/*
 * Объект LiquidSystem объединяет объекты LiquidMenu для формирования системы меню.
 * Он обеспечивает те же функции, что и LiquidMenu с добавлением add_menu () и change_menu ().
 */
LiquidSystem menu_system(main_menu, limit_menu, cylinder_menu, 1);


///////////////////////////Процедуры меню begin///////////////////////////////////
// Функция обратного вызова, которая будет прикреплена к Back_line.

void go_back() {
  // Эта функция принимает ссылку на разыскиваемое меню.
  if (!stateAutoCycleManual && stateStartFeed && !stateTopSlider){
    startMenu = false;
  }

  if (stateGeneralStop) {
    menu_system.change_menu(main_menu);
    if (menu_system.get_currentScreen() == &settings_screen) {
      menu_system.set_focusedLine(0);
    }
  } 

}

void goto_limit_menu() {
  menu_system.change_menu(limit_menu);

  if (menu_system.get_currentScreen() == &oSecondary_screen) {
    menu_system.change_screen(&bootom_screen);
  }

  menu_system.set_focusedLine(1);
}

void goto_cylinder_menu() {
  menu_system.change_menu(cylinder_menu);

  if (menu_system.get_currentScreen() == &oSecondary_screen) {
    menu_system.change_screen(&diametr_screen);
  }

  menu_system.set_focusedLine(1);
}

void set_limit_top() {
  if (!stateAutoCycleManual && stateStartFeed && !stateTopSlider) {
    if (data.LinearMove > data.LimitBottom) {
      lcd.clear();
      lcd.setBacklight(RED);
      lcd.setCursor(5, 0);
      lcd.print("ERROR");
      lcd.setCursor(2, 1);
      lcd.print("TOP > BOOTOM");
      delay(2000);
      lcd.setBacklight(WHITE);
      menu_system.update();
    } else {
      lcd.setBacklight(GREEN);
      data.LimitTop = data.LinearMove;
      menu_system.update();
      delay(500);
      lcd.setBacklight(WHITE);
    }
  }

  if (stateGeneralStop) {
    IncDecMode = trigerRS(IncDecMode, true, IncDecMode);

    if ((menu_system.get_currentScreen() == &top_screen) && IncDecMode) {

      lcd.setBacklight(GREEN);
      menu_system.set_focusPosition(Position::RIGHT);

    } else {

      lcd.setBacklight(WHITE);
      menu_system.set_focusPosition(Position::LEFT);
    }
  }
}

void increase_limit_top() {
  //constrain(LEDbright, 0, 255);
  if (data.LimitTop <= (data.LimitBottom - 5)) {

    if (!oneBool && !twoBool) {
      data.LimitTop += 0.01;
      oneBool = stateMillisDelay(&previousMillisSped1, &interval1);
    }

    if (oneBool && !twoBool) {
      if (data.LimitTop > (data.LimitBottom - 5.5)) {
        oneBool = false;
        previousMillisSped1 = 0;
      } else {
        data.LimitTop += 0.10;
        twoBool = stateMillisDelay(&previousMillisSped2, &interval2);
      }
    }

    if (oneBool && twoBool && !threeBool) {
      if (data.LimitTop > (data.LimitBottom - 6)) {
        twoBool = false;
        previousMillisSped2 = 0;
      } else {
        data.LimitTop += 1.00;
        threeBool = stateMillisDelay(&previousMillisSped3, &interval3);
      }
    }

    if (oneBool && twoBool && threeBool) {
      if (data.LimitTop > (data.LimitBottom - 7)) {
        threeBool = false;
        previousMillisSped3 = 0;
      } else {
        data.LimitTop += 10.00;
      }
    }

  } else {
    data.LimitTop = data.LimitBottom - 5;
    lcd.clear();
    lcd.setBacklight(RED);
    lcd.setCursor(5, 0);
    lcd.print("ERROR");
    lcd.setCursor(2, 1);
    lcd.print("TOP > BOTTOM");
    delay(2000);
    lcd.setBacklight(GREEN);
    menu_system.update();
  }
}

void decrease_limit_top() {

  if (data.LimitTop >= 0) {

    if (!oneBool && !twoBool) {
      data.LimitTop -= 0.01;
      oneBool = stateMillisDelay(&previousMillisSped1, &interval1);
    }

    if (oneBool && !twoBool) {
      if (data.LimitTop < 0.5) {
        oneBool = false;
        previousMillisSped1 = 0;
      } else {
        data.LimitTop -= 0.10;
        twoBool = stateMillisDelay(&previousMillisSped2, &interval2);
      }
    }

    if (oneBool && twoBool && !threeBool) {
      if (data.LimitTop < 1) {
        twoBool = false;
        previousMillisSped2 = 0;
      } else {
        data.LimitTop -= 1.00;
        threeBool = stateMillisDelay(&previousMillisSped3, &interval3);
      }
    }

    if (oneBool && twoBool && threeBool) {
      if (data.LimitTop < 2) {
        threeBool = false;
        previousMillisSped3 = 0;
      } else {
        data.LimitTop -= 10.00;
      }
    }

  } else {
    data.LimitTop = 0;
    lcd.clear();
    lcd.setBacklight(RED);
    lcd.setCursor(5, 0);
    lcd.print("ERROR");
    lcd.setCursor(2, 1);
    lcd.print("TOP < 0");
    delay(2000);
    lcd.setBacklight(GREEN);
    menu_system.update();
  }
}

void set_limit_bootom() {
  if (!stateAutoCycleManual && stateStartFeed && !stateTopSlider) {
    if (data.LinearMove < data.LimitTop) {
      lcd.clear();
      lcd.setBacklight(RED);
      lcd.setCursor(5, 0);
      lcd.print("ERROR");
      lcd.setCursor(2, 1);
      lcd.print("BOOTOM < TOP");
      delay(2000);
      lcd.setBacklight(WHITE);
      menu_system.update();
    } else {
      lcd.setBacklight(GREEN);
      data.LimitBottom = data.LinearMove;
      menu_system.update();
      delay(500);
      lcd.setBacklight(WHITE);
    }
  }

  if (stateGeneralStop) {
    IncDecMode = trigerRS(IncDecMode, true, IncDecMode);

    if ((menu_system.get_currentScreen() == &bootom_screen) && IncDecMode) {

      lcd.setBacklight(GREEN);
      menu_system.set_focusPosition(Position::RIGHT);

    } else {

      lcd.setBacklight(WHITE);
      menu_system.set_focusPosition(Position::LEFT);
    }
  }
}

void increase_limit_bootom() {

  if (data.LimitBottom <= 500) {

    if (!oneBool && !twoBool) {
      data.LimitBottom += 0.01;
      oneBool = stateMillisDelay(&previousMillisSped1, &interval1);
    }

    if (oneBool && !twoBool) {
      if (data.LimitBottom > 499) {
        oneBool = false;
        previousMillisSped1 = 0;
      } else {
        data.LimitBottom += 0.10;
        twoBool = stateMillisDelay(&previousMillisSped2, &interval2);
      }
    }

    if (oneBool && twoBool && !threeBool) {
      if (data.LimitBottom > 498) {
        twoBool = false;
        previousMillisSped2 = 0;
      } else {
        data.LimitBottom += 1.00;
        threeBool = stateMillisDelay(&previousMillisSped3, &interval3);
      }
    }

    if (oneBool && twoBool && threeBool) {
      if (data.LimitBottom > 497) {
        threeBool = false;
        previousMillisSped3 = 0;
      } else {
        data.LimitBottom += 10.00;
      }
    }

  } else {
    data.LimitBottom = 500;
    lcd.clear();
    lcd.setBacklight(RED);
    lcd.setCursor(5, 0);
    lcd.print("ERROR");
    lcd.setCursor(2, 1);
    lcd.print("BOOTOM > 500");
    delay(2000);
    lcd.setBacklight(GREEN);
    menu_system.update();
  }
}

void decrease_limit_bootom() {
  if (data.LimitBottom >= data.LimitTop + 5) {

    if (!oneBool && !twoBool) {
      data.LimitBottom -= 0.01;
      oneBool = stateMillisDelay(&previousMillisSped1, &interval1);
    }

    if (oneBool && !twoBool) {
      if (data.LimitBottom < data.LimitTop + 5.5) {
        oneBool = false;
        previousMillisSped1 = 0;
      } else {
        data.LimitBottom -= 0.10;
        twoBool = stateMillisDelay(&previousMillisSped2, &interval2);
      }
    }

    if (oneBool && twoBool && !threeBool) {
      if (data.LimitBottom < data.LimitTop + 6) {
        twoBool = false;
        previousMillisSped2 = 0;
      } else {
        data.LimitBottom -= 1.00;
        threeBool = stateMillisDelay(&previousMillisSped3, &interval3);
      }
    }

    if (oneBool && twoBool && threeBool) {
      if (data.LimitBottom < data.LimitTop + 7) {
        threeBool = false;
        previousMillisSped3 = 0;
      } else {
        data.LimitBottom -= 10.00;
      }
    }

  } else {
    data.LimitBottom = data.LimitTop + 5;
    lcd.clear();
    lcd.setBacklight(RED);
    lcd.setCursor(5, 0);
    lcd.print("ERROR");
    lcd.setCursor(2, 1);
    lcd.print("BOOTOM < TOP");
    delay(2000);
    lcd.setBacklight(GREEN);
    menu_system.update();
  }
}

void mode_edit_value() {
  IncDecMode = trigerRS(IncDecMode, true, IncDecMode);

  if ((menu_system.get_currentScreen() == &diametr_screen) && IncDecMode
      || (menu_system.get_currentScreen() == &angle_screen) && IncDecMode) {

    lcd.setBacklight(GREEN);
    menu_system.set_focusPosition(Position::RIGHT);

  } else {

    lcd.setBacklight(WHITE);
    menu_system.set_focusPosition(Position::LEFT);
  }
}

void increase_diametr() {
  if (data.CylinderDiametr <= 165) {

    if (!oneBool && !twoBool) {
      data.CylinderDiametr += 0.01;
      oneBool = stateMillisDelay(&previousMillisSped1, &interval1);
    }

    if (oneBool && !twoBool) {
      if (data.CylinderDiametr > 164) {
        oneBool = false;
        previousMillisSped1 = 0;
      } else {
        data.CylinderDiametr += 0.10;
        twoBool = stateMillisDelay(&previousMillisSped2, &interval2);
      }
    }

    if (oneBool && twoBool && !threeBool) {
      if (data.CylinderDiametr > 163) {
        twoBool = false;
        previousMillisSped2 = 0;
      } else {
        data.CylinderDiametr += 1.00;
        threeBool = stateMillisDelay(&previousMillisSped3, &interval3);
      }
    }

    if (oneBool && twoBool && threeBool) {
      if (data.CylinderDiametr > 155) {
        threeBool = false;
        previousMillisSped3 = 0;
      } else {
        data.CylinderDiametr += 10.00;
      }
    }

  } else {
    data.CylinderDiametr = 165;
    lcd.clear();
    lcd.setBacklight(RED);
    lcd.setCursor(5, 0);
    lcd.print("ERROR");
    lcd.setCursor(2, 1);
    lcd.print("Diametr > 165");
    delay(2000);
    lcd.setBacklight(GREEN);
    menu_system.update();
  }
}

void decrease_diametr() {
  if (data.CylinderDiametr >= 30) {

    if (!oneBool && !twoBool) {
      data.CylinderDiametr -= 0.01;
      oneBool = stateMillisDelay(&previousMillisSped1, &interval1);
    }

    if (oneBool && !twoBool) {
      if (data.CylinderDiametr < 31) {
        oneBool = false;
        previousMillisSped1 = 0;
      } else {
        data.CylinderDiametr -= 0.10;
        twoBool = stateMillisDelay(&previousMillisSped2, &interval2);
      }
    }

    if (oneBool && twoBool && !threeBool) {
      if (data.CylinderDiametr < 33) {
        twoBool = false;
        previousMillisSped2 = 0;
      } else {
        data.CylinderDiametr -= 1.00;
        threeBool = stateMillisDelay(&previousMillisSped3, &interval3);
      }
    }

    if (oneBool && twoBool && threeBool) {
      if (data.CylinderDiametr < 45) {
        threeBool = false;
        previousMillisSped3 = 0;
      } else {
        data.CylinderDiametr -= 10.00;
      }
    }
  } else {
    data.CylinderDiametr = 30;
    lcd.clear();
    lcd.setBacklight(RED);
    lcd.setCursor(5, 0);
    lcd.print("ERROR");
    lcd.setCursor(2, 1);
    lcd.print("Diametr < 30");
    delay(2000);
    lcd.setBacklight(GREEN);
    menu_system.update();
  }
}

void increase_angle() {
  if (data.CylinderAngle < 120) {

    if (!oneBool && !twoBool) {
      data.CylinderAngle += 1.00;
      oneBool = stateMillisDelay(&previousMillisSped1, &interval1);
    }

    if (oneBool && !twoBool) {
      if (data.CylinderAngle > 100) {
        oneBool = false;
        previousMillisSped1 = 0;
      } else {
        data.CylinderAngle += 10.00;
        twoBool = stateMillisDelay(&previousMillisSped2, &interval2);
      }
    }
  } else {
    data.CylinderAngle = 120;
    lcd.clear();
    lcd.setBacklight(RED);
    lcd.setCursor(5, 0);
    lcd.print("ERROR");
    lcd.setCursor(3, 1);
    lcd.print("Angle > 120");
    delay(2000);
    lcd.setBacklight(GREEN);
    menu_system.update();
  }
}

void decrease_angle() {
  if (data.CylinderAngle > 10) {

    if (!oneBool && !twoBool) {
      data.CylinderAngle -= 1.00;
      oneBool = stateMillisDelay(&previousMillisSped1, &interval1);
    }

    if (oneBool && !twoBool) {
      if (data.CylinderAngle < 30) {
        oneBool = false;
        previousMillisSped1 = 0;
      } else {
        data.CylinderAngle -= 10.00;
        twoBool = stateMillisDelay(&previousMillisSped2, &interval2);
      }
    }
  } else {
    data.CylinderAngle = 10;
    lcd.clear();
    lcd.setBacklight(RED);
    lcd.setCursor(5, 0);
    lcd.print("ERROR");
    lcd.setCursor(3, 1);
    lcd.print("Angle < 10");
    delay(2000);
    lcd.setBacklight(GREEN);
    menu_system.update();
  }
}


///////////////////////////Процедуры меню end/////////////////////////////////////

bool trigerRS(bool currentState, uint8_t TrigSet, uint8_t TrigReset) {  //Триггер с приоритетом сброса
  if (TrigReset) { return false; }
  if (TrigSet) { return true; }
  return currentState;
}

#ifdef ENABLE_KYPAD
void pciSetup(byte pin) {
  //*Pin Change Interrupt Прерывание по изменению вывода*//
  // PCICR Регистр управления PCINT прерываниями, имеются три группы PCI0..2 в которые входят выводы по 8 штук
  // PCIFR Регистр флагов сработавших прерываний PCINT, показывает в какой группе сработало прерывание от вывода
  // PCMSKx Регистр маскировки прерываний каждого из портов для групп PCI0..2, активирует прерывание по выводу
  // 1 Задать обработчик для соответствующего прерывания PCINT, используя макрос ISR.
  // 2 Разрешить генерацию прерываний интересующим выводом микроконтроллера (регистр группы PCMSKx).
  // 3 Разрешить обработку прерывания PCINT, которое генерирует интересующий вывод (регистр PCICR).
  // 4 Установить бит I, разрешающий обработку прерываний глобально (регистр SREG).
  //PCMSK0 |= 1 << 6;
  *digitalPinToPCMSK(pin) |= bit(digitalPinToPCMSKbit(pin));  // Разрешаем PCINT для указанного пина
  //PCIFR |= 0 << 0
  PCIFR |= bit(digitalPinToPCICRbit(pin));  // Очищаем признак запроса прерывания для соответствующей группы пинов
  //PCICR |= 1 << 0;
  PCICR |= bit(digitalPinToPCICRbit(pin));  // Разрешаем PCINT для соответствующей группы пинов
  SREG |= 1 << SREG_I;                      // бит 7 Разрешить прерывания микроконтроллера
}

void readKeypad() {
  stateGeneralStop = trigerRS(stateGeneralStop, !digitalRead(buttonGeneralStop), !digitalRead(buttonStartFeed));                   // Общий стоп
  stateStartFeed = trigerRS(stateStartFeed, !digitalRead(buttonStartFeed), !digitalRead(buttonGeneralStop));                       // Подача-пуск
  digitalWrite(motorStartFeed, !stateStartFeed);                                                                                   // Запускаем мотор возвратно поступательного движения
  stateAutoCycleManual = trigerRS(stateAutoCycleManual, digitalRead(switchAutoCycleManual), !digitalRead(switchAutoCycleManual));  // Переключатель режимов: "Ввод хоны", "Ручной"
  stateTopSlider = trigerRS(stateTopSlider, digitalRead(switchTopSlider), !digitalRead(switchTopSlider));                          // Концевик парковки ползуна в верху исходного состояния
  stateEndCycle = trigerRS(stateEndCycle, !digitalRead(buttonEndCycle), stateGeneralStop);                                         // Конец цикла
}
#endif

bool stateMillisDelay(unsigned long* previousMillis, unsigned long* interval) {
  unsigned long currentMillis = millis();

  if (*previousMillis == 0) {
    *previousMillis = currentMillis;
  }

  //проверяем не прошел ли нужный интервал, если прошел то
  if ((currentMillis - *previousMillis) >= *interval) {
    // сохраняем время последнего переключения
    *previousMillis = 0;
    return HIGH;
  }
  return LOW;
}

float getLinearMotion() {
  return angleSensor.LinearDisplacementRack(angleSensor.AbsoluteAngleRotation(&AbsoluteAngle, angleSensor.RotationRawToAngle(angleSensor.getRawRotation(true, 64)), &AnglePrevious), NormalModule, NumberGearTeeth);

  // lcd.clear();
  // lcd.setCursor(0, 0);
  // lcd.print(getLinearMotion(), 4);
  // lcd.print(" mm");

  // lcd.setCursor(0, 1);
  // //lcd.print(val, DEC);

  // //lcd.print(angleSensor.RotationRawToRadian(angleSensor.getRawRotation(true)), DEC);
  // lcd.print(int(AbsoluteAngle), DEC);  //lcd.print(millis()/1000);
  // lcd.print(char(223));

  // lcd.print(int(angleSensor.GetAngularMinutes(AbsoluteAngle)), DEC);  //lcd.print(millis()/1000);
  // lcd.print(char(34));

  // lcd.print(int(angleSensor.GetAngularSeconds(AbsoluteAngle)), DEC);
  // lcd.print(char(39));
  // lcd.print("  ");
}

void setup() {
  cli();

#ifdef ENABLE_KYPAD
  pinMode(interruptRemote, INPUT_PULLUP);  // Подтянем пины источники PCINT к питанию
  pciSetup(interruptRemote);               // И разрешим на них прерывания T6
#endif

  /////////////Инициализация входов и подтягивание входов к положительному потенциалу с помощью внутрених резисторов/////////////
  pinMode(buttonEndCycle, INPUT_PULLUP);
  pinMode(buttonStartFeed, INPUT_PULLUP);
  pinMode(buttonSpindleStart, INPUT_PULLUP);
  pinMode(buttonSpindleStop, INPUT_PULLUP);
  pinMode(buttonPush, INPUT_PULLUP);
  pinMode(buttonGeneralStop, INPUT_PULLUP);
  pinMode(switchAutoCycleManual, INPUT_PULLUP);
  pinMode(switchTopSlider, INPUT_PULLUP);
  pinMode(endSwitchTop, INPUT_PULLUP);
  pinMode(endSwitchBottom, INPUT_PULLUP);

  /////////////Инициализация выходов и устанавливаем высокое состояние на выходе/////////////
  pinMode(electromagnetTop, OUTPUT);
  digitalWrite(electromagnetTop, true);

  pinMode(electromagnetBottom, OUTPUT);
  digitalWrite(electromagnetBottom, true);

  pinMode(electromagnetManual, OUTPUT);
  digitalWrite(electromagnetManual, true);

  pinMode(electromagnetBrake, OUTPUT);
  digitalWrite(electromagnetBrake, true);

  pinMode(motorSpindle, OUTPUT);
  digitalWrite(motorSpindle, true);

  pinMode(motorStartFeed, OUTPUT);
  digitalWrite(motorStartFeed, true);

  pinMode(motorSelfCoolant, OUTPUT);
  digitalWrite(motorSelfCoolant, true);

#ifdef ENABLE_KYPAD
  readKeypad();
#endif

  /////////////Инициализация энкодера/////////////
  angleSensor.init();
  //AnglePrevious = AngleCurrent = angleSensor.RotationRawToAngle(angleSensor.getRawRotation(true));

  lcd.begin(16, 2);
  //lcd.setBacklight(Color::WHITE);
  lcd.setBacklight(WHITE);

#ifdef CLEAR_EEPROM
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.get(0, data);
#endif

#ifdef ENABLE_EEPROM

  EEPROM.get(0, data);

  if (data.InitData != '*') {
    data.InitData = '*';
    data.LinearMove = 0;
    data.LimitTop = 10;
    data.LimitBottom = 50;
    data.CylinderDiametr = 80;
    data.CylinderAngle = 60;

    EEPROM.put(0, data);
    EEPROM.get(0, data);

    lcd.setCursor(0, 0);
    lcd.print("Init EEPROM OK");
    lcd.setCursor(0, 1);
    lcd.print(data.InitData);
    delay(1000);
  }

#endif

  back_line.set_focusPosition(Position::LEFT);
  //back_line.attach_function(1, go_back);
  //back_line.attach_function(2, go_back);
  back_line.attach_function(3, go_back);

  limits_line.set_focusPosition(Position::LEFT);
  //limits_line.attach_function(1, goto_limit_menu);
  //limits_line.attach_function(2, goto_limit_menu);
  limits_line.attach_function(3, goto_limit_menu);

  cylinder_line.set_focusPosition(Position::LEFT);
  //cylinder_line.attach_function(1, goto_cylinder_menu);
  //cylinder_line.attach_function(2, goto_cylinder_menu);
  cylinder_line.attach_function(3, goto_cylinder_menu);

  //main_menu.switch_focus(true);

  limit_top_line.set_focusPosition(Position::LEFT);
  limit_top_line.attach_function(1, increase_limit_top);
  limit_top_line.attach_function(2, decrease_limit_top);
  limit_top_line.attach_function(3, set_limit_top);

  limit_bootom_line.set_focusPosition(Position::LEFT);
  limit_bootom_line.attach_function(1, increase_limit_bootom);
  limit_bootom_line.attach_function(2, decrease_limit_bootom);
  limit_bootom_line.attach_function(3, set_limit_bootom);

  diametr_value_line.set_focusPosition(Position::LEFT);
  diametr_value_line.attach_function(1, increase_diametr);
  diametr_value_line.attach_function(2, decrease_diametr);
  diametr_value_line.attach_function(3, mode_edit_value);

  angle_value_line.set_focusPosition(Position::LEFT);
  angle_value_line.set_decimalPlaces(0);
  angle_value_line.attach_function(1, increase_angle);
  angle_value_line.attach_function(2, decrease_angle);
  angle_value_line.attach_function(3, mode_edit_value);

  strncpy(input_saved, string_saved, sizeof(string_saved));
  strncpy(output_saved, string_saved, sizeof(string_saved));

  sei();
}

void loop() {

  stateGeneralStop = trigerRS(stateGeneralStop, !digitalRead(buttonGeneralStop), !digitalRead(buttonStartFeed));                   // Общий стоп
  stateStartFeed = trigerRS(stateStartFeed, !digitalRead(buttonStartFeed), !digitalRead(buttonGeneralStop));                       // Подача-пуск
  digitalWrite(motorStartFeed, !stateStartFeed);                                                                                   // Запускаем мотор возвратно поступательного движения
  stateAutoCycleManual = trigerRS(stateAutoCycleManual, digitalRead(switchAutoCycleManual), !digitalRead(switchAutoCycleManual));  // Переключатель режимов: "Ввод хоны", "Ручной"
  stateTopSlider = trigerRS(stateTopSlider, digitalRead(switchTopSlider), !digitalRead(switchTopSlider));                          // Концевик парковки ползуна в верху исходного состояния
  stateEndCycle = trigerRS(stateEndCycle, !digitalRead(buttonEndCycle), stateGeneralStop);                                         // Конец цикла

  /////////////////////////////////////////////////////ЛОГИКА СОСТОЯНИЯ///////////////////////////////////////////////////////
  if (stateStartFeed) {          // Кнопку подача-пуск нажали. Запускаем мотор возвратно поступательного движения
    if (stateAutoCycleManual) {  // Переключатель включен в режим Цикл

      digitalWrite(electromagnetBrake, true);   // отключаем электромагнит растормаживания
      digitalWrite(electromagnetManual, true);  // отключаем электромагнит ручной подачи

      if (stateTopSlider) {  // Если ползун на концевике парковки

        AbsoluteAngle = 0;
        AnglePrevious = AngleCurrent = angleSensor.RotationRawToAngle(angleSensor.getRawRotation(true, 64));

        digitalWrite(electromagnetTop, true);    // выключить электромагнит движения вверх
        digitalWrite(electromagnetBrake, true);  // выключаем электромагнит растормживания
        stateEndCycle = false;                   // Конец цикла сбрасываем так как ползун стоит на концевике парковки

        statePush = trigerRS(statePush,
                             !digitalRead(buttonPush),
                             digitalRead(buttonPush) || stateGeneralStop);  // Толчковый ввод хоны
#ifdef ENABLE_SWITCH
        if (statePush && !digitalRead(endSwitchTop)) {  // Если кнопка Толковая нажата и переключатель включён вверх
#endif
          digitalWrite(electromagnetBrake, false);   // включаем электромагнит растормаживания
          digitalWrite(electromagnetBottom, false);  // включаем электромагнит движения вниз
        } else {

          digitalWrite(electromagnetBrake, true);   // выключаем электромагнит растормаживания
          digitalWrite(electromagnetBottom, true);  // выключаем электромагнит движения вниз
        }

      } else {  // Если ползун сошёл с концевика парковки

        if (stateEndCycle) {  // Если кнопку Конец Цикла нажали

          stateSpindle = trigerRS(stateSpindle,
                                  !digitalRead(buttonSpindleStart),
                                  !digitalRead(buttonSpindleStop) || stateEndCycle || stateGeneralStop);  // Шпиндель Стоп или Старт

          digitalWrite(motorSpindle, !stateSpindle);      // выключаем мотор шпинделя
          digitalWrite(motorSelfCoolant, !stateSpindle);  // выключаем мотор помпы СОЖ
          digitalWrite(electromagnetBottom, true);        // выключить электромагнит движения вниз

          digitalWrite(electromagnetBrake, false);  // включаем электромагнит растормаживание
          digitalWrite(electromagnetTop, false);    // включить электромагнит движения вверх

        } else {  // Если кнопку Конец Цикла не нажали

          if (stateSpindle) {  // Если в ручном режиме ввели в цилиндр и запустили шпиндель и перевели переключатель в Цикл

            digitalWrite(electromagnetBrake, false);  // включаем электромагнит растормаживания
            stateStartCycle = true;                   // вход в цикл

          } else {  // Шпиндель был выключен

            statePush = trigerRS(statePush,
                                 !digitalRead(buttonPush),
                                 digitalRead(buttonPush) || stateSpindle || stateEndCycle || stateGeneralStop);  // Толчковый ввод хоны

            stateSpindle = trigerRS(stateSpindle,
                                    !digitalRead(buttonSpindleStart),
                                    !digitalRead(buttonSpindleStop) || statePush || stateEndCycle || stateGeneralStop || digitalRead(endSwitchBottom));  // Шпиндель Стоп или Старт
#ifdef ENABLE_SWITCH
            if (statePush && !digitalRead(endSwitchTop)) {  // Если кнопка Толковая нажата и переключатель путевой включён вверх
#endif
              digitalWrite(electromagnetBrake, false);   // включаем электромагнит растормаживания
              digitalWrite(electromagnetBottom, false);  // включаем электромагнит движения вниз

            } else {  // Если кнопка Толковая не нажата и переключатель путевой не включён вверх

              digitalWrite(electromagnetBrake, true);   // отключаем электромагнит растормаживания
              digitalWrite(electromagnetBottom, true);  // отключаем электромагнит движения вниз

              if (stateSpindle && !digitalRead(endSwitchBottom)) {  //Если кнопку шпиндель пуск нажали и хона находится в нижнем положении цилиндра, концевик нижний включен

                digitalWrite(electromagnetBrake, false);  // включаем электромагнит растормаживания
                stateStartCycle = true;                   // вход в цикл
              }
            }
          }
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(getLinearMotion(), 4);
        lcd.print(" mm");
        delay(100);
      }
    } else {  // Переключатель включен в режим Ручной

      digitalWrite(electromagnetBrake, false);   // включаем электромагнит растормаживания
      digitalWrite(electromagnetManual, false);  // включаем электромагнит ручной подачи

      if (stateTopSlider) {  // Если ползун на концевике парковки. Концевик парковки ползуна в верху исходного состояния
        AbsoluteAngle = 0;
        AnglePrevious = AngleCurrent = angleSensor.RotationRawToAngle(angleSensor.getRawRotation(true, 64));
      } else {  // Если ползун сошёл с концевика парковки

        stateSpindle = trigerRS(stateSpindle,
                                !digitalRead(buttonSpindleStart),
                                !digitalRead(buttonSpindleStop) || stateEndCycle || stateGeneralStop);  // Шпиндель Стоп или Стоп

        digitalWrite(motorSpindle, !stateSpindle);      // включение выключение мотора шпинделя
        digitalWrite(motorSelfCoolant, !stateSpindle);  // включение выключение мотора помпы СОЖ

        /////////////////////////////////////////////////////LCD DISPLAY BUTTONS READ///////////////////////////////////////////////////////
        Menu();

        // lcd.clear();
        // lcd.setCursor(0, 0);
        // lcd.print(getLinearMotion(), 4);
        // lcd.print(" mm");
        // delay(100);
      }
    }
  } else {  // Кнопку Общий стоп нажали

    stateSpindle = trigerRS(stateSpindle,
                            !digitalRead(buttonSpindleStart),
                            !digitalRead(buttonSpindleStop) || stateEndCycle || stateGeneralStop);  // Шпиндель Стоп или Старт

    digitalWrite(motorSpindle, !stateSpindle);      // выключение мотора шпинделя
    digitalWrite(motorSelfCoolant, !stateSpindle);  // выключаем мотор помпы СОЖ
    digitalWrite(electromagnetTop, true);           // выключить электромагнит движения вверх
    digitalWrite(electromagnetBottom, true);        // выключить электромагнит движения вниз
    digitalWrite(electromagnetManual, true);        // выключаем электромагнит ручной подачи
    digitalWrite(electromagnetBrake, true);         // выключаем электромагнит растормаживания

    /////////////////////////////////////////////////////LCD DISPLAY BUTTONS READ///////////////////////////////////////////////////////
    Menu();
  }

  /////////////////////////////////////////////////////ЦИКЛ///////////////////////////////////////////////////////
  while (stateStartCycle) {  // Включён режим Цикл

    stateGeneralStop = trigerRS(stateGeneralStop, !digitalRead(buttonGeneralStop), !digitalRead(buttonStartFeed));                   // Общий стоп
    stateStartFeed = trigerRS(stateStartFeed, !digitalRead(buttonStartFeed), !digitalRead(buttonGeneralStop));                       // Подача-пуск
    digitalWrite(motorStartFeed, !stateStartFeed);                                                                                   // Запускаем мотор возвратно поступательного движенияя
    stateAutoCycleManual = trigerRS(stateAutoCycleManual, digitalRead(switchAutoCycleManual), !digitalRead(switchAutoCycleManual));  // Переключатель режимов: "Ввод хоны", "Ручной"
    stateTopSlider = trigerRS(stateTopSlider, digitalRead(switchTopSlider), !digitalRead(switchTopSlider));                          // Концевик парковки ползуна в верху исходного состояния
    stateEndCycle = trigerRS(stateEndCycle, !digitalRead(buttonEndCycle), stateGeneralStop);                                         // Конец цикла

    if (stateAutoCycleManual) {  // Переключатель включен в режим Цикл

      digitalWrite(electromagnetManual, true);  // выключить электромагнит ручного управления если был на ручном

      stateSpindle = trigerRS(stateSpindle,
                              !digitalRead(buttonSpindleStart),
                              !digitalRead(buttonSpindleStop) || stateEndCycle || stateGeneralStop);  // Шпиндель Стоп или Старт

      if (stateEndCycle) {  // Если кнопку Конец Цикла нажали

        digitalWrite(motorSpindle, !stateSpindle);      // выключаем мотор шпинделя
        digitalWrite(motorSelfCoolant, !stateSpindle);  // выключаем мотор помпы СОЖ
        digitalWrite(electromagnetBottom, true);        // выключить электромагнит движения вниз
        digitalWrite(electromagnetTop, false);          // включить электромагнит движения вверх

        if (stateTopSlider) {  // Ползун поднялся в верх, исходное состояние конец цикла

          digitalWrite(electromagnetTop, true);    // выключить электромагнит движения вверх
          digitalWrite(electromagnetBrake, true);  // выключаем электромагнит растормаживания
          stateEndCycle = false;                   // сбрасываем состояние выхода из цикла
          stateStartCycle = false;                 // выходим из цикла
        }

      } else {  // Если кнопку Конец Цикла не нажали

        if (stateSpindle) {  // Проверяем состояние шпинделя если включен

          digitalWrite(motorSpindle, !stateSpindle);      // включить мотор шпинделя
          digitalWrite(motorSelfCoolant, !stateSpindle);  // включаем мотор помпы СОЖ

#ifdef ENABLE_SWITCH
          if (!digitalRead(endSwitchTop)) {  // Если переключатель включен в положение верх
#endif
            digitalWrite(electromagnetTop, true);  // выключить электромагнит движения вверх
            delay(100);
            digitalWrite(electromagnetBottom, false);  // включить электромагнит движения вниз
          }

          if (!digitalRead(endSwitchBottom)) {  // Если переключатель включен в положение низ

            digitalWrite(electromagnetBottom, true);  // выключить электромагнит движения вниз
            delay(100);
            digitalWrite(electromagnetTop, false);  // включить электромагнит движения вверх
          }

        } else {  // Проверяем состояние шпинделя если выключен

          digitalWrite(motorSpindle, !stateSpindle);      // отключаем мотор шпинделя
          digitalWrite(motorSelfCoolant, !stateSpindle);  // отключаем мотор помпы СОЖ

#ifdef ENABLE_SWITCH
          if (!digitalRead(endSwitchTop)) {  // Если переключатель включен в положение верх
#endif
            digitalWrite(electromagnetTop, true);  // выключить электромагнит движения вверх
            delay(100);
            digitalWrite(electromagnetBottom, false);  // включить электромагнит движения вниз
          }

          if (!digitalRead(endSwitchBottom)) {  // Если переключатель включен в положение низ

            digitalWrite(electromagnetBottom, true);  // выключить электромагнит движения вниз
            delay(100);
            digitalWrite(electromagnetTop, false);  // включить электромагнит движения вверх
          }

          if (stateGeneralStop) {  // Нажата кнопка Общий стоп

            digitalWrite(motorSpindle, !stateSpindle);      // отключаем мотор шпинделя
            digitalWrite(motorSelfCoolant, !stateSpindle);  // отключаем мотор помпы СОЖ

            digitalWrite(electromagnetTop, true);     // выключить электромагнит движения вверх
            digitalWrite(electromagnetBottom, true);  // выключить электромагнит движения вниз
            digitalWrite(electromagnetBrake, true);   // выключить электромагнит растормаживания

            stateStartCycle = false;  // выходим из цикла
          }
        }
      }

    } else {  // Переключатель включен в режим Ручной

      stateSpindle = trigerRS(stateSpindle,
                              !digitalRead(buttonSpindleStart),
                              !digitalRead(buttonSpindleStop) || stateEndCycle || stateGeneralStop);  // Шпиндель Стоп или Старт

      if (stateGeneralStop) {  // Нажата кнопка Общий стоп

        digitalWrite(electromagnetTop, true);     // выключить электромагнит движения вверх
        digitalWrite(electromagnetBottom, true);  // выключить электромагнит движения вниз

        digitalWrite(motorSpindle, !stateSpindle);      // выключить мотор шпинделя
        digitalWrite(motorSelfCoolant, !stateSpindle);  // выключить мотор помпы СОЖ

        digitalWrite(electromagnetManual, true);  // выключить электромагнит ручного управления
        digitalWrite(electromagnetBrake, true);   // выключить электромагнит растормаживания
        stateStartCycle = false;                  // выходим из цикла
      } else {

        digitalWrite(electromagnetTop, true);     // выключить электромагнит движения вверх
        digitalWrite(electromagnetBottom, true);  // выключить электромагнит движения вниз

        digitalWrite(electromagnetManual, false);  // включить электромагнит ручного управления

        digitalWrite(motorSpindle, !stateSpindle);      // включить мотор шпинделя или отключить в зависимости от stateSpindle 
        digitalWrite(motorSelfCoolant, !stateSpindle);  // включить мотор помпы СОЖ или отключить в зависимости от stateSpindle 

        if (!stateSpindle){
          /////////////////////////////////////////////////////LCD DISPLAY BUTTONS READ///////////////////////////////////////////////////////
          Menu();
        }
        
      }
    }
  }

}

#ifdef ENABLE_KYPAD
ISR(PCINT0_vect) {  // Обработчик запросов прерывания от пинов PCINT0..PCINT7

  cli();         // сбрасываем флаг прерывания (Запретить прерывания)
  readKeypad();  // вызов процетуры опроса клавиатуры
  sei();         // устанавливаем флаг прерывания (Разрешить прерывания)
}
#endif


void Menu() {

  lcd.clear();
  //previousMillisMenu = millis();
  while (lcd.readButtons() > 0 && !startMenu) {
    startMenu = stateMillisDelay(&previousMillisMenu, &intervalMenu);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(second += 1);
    delay(1000);
  }

  if (startMenu) {
    second = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Start Menu");
    delay(1000);
    if (!stateAutoCycleManual && stateStartFeed && !stateTopSlider) {
      menu_system.change_menu(limit_menu);
      menu_system.change_screen(&bootom_screen);
      menu_system.set_focusedLine(1);
    }
    if (stateGeneralStop) {
      menu_system.change_menu(main_menu);
      menu_system.change_screen(&welcome_screen);
    }

  } else {
    if (second > 0) {
      second = 0;
    }
  }

  while (startMenu) {

    data.LinearMove = getLinearMotion();

    if (buttons = lcd.readButtons()) {

      if (buttons & BUTTON_UP) {
        if (IncDecMode) {
          delay(10);
          menu_system.call_function(FunctionTypes::increase);
        } else {
          delay(500);
          if (menu_system.get_currentScreen() == &top_screen
              || menu_system.get_currentScreen() == &bootom_screen
              || menu_system.get_currentScreen() == &diametr_screen
              || menu_system.get_currentScreen() == &angle_screen
              || menu_system.get_currentScreen() == &oSecondary_screen) {
            ///
          } else {
            menu_system.switch_focus(false);
          }
        }
      }

      if (buttons & BUTTON_DOWN) {
        if (IncDecMode) {
          delay(10);
          menu_system.call_function(FunctionTypes::decrease);
        } else {
          delay(500);
          if (menu_system.get_currentScreen() == &top_screen
              || menu_system.get_currentScreen() == &bootom_screen
              || menu_system.get_currentScreen() == &diametr_screen
              || menu_system.get_currentScreen() == &angle_screen
              || menu_system.get_currentScreen() == &oSecondary_screen) {
            ///
          } else {
            menu_system.switch_focus(true);
          }
        }
      }

      if (buttons & BUTTON_LEFT) {
        delay(500);
        if (IncDecMode) {
          ///
        } else {
          menu_system.previous_screen();

          if (menu_system.get_currentScreen() == &top_screen
              || menu_system.get_currentScreen() == &bootom_screen
              || menu_system.get_currentScreen() == &diametr_screen
              || menu_system.get_currentScreen() == &angle_screen
              || menu_system.get_currentScreen() == &oSecondary_screen) {

            menu_system.set_focusedLine(1);
            menu_system.softUpdate();
          }

          if (menu_system.get_currentScreen() == &settings_screen) {
            menu_system.set_focusedLine(0);
            menu_system.update();
          }
        }
      }

      if (buttons & BUTTON_RIGHT) {
        delay(500);
        if (IncDecMode) {
          ///
        } else {
          menu_system.next_screen();

          if (menu_system.get_currentScreen() == &top_screen
              || menu_system.get_currentScreen() == &bootom_screen
              || menu_system.get_currentScreen() == &diametr_screen
              || menu_system.get_currentScreen() == &angle_screen
              || menu_system.get_currentScreen() == &oSecondary_screen) {

            menu_system.set_focusedLine(1);
            menu_system.softUpdate();
          }

          if (menu_system.get_currentScreen() == &settings_screen) {
            menu_system.set_focusedLine(0);
            menu_system.update();
          }
        }
      }

      if (buttons & BUTTON_SELECT) {
        delay(500);
        menu_system.call_function(FunctionTypes::edit);
        menu_system.update();
      }

      //previousMillisMenu = millis();
      if (!stateAutoCycleManual && stateStartFeed && !stateTopSlider){

      }else{
        while (lcd.readButtons() > 0 && startMenu && !IncDecMode) {
          startMenu = !stateMillisDelay(&previousMillisMenu, &intervalMenu);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(second += 1);
          delay(1000);
        }

      }
      
    } else {
      oneBool = false;
      twoBool = false;
      threeBool = false;
      previousMillisSped1 = 0;
      previousMillisSped2 = 0;
      previousMillisSped3 = 0;
    }

    if (startMenu) {
      if (second > 0) {
        second = 0;
        menu_system.update();
      }

      if (stateMillisDelay(&previousMillisMenu, &updateMenu)) {
        if (menu_system.get_currentScreen() == &top_screen \ 
            || menu_system.get_currentScreen() == &bootom_screen
            || menu_system.get_currentScreen() == &diametr_screen
            || menu_system.get_currentScreen() == &angle_screen) {

          //menu_system.softUpdate();
          menu_system.update();
        } else {
          menu_system.softUpdate();
        }
      }

    } else {
      // Сохранение изменённой структуры data
      EEPROM.put(0, data);

      second = 0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Close Menu");
      delay(1000);
    }
  }
}
