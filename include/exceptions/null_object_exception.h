#pragma once
#include <string>
#include "barbershop_exception.h"

class NullObjectException : public BarbershopException {
public:
    NullObjectException(const std::string& where, const std::string& what)
        : BarbershopException("Внутренняя ошибка в «" + where + "»: не задан объект «" + what + "».") {}
};
