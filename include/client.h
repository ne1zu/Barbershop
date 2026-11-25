#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include "person.h"

class Client : public Person {
private:
    bool banned;
    std::string banReason;
    int visitsCount;
    int noShowCount;
    int cancelCount;
    double totalSpent;
    std::vector<std::string> notes;
    static int counter;

public:
    Client();
    Client(std::string_view name, std::string_view phone);
    Client(int idValue, std::string_view name, std::string_view phone, bool bannedValue,
        std::string_view reason, int visits, int noShows, int cancels, double spent);

    int getVisitsCount() const;
    int getNoShowCount() const;
    int getCancelCount() const;
    double getTotalSpent() const;
    std::string getBanReason() const;
    const std::vector<std::string>& getNotes() const;

    bool isBanned() const;
    bool isNew() const;
    int getDiscountPercent() const;

    void ban(std::string_view reason);
    void unban();
    void addNote(std::string_view text);
    void setVisitsCount(int visits);
    void registerVisit(double amount);
    void registerNoShow();
    void registerCancel();

    bool operator==(const Client& other) const;
    bool operator<(const Client& other) const;
    bool operator>(const Client& other) const;

    std::string getEntityType() const override;
    std::string classify() const override;
    double getPriceFactor() const override;
    void printInfo(std::ostream& os) const override;

    friend std::istream& operator>>(std::istream& is, Client& client);
    friend void printClientCard(std::ostream& os, const Client& client, bool staffView);
};
