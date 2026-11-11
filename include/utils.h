#pragma once
#include <string>
#include <string_view>

bool isBlank(std::string_view text);
bool isValidPhone(std::string_view phone);
bool isValidLogin(std::string_view login);
std::string hashPassword(std::string_view password);
double roundMoney(double value);
std::string formatMoney(double value);
std::string formatMinutes(int minutes);
