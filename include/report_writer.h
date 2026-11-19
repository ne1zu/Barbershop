#pragma once
#include <string>
#include "barbershop.h"
#include "payment.h"

class ReportWriter {
private:
    std::string filename;

public:
    explicit ReportWriter(const std::string& filenameValue);

    void writeReport(const Barbershop& shop) const;
    void writeReceipt(const Payment& payment) const;
};
