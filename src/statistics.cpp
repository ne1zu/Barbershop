#include "statistics.h"
#include "predicates.h"
#include "utils.h"

ShopStatistics calculateStatistics(const Barbershop& shop) {
    ShopStatistics stats = {};
    Collection<Appointment> appointments = shop.getAppointments();
    Collection<Client> clients = shop.getClients();

    for (const auto& payment : shop.getPayments()) {
        stats.revenue += payment->getAmount();
        stats.paymentsCount++;
    }
    if (stats.paymentsCount > 0) stats.averageCheck = roundMoney(stats.revenue / stats.paymentsCount);

    stats.pending = countIf(appointments, hasStatus, AppointmentStatus::Pending);
    stats.confirmed = countIf(appointments, hasStatus, AppointmentStatus::Confirmed);
    stats.arrived = countIf(appointments, hasStatus, AppointmentStatus::Arrived);
    stats.noShow = countIf(appointments, hasStatus, AppointmentStatus::NoShow);
    stats.cancelled = countIf(appointments, hasStatus, AppointmentStatus::Cancelled);

    stats.clients = clients.getCount();
    stats.newClients = countIf(clients, isNewClient);
    stats.bannedClients = countIf(clients, isBannedClient);
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
    return stats;
}
