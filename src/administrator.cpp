#include "administrator.h"
#include "config.h"
#include "utils.h"
#include "exceptions/invalid_data_exception.h"

int Administrator::counter = 0;

Administrator::Administrator() : Person(0, "", "") {}

Administrator::Administrator(std::string_view nameValue, std::string_view phoneValue, std::string_view loginValue,
    std::string_view password, std::string_view positionValue)
    : Person(++counter, "", "") {
    setName(nameValue);
    setPhone(phoneValue);
    setLogin(loginValue);
    setPassword(password);
    setPosition(positionValue);
}

std::string Administrator::getLogin() const { return login; }
std::string Administrator::getPosition() const { return position; }

void Administrator::setLogin(std::string_view loginValue) {
    if (!isValidLogin(loginValue)) {
        throw InvalidDataException("login", "3-20 characters: letters, digits or '_'");
    }
    login = loginValue;
}

void Administrator::setPassword(std::string_view password) {
    if (static_cast<int>(password.size()) < MIN_PASSWORD_LENGTH) {
        throw InvalidDataException("password", "must be at least " + std::to_string(MIN_PASSWORD_LENGTH) + " characters");
    }
    passwordHash = hashPassword(password);
}

void Administrator::setPosition(std::string_view positionValue) {
    if (isBlank(positionValue)) throw InvalidDataException("position", "must not be empty");
    position = positionValue;
}

bool Administrator::checkPassword(std::string_view password) const {
    return passwordHash == hashPassword(password);
}

bool Administrator::operator==(const Administrator& other) const { return login == other.login; }

std::string Administrator::getEntityType() const { return "Administrator"; }

std::string Administrator::classify() const {
    return position == "Owner" ? "Owner" : "Manager";
}

void Administrator::printInfo(std::ostream& os) const {
    os << getEntityType() << " ";
    Person::printInfo(os);
    os << " | Login: " << login << " | Position: " << position;
}

std::istream& operator>>(std::istream& is, Administrator& admin) {
    std::string nameValue;
    std::string phoneValue;
    std::string loginValue;
    std::string passwordValue;
    std::string positionValue;

    std::cout << "Enter Administrator Name: ";
    std::getline(is >> std::ws, nameValue);

    std::cout << "Enter Phone (+375XXXXXXXXX): ";
    std::getline(is >> std::ws, phoneValue);

    std::cout << "Enter Login: ";
    std::getline(is >> std::ws, loginValue);

    std::cout << "Enter Password: ";
    std::getline(is >> std::ws, passwordValue);

    std::cout << "Enter Position (Owner / Manager): ";
    std::getline(is >> std::ws, positionValue);
    if (!is) return is;

    admin = Administrator(nameValue, phoneValue, loginValue, passwordValue, positionValue);
    return is;
}
