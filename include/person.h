#pragma once
#include <string>
#include <string_view>
#include <iostream>

class Person {
protected:
    int id;
    std::string name;
    std::string phone;

public:
    Person(int idValue, std::string_view nameValue, std::string_view phoneValue);
    virtual ~Person() = default;

    int getId() const;
    std::string getName() const;
    std::string getPhone() const;
    void setName(std::string_view nameValue);
    void setPhone(std::string_view phoneValue);

    virtual std::string getEntityType() const = 0;
    virtual std::string classify() const = 0;
    virtual double getPriceFactor() const;
    virtual void printInfo(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, const Person& entity);
};
