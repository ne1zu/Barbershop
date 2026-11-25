#pragma once
#include <string>
#include "barbershop_exception.h"

class FileOpenException : public BarbershopException {
public:
    FileOpenException(const std::string& filename, const std::string& purpose)
        : BarbershopException("Не удаётся открыть файл «" + filename + "» (" + purpose
            + "). Проверьте путь и права доступа.") {}
};
