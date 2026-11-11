#pragma once
#include <string>
#include "barbershop_exception.h"

class LinkViolationException : public BarbershopException {
public:
    LinkViolationException(const std::string& objectText, const std::string& reason)
        : BarbershopException("Link violation for " + objectText + ": " + reason + ".") {}
};
