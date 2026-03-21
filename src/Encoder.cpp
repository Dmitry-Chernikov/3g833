#include "Encoder.h"

AS5048A angleSensor(SS);

void initEncoder() {
    angleSensor.init();
}

float getAngle(bool EnableMedianValue, byte NumberFunctionValues) {
    //delay(200);
    //angleSensor.printState();
    //Serial.println(angleSensor.RotationRawToAngle(angleSensor.getRawRotation(true, 64)));
    return angleSensor.rotationRawToAngle(angleSensor.getRawRotation(NumberFunctionValues, EnableMedianValue, false));
}

float getLinearMotion() {
    return angleSensor.linearDisplacementRack(
        angleSensor.absoluteAngleRotation(&_data.absoluteAngle, getAngle(), &_data.anglePrevious), _NormalModule,
        _NumberGearTeeth);

    // _lcd.clear();
    // _lcd.setCursor(0, 0);
    // _lcd.print(getLinearMotion(), 4);
    // _lcd.print(" mm");

    // _lcd.setCursor(0, 1);
    // //_lcd.print(val, DEC);

    // //_lcd.print(angleSensor.RotationRawToRadian(angleSensor.getRawRotation(true)),
    // DEC); _lcd.print(int(absoluteAngle), DEC);  //_lcd.print(millis()/1000);
    // _lcd.print(char(223));

    // _lcd.print(int(angleSensor.GetAngularMinutes(absoluteAngle)), DEC);
    // //_lcd.print(millis()/1000); _lcd.print(char(34));

    // _lcd.print(int(angleSensor.GetAngularSeconds(absoluteAngle)), DEC);
    // _lcd.print(char(39));
    // _lcd.print("  ");
}