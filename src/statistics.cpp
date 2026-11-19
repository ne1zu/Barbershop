#include "statistics.h"
#include <algorithm>
#include <numeric>
#include <set>
#include "predicates.h"
#include "utils.h"

namespace {
    bool isGreaterCount(const std::pair<const std::string, int>& first,
        const std::pair<const std::string, int>& second) {
        return first.second < second.second;
    }

    bool isGreaterRevenue(const std::pair<const std::string, double>& first,
        const std::pair<const std::string, double>& second) {
        return first.second < second.second;
    }

    void fillPaymentStatistics(const Barbershop& shop, ShopStatistics& stats) {
        const Collection<Payment>& payments = shop.getPayments();
        stats.paymentsCount = payments.getCount();
        stats.revenue = std::accumulate(payments.begin(), payments.end(), 0.0,
            [](double sum, const std::shared_ptr<Payment>& payment) { return sum + payment->getAmount(); });
        if (payments.isEmpty()) return;

        stats.averageCheck = roundMoney(stats.revenue / stats.paymentsCount);
        std::shared_ptr<Payment> smallest = findMin(payments);
        std::shared_ptr<Payment> largest = findMax(payments);
        stats.smallestPayment = smallest->getAmount();
        stats.largestPayment = largest->getAmount();

        std::for_each(payments.begin(), payments.end(), [&stats](const std::shared_ptr<Payment>& payment) {
            stats.revenueByMaster[payment->getAppointment()->getMaster()->getName()] += payment->getAmount();
        });
        std::map<std::string, double>::const_iterator best = std::max_element(stats.revenueByMaster.begin(),
            stats.revenueByMaster.end(), isGreaterRevenue);
        stats.topMaster = best->first;
        stats.topMasterRevenue = best->second;
    }

    void fillAppointmentStatistics(const Barbershop& shop, ShopStatistics& stats) {
        const Collection<Appointment>& appointments = shop.getAppointments();
        stats.pending = countIf(appointments, hasStatus, AppointmentStatus::Pending);
        stats.confirmed = countIf(appointments, hasStatus, AppointmentStatus::Confirmed);
        stats.arrived = countIf(appointments, hasStatus, AppointmentStatus::Arrived);
        stats.noShow = countIf(appointments, hasStatus, AppointmentStatus::NoShow);
        stats.cancelled = countIf(appointments, hasStatus, AppointmentStatus::Cancelled);

        std::set<int> servedIds;
        std::for_each(appointments.begin(), appointments.end(), [&](const std::shared_ptr<Appointment>& item) {
            if (item->getStatus() != AppointmentStatus::Arrived) return;
            servedIds.insert(item->getClient()->getId());
            stats.visitsByService[item->getService()->getName()]++;
        });
        stats.servedClients = static_cast<int>(servedIds.size());
        if (stats.visitsByService.empty()) return;

        std::map<std::string, int>::const_iterator best = std::max_element(stats.visitsByService.begin(),
            stats.visitsByService.end(), isGreaterCount);
        stats.topService = best->first;
        stats.topServiceVisits = best->second;
    }

    void fillClientStatistics(const Barbershop& shop, ShopStatistics& stats) {
        const Collection<Client>& clients = shop.getClients();
        stats.clients = clients.getCount();
        stats.newClients = countIf(clients, isNewClient);
        stats.bannedClients = countIf(clients, isBannedClient);
        std::for_each(clients.begin(), clients.end(), [&stats](const std::shared_ptr<Client>& client) {
            stats.clientsByLevel[client->classify()]++;
        });
    }
}

ShopStatistics calculateStatistics(const Barbershop& shop) {
    ShopStatistics stats = {};
    fillPaymentStatistics(shop, stats);
    fillAppointmentStatistics(shop, stats);
    fillClientStatistics(shop, stats);
    return stats;
}

MasterStatistics calculateMasterStatistics(const Barbershop& shop, const Master& master) {
    MasterStatistics stats = {};
    Collection<Appointment> ofMaster = findAll(shop.getAppointments(), isOfMaster, master);

    for (const auto& payment : shop.getPayments()) {
        if (*payment->getAppointment()->getMaster() == master) {
            stats.revenue += payment->getAmount();
            stats.paymentsCount++;
        }
    }
    if (stats.paymentsCount > 0) stats.averageCheck = roundMoney(stats.revenue / stats.paymentsCount);

    stats.arrived = countIf(ofMaster, hasStatus, AppointmentStatus::Arrived);
    stats.noShow = countIf(ofMaster, hasStatus, AppointmentStatus::NoShow);
    stats.cancelled = countIf(ofMaster, hasStatus, AppointmentStatus::Cancelled);
    stats.upcoming = countIf(ofMaster, isActiveAppointment);

    std::set<int> clientIds;
    std::for_each(ofMaster.begin(), ofMaster.end(), [&clientIds](const std::shared_ptr<Appointment>& item) {
        if (item->getStatus() == AppointmentStatus::Arrived) clientIds.insert(item->getClient()->getId());
    });
    stats.uniqueClients = static_cast<int>(clientIds.size());
    return stats;
}
