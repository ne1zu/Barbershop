#pragma once
#include <string>
#include "barbershop_exception.h"

class ClientBannedException : public BarbershopException {
private:
    std::string clientName;

public:
    explicit ClientBannedException(const std::string& clientNameValue)
        : BarbershopException("Client '" + clientNameValue
            + "' cannot be booked. Please contact the barbershop administrator."),
        clientName(clientNameValue) {}

    const std::string& getClientName() const { return clientName; }
};
