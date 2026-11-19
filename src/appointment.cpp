#include "appointment.h"
#include "utils.h"
#include "exceptions/null_object_exception.h"
#include "exceptions/invalid_data_exception.h"

int Appointment::counter = 1;

bool parseStatus(const std::string& text, AppointmentStatus& status) {
    const AppointmentStatus all[] = { AppointmentStatus::Pending, AppointmentStatus::Confirmed,
        AppointmentStatus::Arrived, AppointmentStatus::NoShow, AppointmentStatus::Cancelled };
    for (AppointmentStatus candidate : all) {
        if (statusToString(candidate) == text) {
            status = candidate;
            return true;
        }
    }
    return false;
}

std::string statusToString(AppointmentStatus status) {
    switch (status) {
    case AppointmentStatus::Pending: return "Pending";
    case AppointmentStatus::Confirmed: return "Confirmed";
    case AppointmentStatus::Arrived: return "Arrived";
    case AppointmentStatus::NoShow: return "NoShow";
    case AppointmentStatus::Cancelled: return "Cancelled";
    }
    return "Unknown";
}

Appointment::Appointment(std::shared_ptr<Client> clientValue, std::shared_ptr<Master> masterValue,
    std::shared_ptr<Service> serviceValue, const DateTime& startValue)
    : id(0), client(clientValue), master(masterValue), service(serviceValue), price(0.0), duration(0),
    start(startValue), status(AppointmentStatus::Pending), firstVisit(false), paid(false) {
    if (!client) throw NullObjectException("Appointment", "client");
    if (!master) throw NullObjectException("Appointment", "master");
    if (!service) throw NullObjectException("Appointment", "service");
    price = master->calculatePrice(*service);
    duration = master->calculateDuration(*service);
    firstVisit = client->isNew();
}

Appointment::Appointment(int idValue, std::shared_ptr<Client> clientValue, std::shared_ptr<Master> masterValue,
    std::shared_ptr<Service> serviceValue, double priceValue, int durationValue, const DateTime& startValue,
    AppointmentStatus statusValue, bool firstVisitValue, bool paidValue)
    : id(idValue), client(clientValue), master(masterValue), service(serviceValue), price(priceValue),
    duration(durationValue), start(startValue), status(statusValue), firstVisit(firstVisitValue),
    paid(paidValue) {
    if (!client) throw NullObjectException("Appointment", "client");
    if (!master) throw NullObjectException("Appointment", "master");
    if (!service) throw NullObjectException("Appointment", "service");
    if (idValue <= 0) throw InvalidDataException("appointment id", "must be positive");
    if (priceValue < 0) throw InvalidDataException("price", "must not be negative");
    if (durationValue <= 0) throw InvalidDataException("duration", "must be positive");
    if (paidValue && statusValue != AppointmentStatus::Arrived) {
        throw InvalidDataException("paid", "only a visit marked as arrived can be paid");
    }
    if (counter <= idValue) counter = idValue + 1;
}

void Appointment::setStatus(AppointmentStatus statusValue) { status = statusValue; }
void Appointment::setStart(const DateTime& startValue) { start = startValue; }
void Appointment::markPaid() { paid = true; }
void Appointment::assignId() {
    if (id == 0) id = counter++;
}

int Appointment::getId() const { return id; }
std::shared_ptr<Client> Appointment::getClient() const { return client; }
std::shared_ptr<Master> Appointment::getMaster() const { return master; }
std::shared_ptr<Service> Appointment::getService() const { return service; }
double Appointment::getPrice() const { return price; }
int Appointment::getDuration() const { return duration; }
DateTime Appointment::getStart() const { return start; }
DateTime Appointment::getEnd() const { return start.addMinutes(duration); }
AppointmentStatus Appointment::getStatus() const { return status; }

std::string Appointment::getTimeRange() const {
    return start.toString() + "-" + getEnd().timeString();
}

bool Appointment::isActive() const {
    return status == AppointmentStatus::Pending || status == AppointmentStatus::Confirmed;
}

bool Appointment::occupiesTime() const {
    return isActive() || status == AppointmentStatus::Arrived;
}

bool Appointment::isFirstVisit() const { return firstVisit; }
bool Appointment::isPaid() const { return paid; }

bool Appointment::canTransitionTo(AppointmentStatus newStatus) const {
    if (!isActive()) return false;
    if (status == AppointmentStatus::Pending) {
        return newStatus != AppointmentStatus::Pending;
    }
    return newStatus != AppointmentStatus::Pending && newStatus != AppointmentStatus::Confirmed;
}

bool Appointment::overlapsInterval(const DateTime& otherStart, int otherDuration) const {
    DateTime otherEnd = otherStart.addMinutes(otherDuration);
    return start < otherEnd && otherStart < getEnd();
}

bool Appointment::operator==(const Appointment& other) const { return id == other.id; }
bool Appointment::operator<(const Appointment& other) const { return start < other.start; }
bool Appointment::operator>(const Appointment& other) const { return start > other.start; }

std::string Appointment::getEntityType() const { return "Appointment"; }

void Appointment::printInfo(std::ostream& os) const {
    os << getEntityType() << " #" << id << " | " << getTimeRange()
        << " | Client: " << client->getName() << (firstVisit ? " [NEW]" : "")
        << " | Master: " << master->getName()
        << " | " << service->getName()
        << " | Price: " << formatMoney(price)
        << " | " << statusToString(status) << (paid ? " [PAID]" : "");
}

std::string Appointment::classify() const {
    if (start.minuteOfDay() < 12 * 60) return "Morning";
    if (start.minuteOfDay() < 17 * 60) return "Afternoon";
    return "Evening";
}

std::ostream& operator<<(std::ostream& os, const Appointment& appointment) {
    appointment.printInfo(os);
    return os;
}
