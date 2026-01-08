#pragma once

#include "SUSWE321.h"

class ParameterGroup {
public:
    ParameterGroup(const char *groupName, int maxParameters = 10);
    ~ParameterGroup();
    void addParameter(const Parameter &param);

private:
    const char* GROUP_NAME;         // Название группы параметров
    const int   MAX_PARAMETERS;     // Максимальное количество параметров в группе, по умолчанию 10 в конструкторе класса
    Parameter*  _parameters;        // Массив параметров
    int         _parameterCount;    // Количество добавленных параметров
};
