#pragma once
#include <string>

class Logger {
private:
    std::string filename;
    mutable bool failureReported;

public:
    explicit Logger(const std::string& filenameValue);

    static std::string makeLine(const std::string& event, const std::string& details);
    void log(const std::string& event, const std::string& details = "") const;
};
