#pragma once
#include <string>
#include "barbershop_exception.h"

class ClientBannedException : public BarbershopException {
public:
    ClientBannedException(const std::string& clientName, const std::string& reason)
        : BarbershopException("Client '" + clientName + "' is banned and cannot be booked. Reason: " + reason + ".") {}
};
