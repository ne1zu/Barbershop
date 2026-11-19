#include "report_writer.h"
#include <fstream>
#include "predicates.h"
#include "statistics.h"
#include "utils.h"
#include "exceptions/exceptions.h"

namespace {
    void writeSummary(std::ostream& os, const Barbershop& shop) {
        os << "=== SUMMARY ===\n";
        os << shop.getName() << " (" << shop.getAddress() << ")\n";
        os << "Masters: " << shop.getMasters().getCount() << "\n";
        os << "Administrators: " << shop.getAdministrators().getCount() << "\n";
        os << "Services: " << shop.getServices().getCount() << "\n";
        os << "Clients: " << shop.getClients().getCount() << "\n";
        os << "Appointments: " << shop.getAppointments().getCount() << "\n\n";
    }

    void writeFinance(std::ostream& os, const ShopStatistics& stats) {
        os << "=== FINANCE ===\n";
        os << "Revenue: " << formatMoney(stats.revenue) << " | Payments: " << stats.paymentsCount
            << " | Average check: " << formatMoney(stats.averageCheck) << "\n";
        if (stats.paymentsCount > 0) {
            os << "Smallest payment: " << formatMoney(stats.smallestPayment)
                << " | Largest payment: " << formatMoney(stats.largestPayment) << "\n";
            os << "Best master by revenue: " << stats.topMaster << " (" << formatMoney(stats.topMasterRevenue) << ")\n";
        }
        if (!stats.topService.empty()) {
            os << "Most popular service: " << stats.topService << " (" << stats.topServiceVisits << " visits)\n";
        }
        os << "\n";
    }

    void writeStatuses(std::ostream& os, const ShopStatistics& stats) {
        os << "=== APPOINTMENTS BY STATUS ===\n";
        os << "Pending: " << stats.pending << " | Confirmed: " << stats.confirmed << " | Arrived: " << stats.arrived
            << " | No-show: " << stats.noShow << " | Cancelled: " << stats.cancelled << "\n\n";
    }

    void writeClients(std::ostream& os, const ShopStatistics& stats) {
        os << "=== CLIENTS BY LEVEL ===\n";
        for (const auto& entry : stats.clientsByLevel) os << entry.first << ": " << entry.second << "\n";
        os << "Served clients: " << stats.servedClients << " | New: " << stats.newClients
            << " | Banned: " << stats.bannedClients << "\n\n";
    }

    void writeMasters(std::ostream& os, const Barbershop& shop) {
        os << "=== MASTERS ===\n";
        for (const auto& master : shop.getMasters()) {
            MasterStatistics stats = calculateMasterStatistics(shop, *master);
            os << master->getName() << " [" << master->classify() << ", rating " << master->getRating() << "] revenue "
                << formatMoney(stats.revenue) << ", visits " << stats.arrived << ", unique clients "
                << stats.uniqueClients << ", upcoming " << stats.upcoming << "\n";
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
    if (!file) throw FileOpenException(filename, "writing");

    ShopStatistics stats = calculateStatistics(shop);
    file << "BARBERSHOP REPORT\n";
    file << "Generated: " << DateTime::now() << "\n\n";
    writeSummary(file, shop);
    writeFinance(file, stats);
    writeStatuses(file, stats);
    writeClients(file, stats);
    writeMasters(file, shop);
    writeAppointmentList(file, "UPCOMING APPOINTMENTS", findAll(shop.getAppointments(), isActiveAppointment));
    writeAppointmentList(file, "VISITS WAITING FOR PAYMENT", findAll(shop.getAppointments(), isUnpaidArrived));

    file.close();
    if (!file) throw FileIOException(filename, "writing");
}

void ReportWriter::writeReceipt(const Payment& payment) const {
    std::ofstream file(filename);
    if (!file) throw FileOpenException(filename, "writing");
    file << payment;
    file.close();
    if (!file) throw FileIOException(filename, "writing");
}
