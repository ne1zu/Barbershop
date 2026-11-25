#pragma once
#include <string>
#include "barbershop_exception.h"

class FileIOException : public BarbershopException {
public:
    FileIOException(const std::string& filename, const std::string& operation)
        : BarbershopException("Ошибка при работе с файлом «" + filename + "» (" + operation + ").") {}
};
