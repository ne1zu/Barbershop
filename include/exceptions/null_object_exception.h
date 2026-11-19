#pragma once
#include <string>
#include "barbershop_exception.h"

class NullObjectException : public BarbershopException {
public:
    NullObjectException(const std::string& where, const std::string& what)
        : BarbershopException("Null object in '" + where + "': '" + what + "' is not set.") {}
};
