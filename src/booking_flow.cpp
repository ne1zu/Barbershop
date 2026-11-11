#include "booking_flow.h"
#include <vector>
#include "config.h"
#include "menu_common.h"
#include "predicates.h"
#include "slot_finder.h"
#include "utils.h"

void runManualBooking(Barbershop& shop, const std::shared_ptr<Person>& actor,
    const std::shared_ptr<Client>& client) {
    std::shared_ptr<Master> master = pickFromCollection("master", shop.getMasters());
    std::shared_ptr<Service> service = pickOfferedService(*master);
    int duration = master->calculateDuration(*service);
    std::cout << "Price: " << formatMoney(master->calculatePrice(*service)) << " | Duration: "
        << duration << " min\n";

    DateTime day = readDate("Date (dd.mm.yyyy): ");
    std::vector<DateTime> times = findFreeStartTimes(shop, *master, day, duration);
    if (times.empty()) {
        throw InvalidOperationException("booking", "no free time on this day, try another day or the smart booking");
    }
    std::cout << "Free start times:\n";
    printTimes(times);

    int number = readInteger("Time number: ");
    if (number < 1 || number > static_cast<int>(times.size())) {
        throw IndexOutOfRangeException(number - 1, static_cast<int>(times.size()));
    }
    printBooked(*shop.book(client, master, service, times[number - 1], actor));
}

void runSmartBooking(Barbershop& shop, const std::shared_ptr<Person>& actor,
    const std::shared_ptr<Client>& client) {
    std::shared_ptr<Service> service = pickFromCollection("service", shop.getServices());
    Collection<Master> candidates = findAll(shop.getMasters(), canPerformService, *service);
    if (candidates.isEmpty()) {
        throw InvalidOperationException("smart booking", "no master offers the service '" + service->getName() + "'");
    }
    std::cout << "Masters who offer this service:\n";
    candidates.print(std::cout, "  ");

    std::shared_ptr<Master> master;
    int masterNumber = readIntValue("Master number (0 = any master): ", 0, candidates.getCount());
    if (masterNumber > 0) master = candidates.at(masterNumber - 1);
    int days = readIntValue("Search period in days (1-" + std::to_string(MAX_SEARCH_DAYS) + "): ", 1, MAX_SEARCH_DAYS);
    int hour = readIntValue("Preferred hour (0-23, -1 = any): ", -1, 23);

    SlotRequest request = { service, master, DateTime::now(), days, hour < 0 ? -1 : hour * 60 };
    std::vector<SlotOption> options = suggestSlots(shop, request);
    if (options.empty()) {
        throw InvalidOperationException("smart booking", "no free time was found in this period");
    }

    int count = static_cast<int>(options.size());
    std::cout << "Best options:\n";
    for (int i = 0; i < count; ++i) {
        std::cout << "  " << (i + 1) << ". " << options[i].start << " | " << options[i].master->getName()
            << " (" << options[i].master->classify() << ") | Price: " << formatMoney(options[i].price)
            << " | Score: " << options[i].score << "\n";
    }
    int chosen = readIntValue("Choose an option (0 = cancel): ", 0, count);
    if (chosen == 0) return;
    printBooked(*shop.book(client, options[chosen - 1].master, service, options[chosen - 1].start, actor));
}
