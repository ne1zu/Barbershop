#pragma once
#include "barbershop_exception.h"

class CancelledException : public BarbershopException {
public:
    CancelledException() : BarbershopException("The action was cancelled.") {}
};
