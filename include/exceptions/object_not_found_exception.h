#pragma once
#include <string>
#include "barbershop_exception.h"

class ObjectNotFoundException : public BarbershopException {
public:
    ObjectNotFoundException(const std::string& objectType, const std::string& key)
        : BarbershopException("Не найдено: " + objectType + " «" + key + "».") {}
};
