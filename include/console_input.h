#pragma once
#include <iostream>
#include <string>
#include "datetime.h"
#include "exceptions/invalid_data_exception.h"

bool readInt(int& value);
int readMenuChoice();
void discardRestOfLine();

std::string readLine(const std::string& prompt);
int readInteger(const std::string& prompt);
int readIntValue(const std::string& prompt, int minValue, int maxValue);
double readDouble(const std::string& prompt);
DateTime readDate(const std::string& prompt);
DateTime readDateTime(const std::string& prompt);
int readClock(const std::string& prompt);

template <typename T>
T readEntity() {
    T entity;
    std::cin >> entity;
    if (!std::cin) {
        discardRestOfLine();
        throw InvalidDataException("input", "a number was expected, the object was not created");
    }
    return entity;
}
