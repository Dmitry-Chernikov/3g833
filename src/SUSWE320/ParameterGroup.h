#pragma once

#include "SUSWE320.h"

class ParameterGroup {
public:
    ParameterGroup(const char *groupName, int maxParameters = 10);
    ~ParameterGroup();
    void addParameter(const Parameter &param);

private:
    const char* _groupName;         // Название группы параметров
    const int   _maxParameters;     // Максимальное количество параметров в группе, по умолчанию 10 в конструкторе класса
    Parameter*  _parameters;        // Массив параметров
    int         _parameterCount;    // Количество добавленных параметров
};
