#include "client_menu.h"
#include "config.h"
#include "booking_flow.h"
#include "menu_common.h"
#include "predicates.h"
#include "staff_actions.h"
#include "utils.h"

namespace {
    void printClientMenu(const Client& client) {
        std::cout << "\n===== CLIENT: " << client.getName() << " [" << client.classify() << ", discount "
            << client.getDiscountPercent() << "%] =====\n";
        std::cout << "1. Masters and services\n";
        std::cout << "2. Book an appointment (choose the time)\n";
        std::cout << "3. Book an appointment (smart suggestion)\n";
        std::cout << "4. My appointments (reschedule / cancel)\n";
        std::cout << "5. My visit history and loyalty\n";
        std::cout << "0. Log out\n";
    }

    void printHistory(const Barbershop& shop, const Client& client) {
        printClientCard(std::cout, client);
        Collection<Appointment> history = findAll(shop.getAppointments(), isOfClient, client);
        history.sort();
        std::cout << "Appointments:\n";
        history.print(std::cout, "  ");
        std::cout << "Discount levels: " << BRONZE_VISITS << " visits - " << BRONZE_DISCOUNT << "%, "
            << SILVER_VISITS << " visits - " << SILVER_DISCOUNT << "%, "
            << GOLD_VISITS << " visits - " << GOLD_DISCOUNT << "%\n";
    }

    void handleChoice(Barbershop& shop, const std::shared_ptr<Client>& client, int choice) {
        switch (choice) {
        case 1:
            printMastersCatalog(shop);
            break;
        case 2:
            runManualBooking(shop, client, client);
            break;
        case 3:
            runSmartBooking(shop, client, client);
            break;
        case 4:
        {
            Collection<Appointment> mine = findAll(shop.getAppointments(), isOfClient, *client);
            mine.sort();
            runAppointmentActions(shop, client, mine);
            break;
        }
        case 5:
            printHistory(shop, *client);
            break;
        default:
            std::cout << "[ERROR] No such menu item.\n";
            break;
        }
    }
}

void runClientMenu(Barbershop& shop, const std::shared_ptr<Client>& client) {
    while (true) {
        printClientMenu(*client);
        int choice = readMenuChoice();
        if (choice == 0) break;
        if (choice < 0) continue;
        try {
            handleChoice(shop, client, choice);
        }
        catch (...) {
            reportCurrentException();
        }
    }
}
