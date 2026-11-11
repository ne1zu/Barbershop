#include "master_menu.h"
#include "menu_common.h"
#include "staff_actions.h"

namespace {
    void printMasterMenu(const Master& master) {
        std::cout << "\n===== MASTER: " << master.getName() << " [" << master.classify() << "] =====\n";
        printToday();
        std::cout << "1. Day journal\n";
        std::cout << "2. New appointment\n";
        std::cout << "3. Appointments (reschedule / cancel / visit result)\n";
        std::cout << "4. Clients (cards, private notes, ban)\n";
        std::cout << "5. My services and prices\n";
        std::cout << "6. My work schedule\n";
        std::cout << "7. My statistics\n";
        std::cout << "0. Log out\n";
    }

    void handleChoice(Barbershop& shop, const std::shared_ptr<Master>& master, int choice) {
        switch (choice) {
        case 1:
            runDayJournal(shop);
            break;
        case 2:
            runStaffNewAppointment(shop, master);
            break;
        case 3:
            runStaffAppointments(shop, master);
            break;
        case 4:
            runClientCards(shop, master);
            break;
        case 5:
            runOffersEditor(shop, master);
            break;
        case 6:
            runScheduleEditor(master);
            break;
        case 7:
            printMasterStatistics(shop, *master);
            break;
        default:
            std::cout << "[ERROR] No such menu item.\n";
            break;
        }
    }
}

void runMasterMenu(Barbershop& shop, const std::shared_ptr<Master>& master) {
    while (true) {
        printMasterMenu(*master);
        int choice = readMenuChoice();
        if (choice == 0) break;
        if (choice < 0) continue;
        try {
            handleChoice(shop, master, choice);
        }
        catch (...) {
            reportCurrentException();
        }
    }
}
