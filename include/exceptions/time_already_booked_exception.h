#pragma once
#include <string>
#include "barbershop_exception.h"

class TimeAlreadyBookedException : public BarbershopException {
private:
    std::string personType;
    std::string personName;

public:
    TimeAlreadyBookedException(const std::string& typeValue, const std::string& nameValue,
        const std::string& busyTime)
        : BarbershopException(typeValue + " '" + nameValue + "' already has an appointment at this time ("
            + busyTime + ")."),
        personType(typeValue), personName(nameValue) {}

    const std::string& getPersonType() const { return personType; }
    const std::string& getPersonName() const { return personName; }
};
