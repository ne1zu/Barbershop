#pragma once
#include <string>
#include "barbershop_exception.h"

class FileIOException : public BarbershopException {
public:
    FileIOException(const std::string& filename, const std::string& operation)
        : BarbershopException("Error while " + operation + " file '" + filename + "'.") {}
};
