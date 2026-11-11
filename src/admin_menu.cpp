#include "admin_menu.h"
#include "menu_common.h"
#include "predicates.h"
#include "staff_actions.h"
#include "statistics.h"
#include "utils.h"

namespace {
    void printAdminMenu(const Administrator& administrator) {
        std::cout << "\n===== ADMINISTRATOR: " << administrator.getName() << " [" << administrator.classify()
            << "] =====\n";
        printToday();
        std::cout << "1. Day journal\n";
        std::cout << "2. New appointment\n";
        std::cout << "3. Appointments (reschedule / cancel / visit result)\n";
        std::cout << "4. Clients (cards, private notes, ban / unban)\n";
        std::cout << "5. Staff\n";
        std::cout << "6. Services catalog\n";
        std::cout << "7. Master services and schedule\n";
        std::cout << "8. Payments\n";
        std::cout << "9. Analytics\n";
        std::cout << "0. Log out\n";
    }

    void runStaffManagement(Barbershop& shop) {
        std::cout << "1. List all people\n2. Add a master\n3. Add an administrator\n";
        std::cout << "4. Remove a master\n5. Remove an administrator\n0. Back\n";
        switch (readMenuChoice()) {
        case 1:
            for (const auto& person : shop.getAllPeople()) {
                std::cout << *person << " [" << person->classify() << "] price factor: "
                    << person->getPriceFactor() << "\n";
            }
            break;
        case 2:
            shop += std::make_shared<Master>(readEntity<Master>());
            std::cout << "[OK] Master added. Add services to him in item 7.\n";
            break;
        case 3:
            shop += std::make_shared<Administrator>(readEntity<Administrator>());
            std::cout << "[OK] Administrator added.\n";
            break;
        case 4:
            shop -= pickFromCollection("master", shop.getMasters());
            std::cout << "[OK] Master removed.\n";
            break;
        case 5:
            shop -= pickFromCollection("administrator", shop.getAdministrators());
            std::cout << "[OK] Administrator removed.\n";
            break;
        default:
            break;
        }
    }

    void runServicesCatalog(Barbershop& shop) {
        std::cout << "Services:\n";
        shop.getServices().print(std::cout, "  ");
        std::cout << "1. Add a service\n2. Remove a service\n0. Back\n";
        int choice = readMenuChoice();
        if (choice == 1) {
            shop += std::make_shared<Service>(readEntity<Service>());
            std::cout << "[OK] Service added. Masters can offer it in item 7.\n";
        }
        else if (choice == 2) {
            shop -= pickFromCollection("service", shop.getServices());
            std::cout << "[OK] Service removed.\n";
        }
    }

    void runMasterSettings(Barbershop& shop) {
        std::shared_ptr<Master> master = pickFromCollection("master", shop.getMasters());
        std::cout << "1. Services and prices\n2. Work schedule\n0. Back\n";
        int choice = readMenuChoice();
        if (choice == 1) runOffersEditor(shop, master);
        else if (choice == 2) runScheduleEditor(master);
    }

    void runPayments(Barbershop& shop, const std::shared_ptr<Administrator>& administrator) {
        Collection<Appointment> unpaid = findAll(shop.getAppointments(), isUnpaidArrived);
        if (unpaid.isEmpty()) {
            std::cout << "No visits are waiting for payment. Mark a visit as arrived first (item 3).\n";
            return;
        }
        unpaid.sort();
        std::shared_ptr<Appointment> appointment = pickFromCollection("visit to pay", unpaid);
        std::cout << "Price: " << formatMoney(appointment->getPrice()) << " | Client discount: "
            << appointment->getClient()->getDiscountPercent() << "%\n";
        int method = readIntValue("Payment method (1 = cash, 2 = card): ", 1, 2);
        std::shared_ptr<Payment> payment = shop.pay(appointment,
            method == 1 ? PaymentMethod::Cash : PaymentMethod::Card, administrator);
        std::cout << *payment;
    }

    void runAnalytics(const Barbershop& shop) {
        ShopStatistics stats = calculateStatistics(shop);
        std::cout << "=== Analytics: " << shop.getName() << " ===\n";
        std::cout << "Revenue: " << formatMoney(stats.revenue) << " | Payments: " << stats.paymentsCount
            << " | Average check: " << formatMoney(stats.averageCheck) << "\n";
        std::cout << "Appointments: pending " << stats.pending << ", confirmed " << stats.confirmed
            << ", arrived " << stats.arrived << ", no-show " << stats.noShow
            << ", cancelled " << stats.cancelled << "\n";
        std::cout << "Clients: " << stats.clients << " | new: " << stats.newClients
            << " | banned: " << stats.bannedClients << "\n";

        std::shared_ptr<Master> best = findMax(shop.getMasters());
        if (best) std::cout << "Highest rated master: " << best->getName() << " (" << best->getRating() << ")\n";
        std::shared_ptr<Client> loyal = findMax(shop.getClients());
        if (loyal) std::cout << "Most frequent client: " << loyal->getName() << " (" << loyal->getVisitsCount() << " visits)\n";

        for (const auto& master : shop.getMasters()) printMasterStatistics(shop, *master);
    }

    void handleChoice(Barbershop& shop, const std::shared_ptr<Administrator>& administrator, int choice) {
        switch (choice) {
        case 1:
            runDayJournal(shop);
            break;
        case 2:
            runStaffNewAppointment(shop, administrator);
            break;
        case 3:
            runStaffAppointments(shop, administrator);
            break;
        case 4:
            runClientCards(shop, administrator);
            break;
        case 5:
            runStaffManagement(shop);
            break;
        case 6:
            runServicesCatalog(shop);
            break;
        case 7:
            runMasterSettings(shop);
            break;
        case 8:
            runPayments(shop, administrator);
            break;
        case 9:
            runAnalytics(shop);
            break;
        default:
            std::cout << "[ERROR] No such menu item.\n";
            break;
        }
    }
}

void runAdminMenu(Barbershop& shop, const std::shared_ptr<Administrator>& administrator) {
    while (true) {
        printAdminMenu(*administrator);
        int choice = readMenuChoice();
        if (choice == 0) break;
        if (choice < 0) continue;
        try {
            handleChoice(shop, administrator, choice);
        }
        catch (...) {
            reportCurrentException();
        }
    }
}
