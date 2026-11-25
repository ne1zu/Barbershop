#include "master_service.h"
#include "config.h"
#include "utils.h"
#include "exceptions/null_object_exception.h"
#include "exceptions/invalid_data_exception.h"

MasterService::MasterService(std::shared_ptr<Service> serviceValue, double priceValue, int durationValue)
    : service(serviceValue), customPrice(0.0), customDuration(0) {
    if (!service) throw NullObjectException("MasterService", "service");
    setCustomPrice(priceValue);
    setCustomDuration(durationValue);
}

std::shared_ptr<Service> MasterService::getService() const { return service; }
double MasterService::getCustomPrice() const { return customPrice; }
int MasterService::getCustomDuration() const { return customDuration; }
bool MasterService::hasCustomPrice() const { return customPrice > 0; }
bool MasterService::hasCustomDuration() const { return customDuration > 0; }

void MasterService::setCustomPrice(double priceValue) {
    if (priceValue < 0) throw InvalidDataException("цена", "не может быть отрицательной (0 — автоматическая цена)");
    double rounded = roundMoney(priceValue);
    if (priceValue > 0 && rounded <= 0) {
        throw InvalidDataException("цена", "не меньше 0,01 (или 0 для автоматической цены)");
    }
    customPrice = rounded;
}

void MasterService::setCustomDuration(int durationValue) {
    if (durationValue < 0 || durationValue > MAX_SERVICE_MINUTES) {
        throw InvalidDataException("длительность", "от 0 до " + std::to_string(MAX_SERVICE_MINUTES)
            + " (0 means the standard duration)");
    }
    if (durationValue % GRID_STEP_MINUTES != 0) {
        throw InvalidDataException("длительность", "кратна " + std::to_string(GRID_STEP_MINUTES) + " минутам");
    }
    customDuration = durationValue;
}

bool MasterService::operator==(const MasterService& other) const {
    return service->getId() == other.service->getId();
}

std::ostream& operator<<(std::ostream& os, const MasterService& offer) {
    os << offer.service->getName() << " | Price: "
        << (offer.hasCustomPrice() ? formatMoney(offer.customPrice) : std::string("auto"))
        << " | Duration: " << (offer.hasCustomDuration() ? offer.customDuration : offer.service->getLength())
        << " min";
    return os;
}
