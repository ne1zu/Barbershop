#pragma once
#include <string>
#include "barbershop_exception.h"

class InvalidStatusTransitionException : public BarbershopException {
public:
    InvalidStatusTransitionException(const std::string& from, const std::string& to)
        : BarbershopException("Нельзя сменить статус «" + from + "» на «" + to + "».") {}
};
