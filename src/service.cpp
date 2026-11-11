#include "service.h"
#include "config.h"
#include "utils.h"
#include "exceptions/invalid_data_exception.h"

int Service::counter = 0;

Service::Service() : id(0), price(0.0), length(0) {}

Service::Service(std::string_view nameValue, double priceValue, int lengthValue)
    : id(++counter), price(0.0), length(0) {
    setName(nameValue);
    setPrice(priceValue);
    setLength(lengthValue);
}

int Service::getId() const { return id; }
std::string Service::getName() const { return name; }
double Service::getPrice() const { return price; }
int Service::getLength() const { return length; }

void Service::setName(std::string_view nameValue) {
    if (isBlank(nameValue)) throw InvalidDataException("service name", "must not be empty");
    name = nameValue;
}

void Service::setPrice(double priceValue) {
    if (priceValue <= 0) throw InvalidDataException("price", "must be positive, got " + formatMoney(priceValue));
    price = priceValue;
}

void Service::setLength(int lengthValue) {
    if (lengthValue <= 0 || lengthValue > MAX_SERVICE_MINUTES) {
        throw InvalidDataException("duration", "must be 1-" + std::to_string(MAX_SERVICE_MINUTES)
            + " minutes, got " + std::to_string(lengthValue));
    }
    if (lengthValue % GRID_STEP_MINUTES != 0) {
        throw InvalidDataException("duration", "must be a multiple of " + std::to_string(GRID_STEP_MINUTES));
    }
    length = lengthValue;
}

bool Service::operator==(const Service& other) const { return name == other.name; }
bool Service::operator<(const Service& other) const { return price < other.price; }
bool Service::operator>(const Service& other) const { return price > other.price; }

std::string Service::getEntityType() const { return "Service"; }

void Service::printInfo(std::ostream& os) const {
    os << getEntityType() << " #" << id << " | " << name << " | Base price: " << formatMoney(price)
        << " | Duration: " << length << " min";
}

std::ostream& operator<<(std::ostream& os, const Service& service) {
    service.printInfo(os);
    return os;
}

std::istream& operator>>(std::istream& is, Service& service) {
    std::string nameValue;
    double priceValue = 0.0;
    int lengthValue = 0;

    std::cout << "Enter Service Name: ";
    std::getline(is >> std::ws, nameValue);

    std::cout << "Enter Base Price: ";
    is >> priceValue;
    if (!is) return is;

    std::cout << "Enter Duration (minutes): ";
    is >> lengthValue;
    if (!is) return is;

    service = Service(nameValue, priceValue, lengthValue);
    return is;
}
