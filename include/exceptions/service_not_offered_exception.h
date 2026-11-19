#pragma once
#include <string>
#include "barbershop_exception.h"

class ServiceNotOfferedException : public BarbershopException {
public:
    ServiceNotOfferedException(const std::string& masterName, const std::string& serviceName)
        : BarbershopException("Master '" + masterName + "' does not offer the service '" + serviceName + "'.") {}
};
