#pragma once
#include <string>
#include "barbershop_exception.h"

class OutsideWorkingHoursException : public BarbershopException {
public:
    OutsideWorkingHoursException(const std::string& masterName, const std::string& timeText)
        : BarbershopException("Мастер «" + masterName + "» не работает в это время (" + timeText + ").") {}
};
