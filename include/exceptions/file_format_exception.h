#pragma once
#include <string>
#include "barbershop_exception.h"

class FileFormatException : public BarbershopException {
private:
    int line;

public:
    FileFormatException(int lineNumber, const std::string& message)
        : BarbershopException("Wrong data in line " + std::to_string(lineNumber) + ": " + message),
        line(lineNumber) {}

    int getLine() const { return line; }
};
