#pragma once
#include <string>
#include "barbershop_exception.h"

class FileOpenException : public BarbershopException {
public:
    FileOpenException(const std::string& filename, const std::string& purpose)
        : BarbershopException("Cannot open file '" + filename + "' for " + purpose
            + ". Check the path and the access rights.") {}
};
