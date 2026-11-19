#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include "datetime.h"
#include "exceptions/invalid_data_exception.h"

std::string readTextFrom(std::istream& is, const std::string& prompt);
std::string readLine(const std::string& prompt);

bool parseNumber(const std::string& text, int& value);
bool parseNumber(const std::string& text, double& value);

int readMenuChoice();
int readSubmenuChoice(int maxChoice);
int readInteger(const std::string& prompt);
int readIntValue(const std::string& prompt, int minValue, int maxValue);
double readDouble(const std::string& prompt);
DateTime readDate(const std::string& prompt);
DateTime readDateTime(const std::string& prompt);
int readClock(const std::string& prompt);

template <typename T, typename Base>
std::string askText(std::istream& is, const std::string& prompt, T& probe, void (Base::*setter)(std::string_view)) {
    while (true) {
        std::string text = readTextFrom(is, prompt);
        try {
            (probe.*setter)(text);
            return text;
        }
        catch (const InvalidDataException& error) {
            std::cout << "[ERROR] " << error.what() << " Try again or press Enter to cancel.\n";
        }
    }
}

template <typename T, typename Base, typename Value>
Value askNumber(std::istream& is, const std::string& prompt, T& probe, void (Base::*setter)(Value)) {
    while (true) {
        std::string text = readTextFrom(is, prompt);
        Value value{};
        if (!parseNumber(text, value)) {
            std::cout << "[ERROR] A number was expected. Try again or press Enter to cancel.\n";
            continue;
        }
        try {
            (probe.*setter)(value);
            return value;
        }
        catch (const InvalidDataException& error) {
            std::cout << "[ERROR] " << error.what() << " Try again or press Enter to cancel.\n";
        }
    }
}

template <typename T>
T readEntity() {
    T entity;
    std::cout << "(press Enter on an empty line to cancel)\n";
    std::cin >> entity;
    return entity;
}
