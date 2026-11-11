#include "datetime.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include "exceptions/invalid_data_exception.h"

namespace {
    const int MIN_YEAR = 2000;
    const int MAX_YEAR = 2100;
    const long long MINUTES_IN_DAY = 24 * 60;
    const long long DAYS_TO_YEAR_2000 = 10957;

    long long daysFromCivil(int year, int month, int day) {
        year -= month <= 2 ? 1 : 0;
        long long era = (year >= 0 ? year : year - 399) / 400;
        long long yearOfEra = year - era * 400;
        long long dayOfYear = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
        long long dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
        return era * 146097 + dayOfEra - 719468;
    }

    void civilFromDays(long long days, int& year, int& month, int& day) {
        days += 719468;
        long long era = (days >= 0 ? days : days - 146096) / 146097;
        long long dayOfEra = days - era * 146097;
        long long yearOfEra = (dayOfEra - dayOfEra / 1460 + dayOfEra / 36524 - dayOfEra / 146096) / 365;
        long long dayOfYear = dayOfEra - (365 * yearOfEra + yearOfEra / 4 - yearOfEra / 100);
        long long monthPart = (5 * dayOfYear + 2) / 153;
        day = static_cast<int>(dayOfYear - (153 * monthPart + 2) / 5 + 1);
        month = static_cast<int>(monthPart < 10 ? monthPart + 3 : monthPart - 9);
        year = static_cast<int>(yearOfEra + era * 400 + (month <= 2 ? 1 : 0));
    }
}

DateTime::DateTime() : year(MIN_YEAR), month(1), day(1), hour(0), minute(0) {}

DateTime::DateTime(int yearValue, int monthValue, int dayValue, int hourValue, int minuteValue)
    : year(yearValue), month(monthValue), day(dayValue), hour(hourValue), minute(minuteValue) {
    if (!isValidDate(dayValue, monthValue, yearValue)) {
        throw InvalidDataException("date", "there is no such date: " + std::to_string(dayValue) + "."
            + std::to_string(monthValue) + "." + std::to_string(yearValue));
    }
    if (!isValidTime(hourValue, minuteValue)) {
        throw InvalidDataException("time", "hours must be 0-23 and minutes 0-59");
    }
}

int DateTime::getYear() const { return year; }
int DateTime::getMonth() const { return month; }
int DateTime::getDay() const { return day; }
int DateTime::getHour() const { return hour; }
int DateTime::getMinute() const { return minute; }

bool DateTime::isValidDate(int dayValue, int monthValue, int yearValue) {
    if (yearValue < MIN_YEAR || yearValue > MAX_YEAR) return false;
    if (monthValue < 1 || monthValue > 12) return false;
    int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    bool leap = (yearValue % 4 == 0 && yearValue % 100 != 0) || (yearValue % 400 == 0);
    int limit = daysInMonth[monthValue - 1];
    if (monthValue == 2 && leap) limit = 29;
    return dayValue >= 1 && dayValue <= limit;
}

bool DateTime::isValidTime(int hourValue, int minuteValue) {
    return hourValue >= 0 && hourValue <= 23 && minuteValue >= 0 && minuteValue <= 59;
}

bool DateTime::parseDate(std::string_view text, int& dayValue, int& monthValue, int& yearValue) {
    std::istringstream in{ std::string(text) };
    char firstDot = 0;
    char secondDot = 0;
    in >> dayValue >> firstDot >> monthValue >> secondDot >> yearValue;
    return !in.fail() && firstDot == '.' && secondDot == '.' && in.peek() == std::char_traits<char>::eof();
}

bool DateTime::parseTime(std::string_view text, int& hourValue, int& minuteValue) {
    std::istringstream in{ std::string(text) };
    char colon = 0;
    in >> hourValue >> colon >> minuteValue;
    return !in.fail() && colon == ':' && in.peek() == std::char_traits<char>::eof();
}

DateTime DateTime::now() {
    std::time_t current = std::time(nullptr);
    std::tm local = {};
#ifdef _WIN32
    localtime_s(&local, &current);
#else
    localtime_r(&current, &local);
#endif
    return DateTime(local.tm_year + 1900, local.tm_mon + 1, local.tm_mday, local.tm_hour, local.tm_min);
}

DateTime DateTime::fromMinutes(long long totalMinutes) {
    long long days = totalMinutes / MINUTES_IN_DAY;
    int minutesOfDay = static_cast<int>(totalMinutes % MINUTES_IN_DAY);
    int yearValue = 0;
    int monthValue = 0;
    int dayValue = 0;
    civilFromDays(days + DAYS_TO_YEAR_2000, yearValue, monthValue, dayValue);
    return DateTime(yearValue, monthValue, dayValue, minutesOfDay / 60, minutesOfDay % 60);
}

long long DateTime::dayNumber() const {
    return daysFromCivil(year, month, day) - DAYS_TO_YEAR_2000;
}

long long DateTime::toMinutes() const {
    return dayNumber() * MINUTES_IN_DAY + minuteOfDay();
}

int DateTime::minuteOfDay() const { return hour * 60 + minute; }

int DateTime::dayOfWeek() const {
    return static_cast<int>((dayNumber() + 5) % 7);
}

std::string DateTime::dayName() const {
    const char* const names[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };
    return names[dayOfWeek()];
}

DateTime DateTime::addMinutes(int minutes) const {
    return fromMinutes(toMinutes() + minutes);
}

DateTime DateTime::addDays(int days) const {
    return fromMinutes(toMinutes() + static_cast<long long>(days) * MINUTES_IN_DAY);
}

DateTime DateTime::withMinuteOfDay(int minutes) const {
    return DateTime(year, month, day, minutes / 60, minutes % 60);
}

std::string DateTime::dateString() const {
    std::ostringstream out;
    out << std::setfill('0') << std::setw(2) << day << "." << std::setw(2) << month << "." << year;
    return out.str();
}

std::string DateTime::timeString() const {
    std::ostringstream out;
    out << std::setfill('0') << std::setw(2) << hour << ":" << std::setw(2) << minute;
    return out.str();
}

std::string DateTime::toString() const {
    return dateString() + " " + timeString();
}

bool DateTime::operator==(const DateTime& other) const { return toMinutes() == other.toMinutes(); }
bool DateTime::operator!=(const DateTime& other) const { return !(*this == other); }
bool DateTime::operator<(const DateTime& other) const { return toMinutes() < other.toMinutes(); }
bool DateTime::operator>(const DateTime& other) const { return toMinutes() > other.toMinutes(); }
bool DateTime::operator<=(const DateTime& other) const { return !(*this > other); }
bool DateTime::operator>=(const DateTime& other) const { return !(*this < other); }

std::ostream& operator<<(std::ostream& os, const DateTime& value) {
    os << value.toString();
    return os;
}

std::istream& operator>>(std::istream& is, DateTime& value) {
    std::string dateText;
    std::string timeText;
    if (!(is >> dateText >> timeText)) return is;

    int dayValue = 0;
    int monthValue = 0;
    int yearValue = 0;
    int hourValue = 0;
    int minuteValue = 0;
    if (!DateTime::parseDate(dateText, dayValue, monthValue, yearValue)
        || !DateTime::parseTime(timeText, hourValue, minuteValue)) {
        is.setstate(std::ios::failbit);
        return is;
    }
    value = DateTime(yearValue, monthValue, dayValue, hourValue, minuteValue);
    return is;
}
