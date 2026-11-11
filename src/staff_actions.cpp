#include "staff_actions.h"
#include "booking_flow.h"
#include "config.h"
#include "menu_common.h"
#include "predicates.h"
#include "statistics.h"
#include "utils.h"

std::shared_ptr<Client> pickOrCreateClient(Barbershop& shop) {
    Collection<Client> clients = shop.getClients();
    std::cout << "Clients:\n";
    clients.print(std::cout, "  ");
    int number = readInteger("Client number (0 = register a new client): ");
    if (number == 0) {
        std::shared_ptr<Client> created = std::make_shared<Client>(readEntity<Client>());
        shop += created;
        std::cout << "[OK] Client registered: " << *created << "\n";
        return created;
    }
    return clients.at(number - 1);
}

void runAppointmentActions(Barbershop& shop, const std::shared_ptr<Person>& actor,
    const Collection<Appointment>& list) {
    if (list.isEmpty()) {
        std::cout << "No appointments found.\n";
        return;
    }
    std::shared_ptr<Appointment> appointment = pickFromCollection("appointment", list);

    bool staff = !isClientActor(actor);
    bool administrator = isAdministratorActor(actor);
    std::cout << "1. Reschedule\n2. Cancel\n";
    if (staff) std::cout << "3. Confirm\n4. Mark as arrived\n5. Mark as no-show\n";
    if (administrator) std::cout << "6. Delete the record\n";
    std::cout << "0. Back\n";

    switch (readMenuChoice()) {
    case 1:
        shop.reschedule(appointment, readDateTime("New date and time (dd.mm.yyyy hh:mm): "), actor);
        std::cout << "[OK] Rescheduled: " << *appointment << "\n";
        break;
    case 2:
        shop.changeStatus(appointment, AppointmentStatus::Cancelled, actor);
        std::cout << "[OK] Cancelled.\n";
        break;
    case 3:
        shop.changeStatus(appointment, AppointmentStatus::Confirmed, actor);
        std::cout << "[OK] Confirmed.\n";
        break;
    case 4:
        shop.changeStatus(appointment, AppointmentStatus::Arrived, actor);
        std::cout << "[OK] Marked as arrived. The administrator can now take the payment.\n";
        break;
    case 5:
        shop.changeStatus(appointment, AppointmentStatus::NoShow, actor);
        std::cout << "[OK] Marked as no-show.\n";
        break;
    case 6:
        if (!administrator) throw AccessDeniedException("delete appointment records");
        shop -= appointment;
        std::cout << "[OK] Record deleted.\n";
        break;
    default:
        break;
    }
}

void runStaffNewAppointment(Barbershop& shop, const std::shared_ptr<Person>& actor) {
    std::shared_ptr<Client> client = pickOrCreateClient(shop);
    std::cout << "1. Choose the time manually\n2. Smart booking\n0. Back\n";
    int choice = readMenuChoice();
    if (choice == 1) runManualBooking(shop, actor, client);
    else if (choice == 2) runSmartBooking(shop, actor, client);
}

void runStaffAppointments(Barbershop& shop, const std::shared_ptr<Person>& actor) {
    std::cout << "1. Active appointments (pending and confirmed)\n2. Appointments of a date\n0. Back\n";
    int choice = readMenuChoice();
    Collection<Appointment> list;
    if (choice == 1) {
        list = findAll(shop.getAppointments(), isActiveAppointment);
    }
    else if (choice == 2) {
        list = findAll(shop.getAppointments(), isOnDay, readDate("Date (dd.mm.yyyy): "));
    }
    else {
        return;
    }
    list.sort();
    runAppointmentActions(shop, actor, list);
}

void runClientCards(Barbershop& shop, const std::shared_ptr<Person>& actor) {
    std::shared_ptr<Client> client = pickOrCreateClient(shop);
    printClientCard(std::cout, *client);

    Collection<Appointment> history = findAll(shop.getAppointments(), isOfClient, *client);
    history.sort();
    std::cout << "Appointments:\n";
    history.print(std::cout, "  ");

    std::cout << "1. Add a private note\n2. Ban the client\n3. Unban the client (administrator)\n0. Back\n";
    switch (readMenuChoice()) {
    case 1:
        shop.addClientNote(client, readLine("Note: "), actor);
        std::cout << "[OK] Note saved.\n";
        break;
    case 2:
        shop.banClient(client, readLine("Reason: "), actor);
        std::cout << "[OK] Client banned.\n";
        break;
    case 3:
        shop.unbanClient(client, actor);
        std::cout << "[OK] Client unbanned.\n";
        break;
    default:
        break;
    }
}

void runDayJournal(const Barbershop& shop) {
    printToday();
    printDayJournal(shop, readDate("Date (dd.mm.yyyy): "));
}

void runOffersEditor(Barbershop& shop, const std::shared_ptr<Master>& master) {
    std::cout << "Services of " << master->getName() << " (level " << master->classify() << ", price factor "
        << master->getPriceFactor() << "):\n";
    printOffers(*master);
    std::cout << "1. Add or change an offer\n2. Remove an offer\n0. Back\n";
    int choice = readMenuChoice();
    if (choice == 1) {
        std::shared_ptr<Service> service = pickFromCollection("service", shop.getServices());
        double price = readDouble("Price (0 = automatic: base price x level factor): ");
        int duration = readIntValue("Duration in minutes (0 = standard): ", 0, MAX_SERVICE_MINUTES);
        master->setOffer(service, price, duration);
        std::cout << "[OK] Offer saved.\n";
    }
    else if (choice == 2) {
        std::shared_ptr<Service> service = pickOfferedService(*master);
        master->removeOffer(service->getId());
        std::cout << "[OK] Offer removed.\n";
    }
}

void runScheduleEditor(const std::shared_ptr<Master>& master) {
    std::cout << "Current schedule of " << master->getName() << ": " << formatMinutes(master->getWorkStart())
        << "-" << formatMinutes(master->getWorkEnd()) << ", day off: " << dayOffName(master->getDayOff()) << "\n";
    int start = readClock("Work start (hh:mm): ");
    int end = readClock("Work end (hh:mm): ");
    int dayOff = readIntValue("Day off (-1 = none, 0 = Monday ... 6 = Sunday): ", -1, 6);
    master->setWorkHours(start, end);
    master->setDayOff(dayOff);
    std::cout << "[OK] Schedule updated.\n";
}

void printMasterStatistics(const Barbershop& shop, const Master& master) {
    MasterStatistics stats = calculateMasterStatistics(shop, master);
    std::cout << "--- " << master.getName() << " (" << master.classify() << ") ---\n";
    std::cout << "Revenue: " << formatMoney(stats.revenue) << " | Payments: " << stats.paymentsCount
        << " | Average check: " << formatMoney(stats.averageCheck) << "\n";
    std::cout << "Visits: " << stats.arrived << " | No-shows: " << stats.noShow
        << " | Cancelled: " << stats.cancelled << " | Upcoming: " << stats.upcoming << "\n";
}
