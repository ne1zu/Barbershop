#pragma once
#include <string>
#include <string_view>
#include <memory>
#include <iostream>
#include "person.h"
#include "service.h"
#include "master_service.h"
#include "collection.h"
#include "datetime.h"

class Master : public Person {
private:
    std::string login;
    std::string passwordHash;
    int experience;
    double rating;
    int workStart;
    int workEnd;
    int dayOff;
    Collection<MasterService> offers;
    static int counter;

public:
    Master();
    Master(std::string_view name, std::string_view phone, std::string_view login,
        std::string_view password, int experience, double rating);

    std::string getLogin() const;
    int getExperience() const;
    double getRating() const;
    int getWorkStart() const;
    int getWorkEnd() const;
    int getDayOff() const;
    Collection<MasterService> getOffers() const;

    void setLogin(std::string_view loginValue);
    void setPassword(std::string_view password);
    void setExperience(int experienceValue);
    void setRating(double ratingValue);
    void setWorkHours(int startMinute, int endMinute);
    void setDayOff(int dayValue);

    bool checkPassword(std::string_view password) const;
    bool worksAt(const DateTime& start, int duration) const;

    void setOffer(std::shared_ptr<Service> service, double customPrice, int customDuration);
    bool removeOffer(int serviceId);
    std::shared_ptr<MasterService> findOffer(int serviceId) const;
    bool canPerform(const Service& service) const;
    double calculatePrice(const Service& service) const;
    int calculateDuration(const Service& service) const;

    bool operator==(const Master& other) const;
    bool operator<(const Master& other) const;
    bool operator>(const Master& other) const;

    std::string getEntityType() const override;
    std::string classify() const override;
    double getPriceFactor() const override;
    void printInfo(std::ostream& os) const override;

    friend std::istream& operator>>(std::istream& is, Master& master);
};
