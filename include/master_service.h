#pragma once
#include <memory>
#include <iostream>
#include "service.h"

class MasterService {
private:
    std::shared_ptr<Service> service;
    double customPrice;
    int customDuration;

public:
    explicit MasterService(std::shared_ptr<Service> serviceValue, double priceValue = 0.0, int durationValue = 0);

    std::shared_ptr<Service> getService() const;
    double getCustomPrice() const;
    int getCustomDuration() const;
    bool hasCustomPrice() const;
    bool hasCustomDuration() const;

    void setCustomPrice(double priceValue);
    void setCustomDuration(int durationValue);

    bool operator==(const MasterService& other) const;

    friend std::ostream& operator<<(std::ostream& os, const MasterService& offer);
};
