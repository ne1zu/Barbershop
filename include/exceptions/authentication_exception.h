#pragma once
#include "barbershop_exception.h"

class AuthenticationException : public BarbershopException {
public:
    AuthenticationException() : BarbershopException("Wrong login or password.") {}
};
