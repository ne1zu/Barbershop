#include "console_input.h"
#include <sstream>
#include "utils.h"
#include "exceptions/cancelled_exception.h"

namespace {
    bool readRawLine(std::istream& is, std::string& text) {
        if (!std::getline(is, text)) return false;
        if (!text.empty() && text.back() == '\r') text.pop_back();
        return true;
    }
}

std::string readTextFrom(std::istream& is, const std::string& prompt) {
    std::cout << prompt;
    std::string text;
    if (!readRawLine(is, text)) throw CancelledException();
    if (isBlank(text)) throw CancelledException();
    return text;
}

std::string readLine(const std::string& prompt) {
    return readTextFrom(std::cin, prompt);
}

bool parseNumber(const std::string& text, int& value) {
    std::istringstream in(text);
    in >> value;
    if (in.fail()) return false;
    in >> std::ws;
    return in.eof();
}

bool parseNumber(const std::string& text, double& value) {
    std::istringstream in(text);
    in >> value;
    if (in.fail()) return false;
    in >> std::ws;
    return in.eof();
}

int readMenuChoice() {
    std::cout << "> ";
    std::string text;
    if (!readRawLine(std::cin, text)) return 0;
    int choice = 0;
    if (!parseNumber(text, choice)) {
        std::cout << "[ERROR] Enter a number from the menu.\n";
        return -1;
    }
    return choice;
}

int readSubmenuChoice(int maxChoice) {
    while (true) {
        int choice = readMenuChoice();
        if (choice >= 0 && choice <= maxChoice) return choice;
        if (choice >= 0) std::cout << "[ERROR] No such menu item.\n";
    }
}

int readInteger(const std::string& prompt) {
    while (true) {
        int value = 0;
        if (parseNumber(readLine(prompt), value)) return value;
        std::cout << "[ERROR] A whole number was expected. Try again or press Enter to cancel.\n";
    }
}

int readIntValue(const std::string& prompt, int minValue, int maxValue) {
    while (true) {
        int value = readInteger(prompt);
        if (value >= minValue && value <= maxValue) return value;
        std::cout << "[ERROR] The number must be between " << minValue << " and " << maxValue
            << ". Try again or press Enter to cancel.\n";
    }
}

double readDouble(const std::string& prompt) {
    while (true) {
        double value = 0.0;
        if (parseNumber(readLine(prompt), value)) return value;
        std::cout << "[ERROR] A number was expected. Try again or press Enter to cancel.\n";
    }
}

DateTime readDate(const std::string& prompt) {
    while (true) {
        std::string text = readLine(prompt);
        int day = 0;
        int month = 0;
        int year = 0;
        if (!DateTime::parseDate(text, day, month, year)) {
            std::cout << "[ERROR] Use the format dd.mm.yyyy. Try again or press Enter to cancel.\n";
            continue;
        }
        try {
            return DateTime(year, month, day);
        }
        catch (const InvalidDataException& error) {
            std::cout << "[ERROR] " << error.what() << " Try again or press Enter to cancel.\n";
        }
    }
}

DateTime readDateTime(const std::string& prompt) {
    while (true) {
        std::istringstream in(readLine(prompt));
        DateTime value;
        try {
            in >> value;
            if (!in.fail()) {
                in >> std::ws;
                if (in.eof()) return value;
            }
            std::cout << "[ERROR] Use the format dd.mm.yyyy hh:mm. Try again or press Enter to cancel.\n";
        }
        catch (const InvalidDataException& error) {
            std::cout << "[ERROR] " << error.what() << " Try again or press Enter to cancel.\n";
        }
    }
}

int readClock(const std::string& prompt) {
    while (true) {
        int hour = 0;
        int minute = 0;
        if (DateTime::parseTime(readLine(prompt), hour, minute) && DateTime::isValidTime(hour, minute)) {
            return hour * 60 + minute;
        }
        std::cout << "[ERROR] Use the format hh:mm. Try again or press Enter to cancel.\n";
    }
}
