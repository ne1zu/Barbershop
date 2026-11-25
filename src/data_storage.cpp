#include "data_storage.h"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>
#include "exceptions/exceptions.h"

using namespace std;

namespace {
    const char FIELD_SEPARATOR = '|';
    const string FORMAT_HEADER = "BARBERSHOP_STATE";
    const int FORMAT_VERSION = 1;
    const int MAX_ID = 1000000000;

    const string SHOP_SECTION = "SHOP";
    const string SERVICES_SECTION = "SERVICES";
    const string CLIENTS_SECTION = "CLIENTS";
    const string NOTES_SECTION = "CLIENT_NOTES";
    const string MASTERS_SECTION = "MASTERS";
    const string OFFERS_SECTION = "OFFERS";
    const string ADMINISTRATORS_SECTION = "ADMINISTRATORS";
    const string APPOINTMENTS_SECTION = "APPOINTMENTS";
    const string PAYMENTS_SECTION = "PAYMENTS";

    const size_t HEADER_FIELD_COUNT = 2;
    const size_t SECTION_FIELD_COUNT = 2;
    const size_t SHOP_FIELD_COUNT = 2;
    const size_t SERVICE_FIELD_COUNT = 4;
    const size_t CLIENT_FIELD_COUNT = 9;
    const size_t NOTE_FIELD_COUNT = 2;
    const size_t MASTER_FIELD_COUNT = 10;
    const size_t OFFER_FIELD_COUNT = 4;
    const size_t ADMINISTRATOR_FIELD_COUNT = 6;
    const size_t APPOINTMENT_FIELD_COUNT = 10;
    const size_t PAYMENT_FIELD_COUNT = 6;

    struct LoadContext {
        ifstream file;
        string filename;
        int lineNumber;
        unique_ptr<Barbershop> shop;
        unordered_map<int, shared_ptr<Client>> clients;
        unordered_map<int, shared_ptr<Master>> masters;
        unordered_map<int, shared_ptr<Service>> services;
        unordered_map<int, shared_ptr<Appointment>> appointments;
        set<int> paidAppointmentIds;

        explicit LoadContext(const string& name) : file(name), filename(name), lineNumber(0) {}
    };

    int countNotes(const Barbershop& shop) {
        int total = 0;
        for (const auto& client : shop.getClients()) total += static_cast<int>(client->getNotes().size());
        return total;
    }

    int countOffers(const Barbershop& shop) {
        int total = 0;
        for (const auto& master : shop.getMasters()) total += master->getOffers().getCount();
        return total;
    }

    void writeSectionHeader(ostream& os, const string& name, int count) {
        os << name << FIELD_SEPARATOR << count << '\n';
    }

    void writeShop(ostream& os, const Barbershop& shop) {
        writeSectionHeader(os, SHOP_SECTION, 1);
        os << shop.getName() << FIELD_SEPARATOR << shop.getAddress() << '\n';
    }

    void writeServices(ostream& os, const Barbershop& shop) {
        writeSectionHeader(os, SERVICES_SECTION, shop.getServices().getCount());
        for (const auto& service : shop.getServices()) {
            os << service->getId() << FIELD_SEPARATOR << service->getName() << FIELD_SEPARATOR
                << service->getPrice() << FIELD_SEPARATOR << service->getLength() << '\n';
        }
    }

    void writeClients(ostream& os, const Barbershop& shop) {
        writeSectionHeader(os, CLIENTS_SECTION, shop.getClients().getCount());
        for (const auto& client : shop.getClients()) {
            os << client->getId() << FIELD_SEPARATOR << client->getName() << FIELD_SEPARATOR
                << client->getPhone() << FIELD_SEPARATOR << (client->isBanned() ? 1 : 0) << FIELD_SEPARATOR
                << client->getBanReason() << FIELD_SEPARATOR << client->getVisitsCount() << FIELD_SEPARATOR
                << client->getNoShowCount() << FIELD_SEPARATOR << client->getCancelCount() << FIELD_SEPARATOR
                << client->getTotalSpent() << '\n';
        }
    }

    void writeNotes(ostream& os, const Barbershop& shop) {
        writeSectionHeader(os, NOTES_SECTION, countNotes(shop));
        for (const auto& client : shop.getClients()) {
            for (const string& note : client->getNotes()) {
                os << client->getId() << FIELD_SEPARATOR << note << '\n';
            }
        }
    }

    void writeMasters(ostream& os, const Barbershop& shop) {
        writeSectionHeader(os, MASTERS_SECTION, shop.getMasters().getCount());
        for (const auto& master : shop.getMasters()) {
            os << master->getId() << FIELD_SEPARATOR << master->getName() << FIELD_SEPARATOR
                << master->getPhone() << FIELD_SEPARATOR << master->getLogin() << FIELD_SEPARATOR
                << master->getPasswordHash() << FIELD_SEPARATOR << master->getExperience() << FIELD_SEPARATOR
                << master->getRating() << FIELD_SEPARATOR << master->getWorkStart() << FIELD_SEPARATOR
                << master->getWorkEnd() << FIELD_SEPARATOR << master->getDayOff() << '\n';
        }
    }

    void writeOffers(ostream& os, const Barbershop& shop) {
        writeSectionHeader(os, OFFERS_SECTION, countOffers(shop));
        for (const auto& master : shop.getMasters()) {
            for (const auto& offer : master->getOffers()) {
                os << master->getId() << FIELD_SEPARATOR << offer->getService()->getId() << FIELD_SEPARATOR
                    << offer->getCustomPrice() << FIELD_SEPARATOR << offer->getCustomDuration() << '\n';
            }
        }
    }

    void writeAdministrators(ostream& os, const Barbershop& shop) {
        writeSectionHeader(os, ADMINISTRATORS_SECTION, shop.getAdministrators().getCount());
        for (const auto& admin : shop.getAdministrators()) {
            os << admin->getId() << FIELD_SEPARATOR << admin->getName() << FIELD_SEPARATOR
                << admin->getPhone() << FIELD_SEPARATOR << admin->getLogin() << FIELD_SEPARATOR
                << admin->getPasswordHash() << FIELD_SEPARATOR << admin->getPosition() << '\n';
        }
    }

    void writeAppointments(ostream& os, const Barbershop& shop) {
        writeSectionHeader(os, APPOINTMENTS_SECTION, shop.getAppointments().getCount());
        for (const auto& appointment : shop.getAppointments()) {
            os << appointment->getId() << FIELD_SEPARATOR << appointment->getClient()->getId() << FIELD_SEPARATOR
                << appointment->getMaster()->getId() << FIELD_SEPARATOR << appointment->getService()->getId()
                << FIELD_SEPARATOR << appointment->getPrice() << FIELD_SEPARATOR << appointment->getDuration()
                << FIELD_SEPARATOR << appointment->getStart().toString() << FIELD_SEPARATOR
                << statusToString(appointment->getStatus()) << FIELD_SEPARATOR
                << (appointment->isFirstVisit() ? 1 : 0) << FIELD_SEPARATOR
                << (appointment->isPaid() ? 1 : 0) << '\n';
        }
    }

    void writePayments(ostream& os, const Barbershop& shop) {
        writeSectionHeader(os, PAYMENTS_SECTION, shop.getPayments().getCount());
        for (const auto& payment : shop.getPayments()) {
            os << payment->getId() << FIELD_SEPARATOR << payment->getAppointment()->getId() << FIELD_SEPARATOR
                << payment->getAmount() << FIELD_SEPARATOR << payment->getDiscountPercent() << FIELD_SEPARATOR
                << paymentMethodToString(payment->getMethod()) << FIELD_SEPARATOR
                << payment->getPaidAt().toString() << '\n';
        }
    }

    string readLine(LoadContext& ctx) {
        string line;
        if (!getline(ctx.file, line)) {
            if (!ctx.file.eof()) throw FileIOException(ctx.filename, "чтение");
            throw FileFormatException(ctx.lineNumber + 1, "неожиданный конец файла");
        }
        ctx.lineNumber++;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        return line;
    }

    vector<string> splitLine(const string& line) {
        vector<string> fields;
        size_t start = 0;
        size_t position = line.find(FIELD_SEPARATOR);
        while (position != string::npos) {
            fields.push_back(line.substr(start, position - start));
            start = position + 1;
            position = line.find(FIELD_SEPARATOR, start);
        }
        fields.push_back(line.substr(start));
        return fields;
    }

    vector<string> readFields(LoadContext& ctx, size_t expectedCount) {
        vector<string> fields = splitLine(readLine(ctx));
        if (fields.size() != expectedCount) {
            throw FileFormatException(ctx.lineNumber,
                "ожидалось полей: " + to_string(expectedCount) + ", найдено: " + to_string(fields.size()));
        }
        return fields;
    }

    int parseInt(const string& text, const LoadContext& ctx, const string& field) {
        istringstream in(text);
        int value = 0;
        if (text.empty() || !(in >> value) || !in.eof()) {
            throw FileFormatException(ctx.lineNumber,
                "поле «" + field + "» должно быть целым числом, найдено \"" + text + "\"");
        }
        return value;
    }

    double parseDouble(const string& text, const LoadContext& ctx, const string& field) {
        istringstream in(text);
        double value = 0.0;
        if (text.empty() || !(in >> value) || !in.eof() || !isfinite(value)) {
            throw FileFormatException(ctx.lineNumber,
                "поле «" + field + "» должно быть числом, найдено \"" + text + "\"");
        }
        return value;
    }

    int parseId(const string& text, const LoadContext& ctx, const string& field) {
        int value = parseInt(text, ctx, field);
        if (value < 1 || value > MAX_ID) {
            throw FileFormatException(ctx.lineNumber,
                "поле «" + field + "» должно быть от 1 до " + to_string(MAX_ID) + ", найдено " + text);
        }
        return value;
    }

    bool parseFlag(const string& text, const LoadContext& ctx, const string& field) {
        if (text != "0" && text != "1") {
            throw FileFormatException(ctx.lineNumber,
                "поле «" + field + "» должно быть 0 или 1, найдено \"" + text + "\"");
        }
        return text == "1";
    }

    DateTime parseDateTime(const string& text, const LoadContext& ctx, const string& field) {
        size_t space = text.find(' ');
        int day = 0;
        int month = 0;
        int year = 0;
        int hour = 0;
        int minute = 0;
        if (space == string::npos || !DateTime::parseDate(text.substr(0, space), day, month, year)
            || !DateTime::parseTime(text.substr(space + 1), hour, minute)) {
            throw FileFormatException(ctx.lineNumber,
                "поле «" + field + "» должно иметь вид дд.мм.гггг чч:мм, найдено \"" + text + "\"");
        }
        return DateTime(year, month, day, hour, minute);
    }

    template <typename T>
    void requireNewId(const unordered_map<int, shared_ptr<T>>& items, int id, const LoadContext& ctx,
        const string& kind) {
        if (items.count(id) > 0) {
            throw FileFormatException(ctx.lineNumber, "повторяющийся номер (" + kind + "): " + to_string(id));
        }
    }

    template <typename T>
    shared_ptr<T> requireExisting(const unordered_map<int, shared_ptr<T>>& items, int id, const LoadContext& ctx,
        const string& kind) {
        typename unordered_map<int, shared_ptr<T>>::const_iterator found = items.find(id);
        if (found == items.end()) {
            throw FileFormatException(ctx.lineNumber, "не существует объект (" + kind + ") с номером " + to_string(id));
        }
        return found->second;
    }

    void readHeader(LoadContext& ctx) {
        vector<string> fields = splitLine(readLine(ctx));
        if (fields[0] != FORMAT_HEADER) {
            throw FileFormatException(ctx.lineNumber,
                "это не файл состояния барбершопа (ожидался заголовок «" + FORMAT_HEADER + "»)");
        }
        if (fields.size() != HEADER_FIELD_COUNT) {
            throw FileFormatException(ctx.lineNumber,
                "ожидалось полей: " + to_string(HEADER_FIELD_COUNT) + ", найдено: " + to_string(fields.size()));
        }
        int version = parseInt(fields[1], ctx, "format version");
        if (version != FORMAT_VERSION) {
            throw FileFormatException(ctx.lineNumber,
                "неподдерживаемая версия формата " + to_string(version) + " (поддерживается: "
                + to_string(FORMAT_VERSION) + ")");
        }
    }

    int readSectionHeader(LoadContext& ctx, const string& sectionName) {
        vector<string> fields = readFields(ctx, SECTION_FIELD_COUNT);
        if (fields[0] != sectionName) {
            throw FileFormatException(ctx.lineNumber,
                "ожидался раздел «" + sectionName + "», найдено \"" + fields[0] + "\"");
        }
        int count = parseInt(fields[1], ctx, "number of records");
        if (count < 0) {
            throw FileFormatException(ctx.lineNumber,
                "число записей не может быть отрицательным, найдено " + to_string(count));
        }
        return count;
    }

    void readShop(LoadContext& ctx) {
        int count = readSectionHeader(ctx, SHOP_SECTION);
        if (count != 1) {
            throw FileFormatException(ctx.lineNumber,
                "раздел " + SHOP_SECTION + " должен содержать ровно одну запись");
        }
        vector<string> f = readFields(ctx, SHOP_FIELD_COUNT);
        ctx.shop = make_unique<Barbershop>(f[0], f[1]);
    }

    void readServices(LoadContext& ctx) {
        int count = readSectionHeader(ctx, SERVICES_SECTION);
        for (int i = 0; i < count; i++) {
            vector<string> f = readFields(ctx, SERVICE_FIELD_COUNT);
            int id = parseId(f[0], ctx, "service id");
            requireNewId(ctx.services, id, ctx, "Service");
            double price = parseDouble(f[2], ctx, "price");
            int length = parseInt(f[3], ctx, "duration");
            shared_ptr<Service> service = make_shared<Service>(id, f[1], price, length);
            *ctx.shop += service;
            ctx.services[id] = service;
        }
    }

    void readClients(LoadContext& ctx) {
        int count = readSectionHeader(ctx, CLIENTS_SECTION);
        for (int i = 0; i < count; i++) {
            vector<string> f = readFields(ctx, CLIENT_FIELD_COUNT);
            int id = parseId(f[0], ctx, "client id");
            requireNewId(ctx.clients, id, ctx, "Client");
            bool banned = parseFlag(f[3], ctx, "banned");
            if (!banned && !f[4].empty()) {
                throw FileFormatException(ctx.lineNumber, "у незаблокированного клиента не может быть причины блокировки");
            }
            int visits = parseInt(f[5], ctx, "visits");
            int noShows = parseInt(f[6], ctx, "no-shows");
            int cancels = parseInt(f[7], ctx, "cancellations");
            double spent = parseDouble(f[8], ctx, "spent");
            shared_ptr<Client> client = make_shared<Client>(id, f[1], f[2], banned, f[4], visits, noShows,
                cancels, spent);
            *ctx.shop += client;
            ctx.clients[id] = client;
        }
    }

    void readNotes(LoadContext& ctx) {
        int count = readSectionHeader(ctx, NOTES_SECTION);
        for (int i = 0; i < count; i++) {
            vector<string> f = readFields(ctx, NOTE_FIELD_COUNT);
            requireExisting(ctx.clients, parseId(f[0], ctx, "client id"), ctx, "Client")->addNote(f[1]);
        }
    }

    void readMasters(LoadContext& ctx) {
        int count = readSectionHeader(ctx, MASTERS_SECTION);
        for (int i = 0; i < count; i++) {
            vector<string> f = readFields(ctx, MASTER_FIELD_COUNT);
            int id = parseId(f[0], ctx, "master id");
            requireNewId(ctx.masters, id, ctx, "Master");
            int experience = parseInt(f[5], ctx, "experience");
            double rating = parseDouble(f[6], ctx, "rating");
            int workStart = parseInt(f[7], ctx, "work start");
            int workEnd = parseInt(f[8], ctx, "work end");
            int dayOff = parseInt(f[9], ctx, "day off");
            shared_ptr<Master> master = make_shared<Master>(id, f[1], f[2], f[3], f[4], experience, rating,
                workStart, workEnd, dayOff);
            *ctx.shop += master;
            ctx.masters[id] = master;
        }
    }

    void readOffers(LoadContext& ctx) {
        int count = readSectionHeader(ctx, OFFERS_SECTION);
        for (int i = 0; i < count; i++) {
            vector<string> f = readFields(ctx, OFFER_FIELD_COUNT);
            shared_ptr<Master> master = requireExisting(ctx.masters, parseId(f[0], ctx, "master id"), ctx, "Master");
            shared_ptr<Service> service = requireExisting(ctx.services, parseId(f[1], ctx, "service id"), ctx, "Service");
            double price = parseDouble(f[2], ctx, "custom price");
            int duration = parseInt(f[3], ctx, "custom duration");
            if (master->canPerform(*service)) {
                throw FileFormatException(ctx.lineNumber,
                    "мастер «" + master->getName() + "» уже оказывает услугу «" + service->getName() + "»");
            }
            master->setOffer(service, price, duration);
        }
    }

    void readAdministrators(LoadContext& ctx) {
        int count = readSectionHeader(ctx, ADMINISTRATORS_SECTION);
        if (count < 1) {
            throw FileFormatException(ctx.lineNumber, "в барбершопе должен быть хотя бы один администратор");
        }
        set<int> usedIds;
        for (int i = 0; i < count; i++) {
            vector<string> f = readFields(ctx, ADMINISTRATOR_FIELD_COUNT);
            int id = parseId(f[0], ctx, "administrator id");
            if (!usedIds.insert(id).second) {
                throw FileFormatException(ctx.lineNumber, "повторяющийся номер администратора: " + to_string(id));
            }
            *ctx.shop += make_shared<Administrator>(id, f[1], f[2], f[3], f[4], f[5]);
        }
    }

    void readAppointments(LoadContext& ctx) {
        int count = readSectionHeader(ctx, APPOINTMENTS_SECTION);
        for (int i = 0; i < count; i++) {
            vector<string> f = readFields(ctx, APPOINTMENT_FIELD_COUNT);
            int id = parseId(f[0], ctx, "appointment id");
            requireNewId(ctx.appointments, id, ctx, "Appointment");
            shared_ptr<Client> client = requireExisting(ctx.clients, parseId(f[1], ctx, "client id"), ctx, "Client");
            shared_ptr<Master> master = requireExisting(ctx.masters, parseId(f[2], ctx, "master id"), ctx, "Master");
            shared_ptr<Service> service = requireExisting(ctx.services, parseId(f[3], ctx, "service id"), ctx, "Service");
            double price = parseDouble(f[4], ctx, "price");
            int duration = parseInt(f[5], ctx, "duration");
            DateTime start = parseDateTime(f[6], ctx, "start");
            AppointmentStatus status = AppointmentStatus::Pending;
            if (!parseStatus(f[7], status)) {
                throw FileFormatException(ctx.lineNumber, "неизвестный статус \"" + f[7] + "\"");
            }
            bool firstVisit = parseFlag(f[8], ctx, "first visit");
            bool paid = parseFlag(f[9], ctx, "paid");
            shared_ptr<Appointment> appointment = make_shared<Appointment>(id, client, master, service, price,
                duration, start, status, firstVisit, paid);
            ctx.shop->restoreAppointment(appointment);
            ctx.appointments[id] = appointment;
        }
    }

    void readPayments(LoadContext& ctx) {
        int count = readSectionHeader(ctx, PAYMENTS_SECTION);
        for (int i = 0; i < count; i++) {
            vector<string> f = readFields(ctx, PAYMENT_FIELD_COUNT);
            int id = parseId(f[0], ctx, "payment id");
            int appointmentId = parseId(f[1], ctx, "appointment id");
            shared_ptr<Appointment> appointment = requireExisting(ctx.appointments, appointmentId, ctx, "Appointment");
            double amount = parseDouble(f[2], ctx, "amount");
            int discount = parseInt(f[3], ctx, "discount");
            PaymentMethod method = PaymentMethod::Cash;
            if (!parsePaymentMethod(f[4], method)) {
                throw FileFormatException(ctx.lineNumber, "неизвестный способ оплаты \"" + f[4] + "\"");
            }
            DateTime paidAt = parseDateTime(f[5], ctx, "payment time");
            ctx.shop->restorePayment(make_shared<Payment>(id, appointment, amount, discount, method, paidAt));
            ctx.paidAppointmentIds.insert(appointmentId);
        }
        for (const auto& entry : ctx.appointments) {
            if (entry.second->isPaid() && ctx.paidAppointmentIds.count(entry.first) == 0) {
                throw FileFormatException(ctx.lineNumber,
                    "запись №" + to_string(entry.first) + " отмечена как оплаченная, но платежа нет");
            }
        }
    }

    void ensureNoTrailingData(LoadContext& ctx) {
        string line;
        while (getline(ctx.file, line)) {
            ctx.lineNumber++;
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (!line.empty()) {
                throw FileFormatException(ctx.lineNumber, "лишние данные после последней записи");
            }
        }
        if (!ctx.file.eof()) throw FileIOException(ctx.filename, "чтение");
    }
}

DataStorage::DataStorage(const string& filenameValue) : filename(filenameValue) {}

void DataStorage::save(const Barbershop& shop) const {
    const string temporaryName = filename + ".tmp";
    ofstream file(temporaryName);
    if (!file) throw FileOpenException(temporaryName, "запись");

    file << setprecision(10);
    file << FORMAT_HEADER << FIELD_SEPARATOR << FORMAT_VERSION << '\n';
    writeShop(file, shop);
    writeServices(file, shop);
    writeClients(file, shop);
    writeNotes(file, shop);
    writeMasters(file, shop);
    writeOffers(file, shop);
    writeAdministrators(file, shop);
    writeAppointments(file, shop);
    writePayments(file, shop);

    file.close();
    if (!file) {
        error_code ignored;
        filesystem::remove(temporaryName, ignored);
        throw FileIOException(temporaryName, "запись");
    }

    error_code renameError;
    filesystem::rename(temporaryName, filename, renameError);
    if (renameError) {
        error_code ignored;
        filesystem::remove(temporaryName, ignored);
        throw FileIOException(filename, "замена");
    }
}

Barbershop DataStorage::load() const {
    LoadContext ctx(filename);
    if (!ctx.file) throw FileOpenException(filename, "чтение");

    try {
        readHeader(ctx);
        readShop(ctx);
        readServices(ctx);
        readClients(ctx);
        readNotes(ctx);
        readMasters(ctx);
        readOffers(ctx);
        readAdministrators(ctx);
        readAppointments(ctx);
        readPayments(ctx);
        ensureNoTrailingData(ctx);
    }
    catch (const FileFormatException&) {
        throw;
    }
    catch (const FileIOException&) {
        throw;
    }
    catch (const BarbershopException& error) {
        throw FileFormatException(ctx.lineNumber, error.what());
    }

    Barbershop result = std::move(*ctx.shop);
    return result;
}