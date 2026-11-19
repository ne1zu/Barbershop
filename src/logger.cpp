#include "logger.h"
#include <ctime>
#include <fstream>
#include <iostream>

namespace {
    std::string currentTimestamp() {
        std::time_t now = std::time(nullptr);
        std::tm local = {};
#ifdef _WIN32
        localtime_s(&local, &now);
#else
        localtime_r(&now, &local);
#endif
        char buffer[32];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &local);
        return buffer;
    }

    std::string makeSingleLine(std::string text) {
        for (char& symbol : text) {
            if (symbol == '\n' || symbol == '\r') symbol = ' ';
        }
        return text;
    }
}

Logger::Logger(const std::string& filenameValue) : filename(filenameValue), failureReported(false) {}

std::string Logger::makeLine(const std::string& event, const std::string& details) {
    std::string line = currentTimestamp() + " | " + event;
    if (!details.empty()) line += " | " + makeSingleLine(details);
    return line;
}

void Logger::log(const std::string& event, const std::string& details) const {
    std::ofstream file(filename, std::ios::app);
    if (file) {
        file << makeLine(event, details) << '\n';
        file.close();
    }

    if (!file && !failureReported) {
        failureReported = true;
        std::cerr << "[WARNING] Cannot write to the journal '" << filename << "'.\n";
    }
}
