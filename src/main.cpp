#include <iostream>
#include <memory>
#include "barbershop.h"
#include "demo_data.h"
#include "client_menu.h"
#include "master_menu.h"
#include "admin_menu.h"
#include "console_input.h"
#include "menu_common.h"

using namespace std;

namespace {
    void printStartMenu(const Barbershop& shop) {
        cout << "\n===== " << shop.getName() << " =====\n";
        cout << "1. Log in as a client (by phone)\n";
        cout << "2. Register as a client\n";
        cout << "3. Log in as staff (login and password)\n";
        cout << "0. Exit\n";
    }

    void loginAsClient(Barbershop& shop) {
        shared_ptr<Client> client = shop.loginClient(readLine("Phone (+375XXXXXXXXX): "));
        runClientMenu(shop, client);
    }

    void registerAsClient(Barbershop& shop) {
        shared_ptr<Client> client = make_shared<Client>(readEntity<Client>());
        shop += client;
        cout << "[OK] You are registered.\n";
        runClientMenu(shop, client);
    }

    void loginAsStaff(Barbershop& shop) {
        string login = readLine("Login: ");
        string password = readLine("Password: ");
        shared_ptr<Person> person = shop.loginStaff(login, password);

        shared_ptr<Master> master = dynamic_pointer_cast<Master>(person);
        if (master) {
            runMasterMenu(shop, master);
            return;
        }
        runAdminMenu(shop, dynamic_pointer_cast<Administrator>(person));
    }
}

int main() {
    Barbershop shop("Sharp Blade Barbershop", "Minsk, Independence Ave 10");
    try {
        fillDemoData(shop);
    }
    catch (...) {
        reportCurrentException();
        return 1;
    }
    cout << shop << "\n";

    while (true) {
        printStartMenu(shop);
        int choice = readMenuChoice();
        if (choice == 0) break;
        if (choice < 0) continue;
        try {
            switch (choice) {
            case 1:
                loginAsClient(shop);
                break;
            case 2:
                registerAsClient(shop);
                break;
            case 3:
                loginAsStaff(shop);
                break;
            default:
                cout << "[ERROR] No such menu item.\n";
                break;
            }
        }
        catch (...) {
            reportCurrentException();
        }
    }
    cout << "Goodbye!\n";
    return 0;
}
