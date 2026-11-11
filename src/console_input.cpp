#include "console_input.h"
#include <limits>

bool readInt(int& value) {
    if (std::cin >> value) return true;
    if (std::cin.eof()) return false;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return false;
}

int readMenuChoice() {
    std::cout << "> ";
    int choice;
    if (readInt(choice)) return choice;
    if (std::cin.eof()) return 0;
    std::cout << "[ERROR] Enter a number from the menu.\n";
    return -1;
}

void discardRestOfLine() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string text;
    if (!std::getline(std::cin >> std::ws, text)) {
        throw InvalidDataException("input", "the input was closed");
    }
    return text;
}

int readInteger(const std::string& prompt) {
    std::cout << prompt;
    int value;
    if (!readInt(value)) throw InvalidDataException("input", "a number was expected");
    return value;
}

int readIntValue(const std::string& prompt, int minValue, int maxValue) {
    int value = readInteger(prompt);
    if (value < minValue || value > maxValue) {
        throw InvalidDataException("number", "must be between " + std::to_string(minValue)
            + " and " + std::to_string(maxValue));
    }
    return value;
}

double readDouble(const std::string& prompt) {
    std::cout << prompt;
    double value;
    if (!(std::cin >> value)) {
        discardRestOfLine();
        throw InvalidDataException("input", "a number was expected");
    }
    return value;
}

DateTime readDate(const std::string& prompt) {
    std::string text = readLine(prompt);
    int day = 0;
    int month = 0;
    int year = 0;
    if (!DateTime::parseDate(text, day, month, year)) {
        throw InvalidDataException("date", "use the format dd.mm.yyyy");
    }
    return DateTime(year, month, day);
}

DateTime readDateTime(const std::string& prompt) {
    std::cout << prompt;
    DateTime value;
    std::cin >> value;
    if (!std::cin) {
        discardRestOfLine();
        throw InvalidDataException("date and time", "use the format dd.mm.yyyy hh:mm");
    }
    return value;
}

int readClock(const std::string& prompt) {
    std::string text = readLine(prompt);
    int hour = 0;
    int minute = 0;
    if (!DateTime::parseTime(text, hour, minute) || !DateTime::isValidTime(hour, minute)) {
        throw InvalidDataException("time", "use the format hh:mm");
    }
    return hour * 60 + minute;
}
