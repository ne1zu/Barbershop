#pragma once
#include <string>
#include <string_view>
#include <iostream>

class DateTime {
private:
    int year;
    int month;
    int day;
    int hour;
    int minute;

public:
    DateTime();
    DateTime(int yearValue, int monthValue, int dayValue, int hourValue = 0, int minuteValue = 0);

    int getYear() const;
    int getMonth() const;
    int getDay() const;
    int getHour() const;
    int getMinute() const;

    static bool isValidDate(int dayValue, int monthValue, int yearValue);
    static bool isValidTime(int hourValue, int minuteValue);
    static bool parseDate(std::string_view text, int& dayValue, int& monthValue, int& yearValue);
    static bool parseTime(std::string_view text, int& hourValue, int& minuteValue);
    static DateTime now();
    static DateTime fromMinutes(long long totalMinutes);

    long long toMinutes() const;
    long long dayNumber() const;
    int minuteOfDay() const;
    int dayOfWeek() const;
    std::string dayName() const;

    DateTime addMinutes(int minutes) const;
    DateTime addDays(int days) const;
    DateTime withMinuteOfDay(int minutes) const;

    std::string dateString() const;
    std::string timeString() const;
    std::string toString() const;

    bool operator==(const DateTime& other) const;
    bool operator!=(const DateTime& other) const;
    bool operator<(const DateTime& other) const;
    bool operator>(const DateTime& other) const;
    bool operator<=(const DateTime& other) const;
    bool operator>=(const DateTime& other) const;

    friend std::ostream& operator<<(std::ostream& os, const DateTime& value);
    friend std::istream& operator>>(std::istream& is, DateTime& value);
};
