#pragma once
#include <stdexcept>
#include <string>

class BarbershopException : public std::runtime_error {
public:
    explicit BarbershopException(const std::string& message) : std::runtime_error(message) {}
};
