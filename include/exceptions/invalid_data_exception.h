#pragma once
#include <string>
#include "barbershop_exception.h"

class InvalidDataException : public BarbershopException {
private:
    std::string field;

public:
    InvalidDataException(const std::string& fieldName, const std::string& reason)
        : BarbershopException("Invalid value for '" + fieldName + "': " + reason + "."), field(fieldName) {}

    const std::string& getField() const { return field; }
};
