#include "client.h"
#include "config.h"
#include "utils.h"
#include "console_input.h"
#include "exceptions/invalid_data_exception.h"

int Client::counter = 1;

Client::Client()
    : Person(0, "", ""), banned(false), visitsCount(0), noShowCount(0), cancelCount(0), totalSpent(0.0) {
}

Client::Client(std::string_view nameValue, std::string_view phoneValue)
    : Client(counter, nameValue, phoneValue, false, "", 0, 0, 0, 0.0) {
}

Client::Client(int idValue, std::string_view nameValue, std::string_view phoneValue, bool bannedValue,
    std::string_view reason, int visits, int noShows, int cancels, double spent)
    : Person(idValue, "", ""), banned(false), visitsCount(0), noShowCount(0), cancelCount(0), totalSpent(0.0) {
    if (idValue <= 0) throw InvalidDataException("номер клиента", "должно быть положительным");
    setName(nameValue);
    setPhone(phoneValue);
    if (visits < 0 || noShows < 0 || cancels < 0 || spent < 0) {
        throw InvalidDataException("статистика клиента", "счётчики и сумма не могут быть отрицательными");
    }
    visitsCount = visits;
    noShowCount = noShows;
    cancelCount = cancels;
    totalSpent = spent;
    if (bannedValue) ban(reason);
    if (counter <= idValue) counter = idValue + 1;
}

int Client::getVisitsCount() const { return visitsCount; }
int Client::getNoShowCount() const { return noShowCount; }
int Client::getCancelCount() const { return cancelCount; }
double Client::getTotalSpent() const { return totalSpent; }
std::string Client::getBanReason() const { return banReason; }
const std::vector<std::string>& Client::getNotes() const { return notes; }

bool Client::isBanned() const { return banned; }
bool Client::isNew() const { return visitsCount == 0; }

int Client::getDiscountPercent() const {
    if (visitsCount >= GOLD_VISITS) return GOLD_DISCOUNT;
    if (visitsCount >= SILVER_VISITS) return SILVER_DISCOUNT;
    if (visitsCount >= BRONZE_VISITS) return BRONZE_DISCOUNT;
    return 0;
}

void Client::ban(std::string_view reason) {
    if (isBlank(reason)) throw InvalidDataException("причина блокировки", "не может быть пустым");
    if (!isStorableText(reason)) throw InvalidDataException("причина блокировки", "не должно содержать «|» и переводов строки");
    banned = true;
    banReason = reason;
}

void Client::setVisitsCount(int visits) {
    if (visits < 0) throw InvalidDataException("число визитов", "не может быть отрицательным");
    visitsCount = visits;
}

void Client::unban() {
    banned = false;
    banReason.clear();
}

void Client::addNote(std::string_view text) {
    if (isBlank(text)) throw InvalidDataException("заметка", "не может быть пустым");
    if (!isStorableText(text)) throw InvalidDataException("заметка", "не должно содержать «|» и переводов строки");
    notes.push_back(std::string(text));
}

void Client::registerVisit(double amount) {
    if (amount < 0) throw InvalidDataException("сумма", "не может быть отрицательным");
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
    Client probe;
    std::string nameValue = askText(is, "Enter Client Name: ", probe, &Client::setName);
    std::string phoneValue = askText(is, "Enter Phone (+375XXXXXXXXX): ", probe, &Client::setPhone);
    client = Client(nameValue, phoneValue);
    return is;
}

void printClientCard(std::ostream& os, const Client& client, bool staffView) {
    os << "--- Client card ---\n";
    os << "Name: " << client.name << " | Phone: " << client.phone << "\n";
    if (!client.banned) {
        os << "Status: Active\n";
    }
    else if (staffView) {
        os << "Status: BANNED (" << client.banReason << ")\n";
    }
    else {
        os << "Status: restricted, please contact the barbershop administrator\n";
    }
    os << "Loyalty: " << (client.banned && !staffView ? std::string("-") : client.classify())
        << " | Discount: " << client.getDiscountPercent() << "%\n";
    os << "Visits: " << client.visitsCount;
    if (staffView) os << " | No-shows: " << client.noShowCount << " | Cancellations: " << client.cancelCount;
    os << " | Spent: " << formatMoney(client.totalSpent) << "\n";
    if (!staffView) return;
    os << "Private notes (visible to staff only):\n";
    if (client.notes.empty()) os << "  (none)\n";
    for (const std::string& note : client.notes) os << "  - " << note << "\n";
}
