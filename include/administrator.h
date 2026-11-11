#pragma once
#include <string>
#include <string_view>
#include <iostream>
#include "person.h"

class Administrator : public Person {
private:
    std::string login;
    std::string passwordHash;
    std::string position;
    static int counter;

public:
    Administrator();
    Administrator(std::string_view name, std::string_view phone, std::string_view login,
        std::string_view password, std::string_view position);

    std::string getLogin() const;
    std::string getPosition() const;

    void setLogin(std::string_view loginValue);
    void setPassword(std::string_view password);
    void setPosition(std::string_view positionValue);

    bool checkPassword(std::string_view password) const;

    bool operator==(const Administrator& other) const;

    std::string getEntityType() const override;
    std::string classify() const override;
    void printInfo(std::ostream& os) const override;

    friend std::istream& operator>>(std::istream& is, Administrator& admin);
};
