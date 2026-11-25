#pragma once
#include <string>
#include "barbershop_exception.h"

class ClientBannedException : public BarbershopException {
private:
    std::string clientName;

public:
    explicit ClientBannedException(const std::string& clientNameValue)
        : BarbershopException("Клиент «" + clientNameValue
            + "» заблокирован и не может записываться. Обратитесь к администратору барбершопа."),
        clientName(clientNameValue) {}

    const std::string& getClientName() const { return clientName; }
};
