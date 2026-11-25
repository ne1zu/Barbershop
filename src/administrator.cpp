#include "administrator.h"
#include "config.h"
#include "utils.h"
#include "console_input.h"
#include "exceptions/invalid_data_exception.h"

int Administrator::counter = 1;

Administrator::Administrator() : Person(0, "", "") {}

Administrator::Administrator(std::string_view nameValue, std::string_view phoneValue, std::string_view loginValue,
    std::string_view password, std::string_view positionValue)
    : Administrator(counter, nameValue, phoneValue, loginValue, makePasswordHash(password), positionValue) {
}

Administrator::Administrator(int idValue, std::string_view nameValue, std::string_view phoneValue,
    std::string_view loginValue, std::string_view hash, std::string_view positionValue)
    : Person(idValue, "", "") {
    if (idValue <= 0) throw InvalidDataException("номер администратора", "должно быть положительным");
    if (!isDigits(hash)) throw InvalidDataException("хеш пароля", "повреждён");
    setName(nameValue);
    setPhone(phoneValue);
    setLogin(loginValue);
    passwordHash = hash;
    setPosition(positionValue);
    if (counter <= idValue) counter = idValue + 1;
}

std::string Administrator::getLogin() const { return login; }
std::string Administrator::getPasswordHash() const { return passwordHash; }
std::string Administrator::getPosition() const { return position; }

void Administrator::setLogin(std::string_view loginValue) {
    if (!isValidLogin(loginValue)) {
        throw InvalidDataException("логин", "3–20 символов: латинские буквы, цифры или «_»");
    }
    login = loginValue;
}

void Administrator::setPassword(std::string_view password) {
    passwordHash = makePasswordHash(password);
}

void Administrator::setPosition(std::string_view positionValue) {
    if (isBlank(positionValue)) throw InvalidDataException("должность", "не может быть пустым");
    if (!isStorableText(positionValue)) {
        throw InvalidDataException("должность", "не должно содержать «|» и переводов строки");
    }
    position = positionValue;
}

bool Administrator::checkPassword(std::string_view password) const {
    return passwordHash == hashPassword(password);
}

bool Administrator::operator==(const Administrator& other) const { return login == other.login; }

std::string Administrator::getEntityType() const { return "Administrator"; }

std::string Administrator::classify() const {
    return position == "Владелец" ? "Owner" : "Manager";
}

void Administrator::printInfo(std::ostream& os) const {
    os << getEntityType() << " ";
    Person::printInfo(os);
    os << " | Login: " << login << " | Position: " << position;
}

std::istream& operator>>(std::istream& is, Administrator& admin) {
    Administrator probe;
    std::string nameValue = askText(is, "Enter Administrator Name: ", probe, &Administrator::setName);
    std::string phoneValue = askText(is, "Enter Phone (+375XXXXXXXXX): ", probe, &Administrator::setPhone);
    std::string loginValue = askText(is, "Enter Login: ", probe, &Administrator::setLogin);
    std::string passwordValue = askText(is, "Enter Password: ", probe, &Administrator::setPassword);
    std::string positionValue = askText(is, "Enter Position (Owner / Manager): ", probe, &Administrator::setPosition);
    admin = Administrator(nameValue, phoneValue, loginValue, passwordValue, positionValue);
    return is;
}
