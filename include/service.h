#pragma once
#include <string>
#include <string_view>
#include <iostream>

class Service {
private:
    int id;
    std::string name;
    double price;
    int length;
    static int counter;

public:
    Service();
    Service(std::string_view nameValue, double priceValue, int lengthValue);

    int getId() const;
    std::string getName() const;
    double getPrice() const;
    int getLength() const;

    void setName(std::string_view nameValue);
    void setPrice(double priceValue);
    void setLength(int lengthValue);

    bool operator==(const Service& other) const;
    bool operator<(const Service& other) const;
    bool operator>(const Service& other) const;

    std::string getEntityType() const;
    void printInfo(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, const Service& service);
    friend std::istream& operator>>(std::istream& is, Service& service);
};
