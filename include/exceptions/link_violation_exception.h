#pragma once
#include <string>
#include "barbershop_exception.h"

class LinkViolationException : public BarbershopException {
public:
    LinkViolationException(const std::string& objectText, const std::string& reason)
        : BarbershopException("Действие невозможно для " + objectText + ": " + reason + ".") {}
};
