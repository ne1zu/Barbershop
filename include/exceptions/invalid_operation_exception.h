#pragma once
#include <string>
#include "barbershop_exception.h"

class InvalidOperationException : public BarbershopException {
public:
    InvalidOperationException(const std::string& operation, const std::string& reason)
        : BarbershopException("Operation '" + operation + "' is not possible: " + reason + ".") {}
};
