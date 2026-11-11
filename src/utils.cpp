#include "utils.h"
#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>

bool isBlank(std::string_view text) {
    return text.find_first_not_of(" \t\r\n") == std::string_view::npos;
}

bool isValidPhone(std::string_view phone) {
    if (phone.size() != 13 || phone.substr(0, 4) != "+375") return false;
    for (std::size_t i = 4; i < phone.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(phone[i]))) return false;
    }
    return true;
}

bool isValidLogin(std::string_view login) {
    if (login.size() < 3 || login.size() > 20) return false;
    for (char symbol : login) {
        if (!std::isalnum(static_cast<unsigned char>(symbol)) && symbol != '_') return false;
    }
    return true;
}

std::string hashPassword(std::string_view password) {
    unsigned long long hash = 14695981039346656037ULL;
    for (char symbol : password) {
        hash ^= static_cast<unsigned char>(symbol);
        hash *= 1099511628211ULL;
    }
    return std::to_string(hash);
}

double roundMoney(double value) {
    return std::round(value * 100.0) / 100.0;
}

std::string formatMoney(double value) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << value;
    return out.str();
}

std::string formatMinutes(int minutes) {
    std::ostringstream out;
    out << std::setfill('0') << std::setw(2) << minutes / 60 << ":" << std::setw(2) << minutes % 60;
    return out.str();
}
