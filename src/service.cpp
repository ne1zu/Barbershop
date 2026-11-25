#include "service.h"
#include "config.h"
#include "utils.h"
#include "console_input.h"
#include "exceptions/invalid_data_exception.h"

int Service::counter = 1;

Service::Service() : id(0), price(0.0), length(0) {}

Service::Service(std::string_view nameValue, double priceValue, int lengthValue)
    : Service(counter, nameValue, priceValue, lengthValue) {
}

Service::Service(int idValue, std::string_view nameValue, double priceValue, int lengthValue)
    : id(idValue), price(0.0), length(0) {
    if (idValue <= 0) throw InvalidDataException("номер услуги", "должно быть положительным");
    setName(nameValue);
    setPrice(priceValue);
    setLength(lengthValue);
    if (counter <= idValue) counter = idValue + 1;
}

int Service::getId() const { return id; }
std::string Service::getName() const { return name; }
double Service::getPrice() const { return price; }
int Service::getLength() const { return length; }

void Service::setName(std::string_view nameValue) {
    if (isBlank(nameValue)) throw InvalidDataException("название услуги", "не может быть пустым");
    if (!isStorableText(nameValue)) {
        throw InvalidDataException("название услуги", "не должно содержать «|» и переводов строки");
    }
    name = nameValue;
}

void Service::setPrice(double priceValue) {
    double rounded = roundMoney(priceValue);
    if (rounded <= 0) throw InvalidDataException("цена", "должна быть положительной, указано " + formatMoney(priceValue));
    price = rounded;
}

void Service::setLength(int lengthValue) {
    if (lengthValue <= 0 || lengthValue > MAX_SERVICE_MINUTES) {
        throw InvalidDataException("длительность", "от 1 до " + std::to_string(MAX_SERVICE_MINUTES)
            + " minutes, got " + std::to_string(lengthValue));
    }
    if (lengthValue % GRID_STEP_MINUTES != 0) {
        throw InvalidDataException("длительность", "кратна " + std::to_string(GRID_STEP_MINUTES) + " минутам");
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
    Service probe;
    std::string nameValue = askText(is, "Enter Service Name: ", probe, &Service::setName);
    double priceValue = askNumber(is, "Enter Base Price: ", probe, &Service::setPrice);
    int lengthValue = askNumber(is, "Enter Duration (minutes, multiple of 15): ", probe, &Service::setLength);
    service = Service(nameValue, priceValue, lengthValue);
    return is;
}
