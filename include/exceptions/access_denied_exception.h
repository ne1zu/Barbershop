#pragma once
#include <string>
#include "barbershop_exception.h"

class AccessDeniedException : public BarbershopException {
public:
    explicit AccessDeniedException(const std::string& action)
        : BarbershopException("Доступ запрещён: вам нельзя " + action + ".") {}
};
