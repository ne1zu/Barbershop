#include "menu_common.h"
#include "predicates.h"
#include "utils.h"

void reportCurrentException() {
    try {
        throw;
    }
    catch (const ClientBannedException& error) {
        std::cout << "[REFUSED] " << error.what() << "\n";
    }
    catch (const TimeAlreadyBookedException& error) {
        std::cout << "[BUSY] " << error.what() << "\nTip: choose another time or use the smart booking.\n";
    }
    catch (const OutsideWorkingHoursException& error) {
        std::cout << "[CLOSED] " << error.what() << "\n";
    }
    catch (const AccessDeniedException& error) {
        std::cout << "[DENIED] " << error.what() << "\n";
    }
    catch (const LinkViolationException& error) {
        std::cout << "[LINKED] " << error.what() << "\n";
    }
    catch (const BarbershopException& error) {
        std::cout << "[ERROR] " << error.what() << "\n";
    }
    catch (const std::exception& error) {
        std::cout << "[UNEXPECTED] " << error.what() << "\n";
    }
}

bool isClientActor(const std::shared_ptr<Person>& actor) {
    return actor && actor->getEntityType() == "Client";
}

bool isAdministratorActor(const std::shared_ptr<Person>& actor) {
    return actor && actor->getEntityType() == "Administrator";
}

std::string dayOffName(int dayOff) {
    const char* const names[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };
    if (dayOff < 0 || dayOff > 6) return "none";
    return names[dayOff];
}

void printToday() {
    DateTime today = DateTime::now();
    std::cout << "Today: " << today.dateString() << " (" << today.dayName() << ")\n";
}

void printDayJournal(const Barbershop& shop, const DateTime& day) {
    std::cout << "=== Journal for " << day.dateString() << " (" << day.dayName() << ") ===\n";
    Collection<Appointment> ofDay = findAll(shop.getAppointments(), isOnDay, day);
    for (const auto& master : shop.getMasters()) {
        std::cout << master->getName() << " (" << master->classify() << ") | ";
        if (master->getDayOff() == day.dayOfWeek()) {
            std::cout << "day off\n";
            continue;
        }
        std::cout << formatMinutes(master->getWorkStart()) << "-" << formatMinutes(master->getWorkEnd()) << "\n";
        Collection<Appointment> list = findAll(ofDay, isOfMaster, *master);
        list.sort();
        if (list.isEmpty()) std::cout << "  (no appointments)\n";
        for (const auto& appointment : list) {
            std::cout << "  " << appointment->getStart().timeString() << "-" << appointment->getEnd().timeString()
                << " | " << appointment->getService()->getName()
                << " | " << appointment->getClient()->getName() << (appointment->isFirstVisit() ? " [NEW]" : "")
                << " | " << statusToString(appointment->getStatus()) << (appointment->isPaid() ? " [PAID]" : "")
                << "\n";
        }
    }
}

void printOffers(const Master& master) {
    Collection<MasterService> offers = master.getOffers();
    if (offers.isEmpty()) {
        std::cout << "  (no services yet)\n";
        return;
    }
    int number = 1;
    for (const auto& offer : offers) {
        std::cout << "  " << number++ << ". " << offer->getService()->getName()
            << " | Price: " << formatMoney(master.calculatePrice(*offer->getService()))
            << " | Duration: " << master.calculateDuration(*offer->getService()) << " min\n";
    }
}

void printMastersCatalog(const Barbershop& shop) {
    for (const auto& master : shop.getMasters()) {
        std::cout << *master << "\n";
        std::cout << "  Works " << formatMinutes(master->getWorkStart()) << "-" << formatMinutes(master->getWorkEnd())
            << ", day off: " << dayOffName(master->getDayOff()) << "\n";
        printOffers(*master);
    }
}

void printTimes(const std::vector<DateTime>& times) {
    int count = static_cast<int>(times.size());
    for (int i = 0; i < count; ++i) {
        std::cout << (i + 1) << ") " << times[i].timeString() << "   ";
        if ((i + 1) % 6 == 0) std::cout << "\n";
    }
    if (count % 6 != 0) std::cout << "\n";
}

void printBooked(const Appointment& appointment) {
    std::cout << "[OK] Booked: " << appointment << "\n";
    int discount = appointment.getClient()->getDiscountPercent();
    if (discount > 0) {
        std::cout << "Loyalty discount " << discount << "% will be applied at payment.\n";
    }
}

std::shared_ptr<Service> pickOfferedService(const Master& master) {
    Collection<MasterService> offers = master.getOffers();
    if (offers.isEmpty()) {
        throw InvalidOperationException("select service", "master '" + master.getName() + "' has no services");
    }
    std::cout << "Services of " << master.getName() << ":\n";
    printOffers(master);
    int number = readInteger("Number (0 = cancel): ");
    if (number == 0) throw InvalidOperationException("select service", "cancelled");
    return offers.at(number - 1)->getService();
}
