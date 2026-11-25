#pragma once
#include <string>
#include "barbershop_exception.h"

class FileFormatException : public BarbershopException {
private:
    int line;

public:
    FileFormatException(int lineNumber, const std::string& message)
        : BarbershopException("Ошибка данных в строке " + std::to_string(lineNumber) + ": " + message),
        line(lineNumber) {}

    int getLine() const { return line; }
};
