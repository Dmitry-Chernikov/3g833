#include "MemoryEeprom.h"

Data _data;
Data _dataBuffer;  // временная переменная для проверки данных в EEPROM с data, чтобы не писать в EEPROM часто


void initMemory(){
  #ifdef ENABLE_EEPROM
    EEPROM.get(0, _data);

    if (_data.initData != '*') {
      _data.initData = '*';

      _data.linearMove = 0;
      _data.anglePrevious = 0;
      _data.absoluteAngle = 0;

      _data.limitTop = 10;
      _data.limitBottom = 50;
      _data.cylinderDiametr = 80;
      _data.cylinderAngle = 60;

      _data.stateElectromagnetTop = true;
      _data.stateElectromagnetBottom = true;
      _data.stateIntermediate = true;

      EEPROM.put(0, _data);

      lcdPrintString(_lcd, "INIT EEPROM OK", String(_data.initData), "", NOT_CHANGE_COLOR, NOT_CHANGE_COLOR, 0, 0, 1000, true, true);
    }
  #endif
}

void clearMemory(){
    #ifdef CLEAR_EEPROM
    lcdPrintString(_lcd, "CLEAR EEPROM", "", "", NOT_CHANGE_COLOR, NOT_CHANGE_COLOR, 0, 0, 0, true, false);

    uint8_t indexLine = 0;
    uint16_t compareParam = EEPROM.length() / 16;

    for (int i = 0; i < EEPROM.length(); i++) {
      EEPROM.write(i, 0);
      if (i = compareParam) {
        lcdPrintString(_lcd, "", "0", "", NOT_CHANGE_COLOR, NOT_CHANGE_COLOR, 0, indexLine, 0, false, false);
        compareParam = compareParam + (EEPROM.length() / 16);
        indexLine = indexLine + 1;
      }
    }

    lcdPrintString(_lcd, "CLEAR EEPROM OK", "", "", NOT_CHANGE_COLOR, NOT_CHANGE_COLOR, 0, 0, 1000, true, false);  
  #endif
}

template< typename LCD, typename B, typename D >
void saveEeprom(LCD lcd, B &dataBuffer, D &data) {
  EEPROM.get(0, dataBuffer);
  if (data != dataBuffer) {
    EEPROM.put(0, data);  // Сохранение изменений структуры data в EEPROM
    lcdPrintString(lcd, "SAVE EEPROM OK", String(data.initData), "", WHITE, NOT_CHANGE_COLOR, 0, 0, 1000, true, true);
  }
}
