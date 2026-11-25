#pragma once
#include <string>
#include "barbershop_exception.h"

class DuplicateObjectException : public BarbershopException {
public:
    DuplicateObjectException(const std::string& objectType, const std::string& key)
        : BarbershopException("Уже существует: " + objectType + " «" + key + "».") {}
};
