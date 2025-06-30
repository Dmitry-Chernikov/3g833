#include "ParameterGroup.h"


ParameterGroup::ParameterGroup(const char *groupName, int maxParameters) : 
                                                                        _groupName(groupName), 
                                                                        _maxParameters(maxParameters), 
                                                                        _parameterCount(0) {
    _parameters = new Parameter[maxParameters]; 
}

ParameterGroup::~ParameterGroup() {
    delete[] _parameters; // Освобождение памяти
}

// Процедура добавления параметра в группу
void ParameterGroup::addParameter(const Parameter &param) {
    if (_parameterCount < _maxParameters) {
        _parameters[_parameterCount++] = param;
    }else{
        Serial.println("Достигнуто максимальное количество параметров!");
    }
}