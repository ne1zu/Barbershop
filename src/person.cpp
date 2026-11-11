#include "person.h"
#include "utils.h"
#include "exceptions/invalid_data_exception.h"

Person::Person(int idValue, std::string_view nameValue, std::string_view phoneValue)
    : id(idValue), name(nameValue), phone(phoneValue) {
}

int Person::getId() const { return id; }
std::string Person::getName() const { return name; }
std::string Person::getPhone() const { return phone; }

void Person::setName(std::string_view nameValue) {
    if (isBlank(nameValue)) throw InvalidDataException("name", "must not be empty");
    name = nameValue;
}

void Person::setPhone(std::string_view phoneValue) {
    if (!isValidPhone(phoneValue)) throw InvalidDataException("phone", "expected format +375XXXXXXXXX");
    phone = phoneValue;
}

double Person::getPriceFactor() const { return 1.0; }

void Person::printInfo(std::ostream& os) const {
    os << "#" << id << " | Name: " << name << " | Phone: " << phone;
}

std::ostream& operator<<(std::ostream& os, const Person& entity) {
    entity.printInfo(os);
    return os;
}
