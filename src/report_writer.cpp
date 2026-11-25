#include "report_writer.h"
#include <fstream>
#include "predicates.h"
#include "statistics.h"
#include "utils.h"
#include "exceptions/exceptions.h"

namespace {
    std::string levelTitle(const std::string& level) {
        if (level == "New") return "Новый";
        if (level == "Regular") return "Постоянный";
        if (level == "Bronze") return "Бронза";
        if (level == "Silver") return "Серебро";
        if (level == "Gold") return "Золото";
        if (level == "Banned") return "Заблокирован";
        if (level == "Top") return "Топ-мастер";
        if (level == "Standard") return "Мастер";
        if (level == "Junior") return "Младший мастер";
        return level;
    }

    void writeSummary(std::ostream& os, const Barbershop& shop) {
        os << "=== ОБЩИЕ СВЕДЕНИЯ ===\n";
        os << shop.getName() << " (" << shop.getAddress() << ")\n";
        os << "Мастеров: " << shop.getMasters().getCount() << "\n";
        os << "Администраторов: " << shop.getAdministrators().getCount() << "\n";
        os << "Услуг: " << shop.getServices().getCount() << "\n";
        os << "Клиентов: " << shop.getClients().getCount() << "\n";
        os << "Записей: " << shop.getAppointments().getCount() << "\n\n";
    }

    void writeFinance(std::ostream& os, const ShopStatistics& stats) {
        os << "=== ФИНАНСЫ ===\n";
        os << "Выручка: " << formatMoney(stats.revenue) << " | Платежей: " << stats.paymentsCount
            << " | Средний чек: " << formatMoney(stats.averageCheck) << "\n";
        if (stats.paymentsCount > 0) {
            os << "Наименьший платёж: " << formatMoney(stats.smallestPayment)
                << " | Наибольший платёж: " << formatMoney(stats.largestPayment) << "\n";
            os << "Лучший мастер по выручке: " << stats.topMaster << " (" << formatMoney(stats.topMasterRevenue) << ")\n";
        }
        if (!stats.topService.empty()) {
            os << "Самая популярная услуга: " << stats.topService << " (" << stats.topServiceVisits << " визитов)\n";
        }
        os << "\n";
    }

    void writeStatuses(std::ostream& os, const ShopStatistics& stats) {
        os << "=== ЗАПИСИ ПО СТАТУСАМ ===\n";
        os << "Ожидают: " << stats.pending << " | Подтверждены: " << stats.confirmed << " | Пришли: " << stats.arrived
            << " | Неявки: " << stats.noShow << " | Отменены: " << stats.cancelled << "\n\n";
    }

    void writeClients(std::ostream& os, const ShopStatistics& stats) {
        os << "=== КЛИЕНТЫ ПО УРОВНЯМ ===\n";
        for (const auto& entry : stats.clientsByLevel) os << levelTitle(entry.first) << ": " << entry.second << "\n";
        os << "Обслужено клиентов: " << stats.servedClients << " | Новых: " << stats.newClients
            << " | Заблокировано: " << stats.bannedClients << "\n\n";
    }

    void writeMasters(std::ostream& os, const Barbershop& shop) {
        os << "=== МАСТЕРА ===\n";
        for (const auto& master : shop.getMasters()) {
            MasterStatistics stats = calculateMasterStatistics(shop, *master);
            os << master->getName() << " [" << levelTitle(master->classify()) << ", рейтинг " << master->getRating() << "] выручка "
                << formatMoney(stats.revenue) << ", визитов " << stats.arrived << ", уникальных клиентов "
                << stats.uniqueClients << ", предстоит " << stats.upcoming << "\n";
        }
        os << "\n";
    }

    void writeAppointmentList(std::ostream& os, const std::string& title, Collection<Appointment> list) {
        os << "=== " << title << " ===\n";
        list.sort();
        list.print(os);
        os << "\n";
    }
}

ReportWriter::ReportWriter(const std::string& filenameValue) : filename(filenameValue) {}

void ReportWriter::writeReport(const Barbershop& shop) const {
    std::ofstream file(filename);
    if (!file) throw FileOpenException(filename, "запись");

    ShopStatistics stats = calculateStatistics(shop);
    file << "ОТЧЁТ БАРБЕРШОПА\n";
    file << "Сформирован: " << DateTime::now() << "\n\n";
    writeSummary(file, shop);
    writeFinance(file, stats);
    writeStatuses(file, stats);
    writeClients(file, stats);
    writeMasters(file, shop);
    writeAppointmentList(file, "ПРЕДСТОЯЩИЕ ЗАПИСИ", findAll(shop.getAppointments(), isActiveAppointment));
    writeAppointmentList(file, "ВИЗИТЫ, ОЖИДАЮЩИЕ ОПЛАТЫ", findAll(shop.getAppointments(), isUnpaidArrived));

    file.close();
    if (!file) throw FileIOException(filename, "запись");
}

void ReportWriter::writeReceipt(const Payment& payment) const {
    std::ofstream file(filename);
    if (!file) throw FileOpenException(filename, "запись");
    file << payment;
    file.close();
    if (!file) throw FileIOException(filename, "запись");
}
