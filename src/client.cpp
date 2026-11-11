#include "client.h"
#include "config.h"
#include "utils.h"
#include "exceptions/invalid_data_exception.h"

int Client::counter = 0;

Client::Client()
    : Person(0, "", ""), banned(false), visitsCount(0), noShowCount(0), cancelCount(0), totalSpent(0.0) {
}

Client::Client(std::string_view nameValue, std::string_view phoneValue)
    : Person(++counter, "", ""), banned(false), visitsCount(0), noShowCount(0), cancelCount(0), totalSpent(0.0) {
    setName(nameValue);
    setPhone(phoneValue);
}

int Client::getVisitsCount() const { return visitsCount; }
int Client::getNoShowCount() const { return noShowCount; }
int Client::getCancelCount() const { return cancelCount; }
double Client::getTotalSpent() const { return totalSpent; }
std::string Client::getBanReason() const { return banReason; }

bool Client::isBanned() const { return banned; }
bool Client::isNew() const { return visitsCount == 0; }

int Client::getDiscountPercent() const {
    if (visitsCount >= GOLD_VISITS) return GOLD_DISCOUNT;
    if (visitsCount >= SILVER_VISITS) return SILVER_DISCOUNT;
    if (visitsCount >= BRONZE_VISITS) return BRONZE_DISCOUNT;
    return 0;
}

void Client::ban(std::string_view reason) {
    if (isBlank(reason)) throw InvalidDataException("ban reason", "must not be empty");
    banned = true;
    banReason = reason;
}

void Client::unban() {
    banned = false;
    banReason.clear();
}

void Client::addNote(std::string_view text) {
    if (isBlank(text)) throw InvalidDataException("note", "must not be empty");
    notes.push_back(std::string(text));
}

void Client::registerVisit(double amount) {
    if (amount < 0) throw InvalidDataException("amount", "must not be negative");
    visitsCount++;
    totalSpent += amount;
}

void Client::registerNoShow() { noShowCount++; }
void Client::registerCancel() { cancelCount++; }

bool Client::operator==(const Client& other) const { return phone == other.phone; }
bool Client::operator<(const Client& other) const { return visitsCount < other.visitsCount; }
bool Client::operator>(const Client& other) const { return visitsCount > other.visitsCount; }

std::string Client::getEntityType() const { return "Client"; }

std::string Client::classify() const {
    if (banned) return "Banned";
    if (visitsCount >= GOLD_VISITS) return "Gold";
    if (visitsCount >= SILVER_VISITS) return "Silver";
    if (visitsCount >= BRONZE_VISITS) return "Bronze";
    if (visitsCount == 0) return "New";
    return "Regular";
}

double Client::getPriceFactor() const {
    return (100 - getDiscountPercent()) / 100.0;
}

void Client::printInfo(std::ostream& os) const {
    os << getEntityType() << " ";
    Person::printInfo(os);
    os << " | Visits: " << visitsCount << " | Discount: " << getDiscountPercent() << "%";
    if (banned) os << " | BANNED";
}

std::istream& operator>>(std::istream& is, Client& client) {
    std::string nameValue;
    std::string phoneValue;

    std::cout << "Enter Client Name: ";
    std::getline(is >> std::ws, nameValue);

    std::cout << "Enter Phone (+375XXXXXXXXX): ";
    std::getline(is >> std::ws, phoneValue);
    if (!is) return is;

    client = Client(nameValue, phoneValue);
    return is;
}

void printClientCard(std::ostream& os, const Client& client) {
    os << "--- Client card ---\n";
    os << "Name: " << client.name << " | Phone: " << client.phone << "\n";
    os << "Status: " << (client.banned ? "BANNED (" + client.banReason + ")" : std::string("Active")) << "\n";
    os << "Loyalty: " << client.classify() << " | Discount: " << client.getDiscountPercent() << "%\n";
    os << "Visits: " << client.visitsCount << " | No-shows: " << client.noShowCount
        << " | Cancellations: " << client.cancelCount << " | Spent: " << formatMoney(client.totalSpent) << "\n";
    os << "Private notes (visible to staff only):\n";
    if (client.notes.empty()) os << "  (none)\n";
    for (const std::string& note : client.notes) os << "  - " << note << "\n";
}
