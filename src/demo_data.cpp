#include "demo_data.h"
#include <memory>
#include <string>
#include <vector>
#include "exceptions/exceptions.h"

namespace {
    DateTime workingDay(const Master& master, const DateTime& from, int step) {
        DateTime day = from;
        while (master.getDayOff() == day.dayOfWeek()) day = day.addDays(step);
        return day;
    }

    void addPaidVisit(Barbershop& shop, const std::shared_ptr<Administrator>& administrator,
        const std::shared_ptr<Client>& client, const std::shared_ptr<Master>& master,
        const std::shared_ptr<Service>& service, int daysAgo) {
        DateTime day = workingDay(*master, DateTime::now().addDays(-daysAgo), -1);
        std::shared_ptr<Appointment> appointment =
            std::make_shared<Appointment>(client, master, service, day.withMinuteOfDay(11 * 60));
        shop += appointment;
        shop.changeStatus(appointment, AppointmentStatus::Arrived, administrator);
        shop.pay(appointment, PaymentMethod::Cash, administrator);
    }

    std::shared_ptr<Appointment> addFutureAppointment(Barbershop& shop, const std::shared_ptr<Client>& client,
        const std::shared_ptr<Master>& master, const std::shared_ptr<Service>& service, int daysAhead, int hour) {
        DateTime day = workingDay(*master, DateTime::now().addDays(daysAhead), 1);
        std::shared_ptr<Appointment> appointment =
            std::make_shared<Appointment>(client, master, service, day.withMinuteOfDay(hour * 60));
        shop += appointment;
        return appointment;
    }
    class Rng {
    private:
        unsigned state;

    public:
        explicit Rng(unsigned seed) : state(seed) {}

        int next(int limit) {
            state = state * 1103515245u + 12345u;
            return static_cast<int>((state >> 16) % static_cast<unsigned>(limit));
        }
    };

    std::shared_ptr<Appointment> tryAdd(Barbershop& shop, const std::shared_ptr<Client>& client,
        const std::shared_ptr<Master>& master, const std::shared_ptr<Service>& service, const DateTime& start) {
        try {
            std::shared_ptr<Appointment> appointment = std::make_shared<Appointment>(client, master, service, start);
            shop += appointment;
            return appointment;
        }
        catch (const BarbershopException&) {
            return nullptr;
        }
    }

    void finishVisit(Barbershop& shop, const std::shared_ptr<Administrator>& owner,
        const std::shared_ptr<Appointment>& appointment, Rng& rng) {
        int roll = rng.next(100);
        if (roll < 7) {
            shop.changeStatus(appointment, AppointmentStatus::NoShow, owner);
        }
        else if (roll < 13) {
            shop.changeStatus(appointment, AppointmentStatus::Cancelled, owner);
        }
        else {
            shop.changeStatus(appointment, AppointmentStatus::Arrived, owner);
            shop.pay(appointment, rng.next(100) < 60 ? PaymentMethod::Card : PaymentMethod::Cash, owner);
        }
    }

    std::vector<std::shared_ptr<Service>> offeredBy(const Master& master, const std::vector<std::shared_ptr<Service>>& all) {
        std::vector<std::shared_ptr<Service>> result;
        for (const auto& service : all) {
            if (master.canPerform(*service)) result.push_back(service);
        }
        return result;
    }

    std::shared_ptr<Client> pickClient(const std::vector<std::shared_ptr<Client>>& clients, Rng& rng) {
        for (int attempt = 0; attempt < 20; ++attempt) {
            std::shared_ptr<Client> client = clients[rng.next(static_cast<int>(clients.size()))];
            if (!client->isBanned()) return client;
        }
        return clients[0];
    }

    void fillDay(Barbershop& shop, const std::shared_ptr<Administrator>& owner, const std::shared_ptr<Master>& master,
        const std::vector<std::shared_ptr<Service>>& services, const std::vector<std::shared_ptr<Client>>& clients,
        int dayOffset, Rng& rng) {
        DateTime day = DateTime::now().addDays(dayOffset);
        if (master->getDayOff() == day.dayOfWeek()) return;
        std::vector<std::shared_ptr<Service>> offers = offeredBy(*master, services);
        if (offers.empty()) return;
        DateTime now = DateTime::now();
        int minute = master->getWorkStart() + 15 * rng.next(4);
        int target = 4 + rng.next(4);
        for (int i = 0; i < target; ++i) {
            std::shared_ptr<Service> service = offers[rng.next(static_cast<int>(offers.size()))];
            int length = service->getLength();
            if (minute + length > master->getWorkEnd()) break;
            DateTime start = day.withMinuteOfDay(minute);
            std::shared_ptr<Appointment> appointment = tryAdd(shop, pickClient(clients, rng), master, service, start);
            if (appointment) {
                long long endMinutes = start.toMinutes() + appointment->getDuration();
                length = appointment->getDuration();
                if (endMinutes <= now.toMinutes()) {
                    finishVisit(shop, owner, appointment, rng);
                }
                else if (start.toMinutes() > now.toMinutes() && dayOffset >= 0) {
                    if (rng.next(100) < 55) shop.changeStatus(appointment, AppointmentStatus::Confirmed, owner);
                }
                else {
                    shop.changeStatus(appointment, AppointmentStatus::Confirmed, owner);
                }
            }
            minute += length + 15 * rng.next(3);
        }
    }

    void addWorkload(Barbershop& shop, const std::shared_ptr<Administrator>& owner,
        const std::vector<std::shared_ptr<Service>>& services, const std::vector<std::shared_ptr<Client>>& clients) {
        Rng rng(20261119u);
        for (int dayOffset = -30; dayOffset <= 7; ++dayOffset) {
            for (const auto& master : shop.getMasters()) {
                fillDay(shop, owner, master, services, clients, dayOffset, rng);
            }
        }
    }
}

void fillDemoData(Barbershop& shop) {
    std::shared_ptr<Administrator> owner =
        std::make_shared<Administrator>("Ольга Иванова", "+375291230000", "admin", "admin123", "Владелец");
    shop += owner;

    std::shared_ptr<Service> haircut = std::make_shared<Service>("Стрижка", 25.0, 45);
    std::shared_ptr<Service> beard = std::make_shared<Service>("Стрижка бороды", 15.0, 30);
    std::shared_ptr<Service> combo = std::make_shared<Service>("Стрижка и борода", 35.0, 75);
    std::shared_ptr<Service> kids = std::make_shared<Service>("Детская стрижка", 18.0, 30);
    std::shared_ptr<Service> shave = std::make_shared<Service>("Бритьё горячим полотенцем", 28.0, 45);
    std::shared_ptr<Service> coloring = std::make_shared<Service>("Окрашивание волос", 45.0, 90);
    shop += haircut;
    shop += beard;
    shop += combo;
    shop += kids;
    shop += shave;
    shop += coloring;
    std::shared_ptr<Service> headShave = std::make_shared<Service>("Бритьё головы", 22.0, 30);
    std::shared_ptr<Service> styling = std::make_shared<Service>("Укладка", 20.0, 30);
    std::shared_ptr<Service> faceCare = std::make_shared<Service>("Уход за бородой и лицом", 30.0, 45);
    shop += headShave;
    shop += styling;
    shop += faceCare;
    std::vector<std::shared_ptr<Service>> allServices = { haircut, beard, combo, kids, shave, coloring, headShave,
        styling, faceCare };

    std::shared_ptr<Master> alex = std::make_shared<Master>("Алексей Петров", "+375291230001", "alex", "alex123", 10, 4.9);
    std::shared_ptr<Master> ivan = std::make_shared<Master>("Иван Сидоров", "+375291230002", "ivan", "ivan123", 5, 4.6);
    std::shared_ptr<Master> max = std::make_shared<Master>("Максим Орлов", "+375291230003", "max", "max12345", 1, 4.2);
    ivan->setWorkHours(9 * 60, 18 * 60);
    ivan->setDayOff(0);
    max->setWorkHours(10 * 60, 19 * 60);
    max->setDayOff(2);

    alex->setOffer(haircut, 0, 0);
    alex->setOffer(beard, 0, 0);
    alex->setOffer(combo, 45.0, 60);
    alex->setOffer(shave, 0, 0);
    alex->setOffer(coloring, 0, 0);
    ivan->setOffer(haircut, 0, 0);
    ivan->setOffer(beard, 0, 0);
    ivan->setOffer(combo, 0, 0);
    ivan->setOffer(kids, 0, 0);
    max->setOffer(haircut, 0, 0);
    max->setOffer(beard, 0, 0);
    max->setOffer(kids, 0, 0);
    shop += alex;
    shop += ivan;
    shop += max;

    struct MasterSpec { const char* name; const char* phone; const char* login; const char* password; int exp; double rating; int start; int end; int off; };
    const MasterSpec specs[] = {
        { "Артём Волков", "+375291230004", "artem", "artem123", 8, 4.8, 9 * 60, 18 * 60, 6 },
        { "Денис Кравец", "+375291230005", "denis", "denis123", 6, 4.7, 10 * 60, 20 * 60, 0 },
        { "Никита Белов", "+375291230006", "nikita", "nikita123", 3, 4.4, 11 * 60, 20 * 60, 1 },
        { "Роман Миронов", "+375291230007", "roman", "roman123", 12, 4.9, 9 * 60, 19 * 60, 2 },
        { "Кирилл Зайцев", "+375291230008", "kirill", "kirill123", 2, 4.3, 10 * 60, 19 * 60, 3 }
    };
    for (int i = 0; i < 5; ++i) {
        std::shared_ptr<Master> extra = std::make_shared<Master>(specs[i].name, specs[i].phone, specs[i].login,
            specs[i].password, specs[i].exp, specs[i].rating);
        extra->setWorkHours(specs[i].start, specs[i].end);
        extra->setDayOff(specs[i].off);
        extra->setOffer(haircut, 0, 0);
        extra->setOffer(beard, 0, 0);
        extra->setOffer(combo, 0, 0);
        extra->setOffer(i % 2 == 0 ? headShave : styling, 0, 0);
        if (i != 2 && i != 4) extra->setOffer(faceCare, 0, 0);
        if (i == 0 || i == 3) extra->setOffer(shave, 0, 0);
        if (i == 3) extra->setOffer(coloring, 0, 0);
        if (i >= 1) extra->setOffer(kids, 0, 0);
        shop += extra;
    }
    alex->setOffer(headShave, 0, 0);
    alex->setOffer(faceCare, 0, 0);
    ivan->setOffer(styling, 0, 0);
    max->setOffer(headShave, 0, 0);

    std::shared_ptr<Client> dmitry = std::make_shared<Client>("Дмитрий Кузнецов", "+375291110001");
    std::shared_ptr<Client> sergey = std::make_shared<Client>("Сергей Волков", "+375291110002");
    std::shared_ptr<Client> oleg = std::make_shared<Client>("Олег Смирнов", "+375291110003");
    std::shared_ptr<Client> pavel = std::make_shared<Client>("Павел Лебедев", "+375291110004");
    shop += dmitry;
    shop += sergey;
    shop += oleg;
    shop += pavel;

    std::vector<std::shared_ptr<Client>> allClients = { dmitry, sergey, oleg, pavel };
    const char* names[] = { "Андрей Ковалёв", "Михаил Попов", "Владимир Соколов", "Алексей Морозов", "Егор Новиков",
        "Илья Фёдоров", "Константин Егоров", "Максим Павлов", "Антон Семёнов", "Виталий Голубев", "Юрий Виноградов",
        "Станислав Богданов", "Глеб Воробьёв", "Тимур Сафин", "Артур Киселёв", "Вадим Ильин", "Данила Жуков",
        "Руслан Тарасов", "Евгений Беляев", "Николай Комаров", "Игорь Орлов", "Леонид Кудрявцев", "Филипп Гусев",
        "Богдан Титов", "Захар Сорокин", "Александр Мельник", "Виктор Лисицын", "Денис Яковлев", "Пётр Анисимов",
        "Александр Рыбаков", "Кирилл Мазур", "Роман Савченко", "Матвей Кравчук", "Семён Шевченко",
        "Артём Бойко", "Тихон Харитонов", "Марк Антонов", "Гордей Никитин", "Лев Чернышёв", "Эдуард Панов" };
    for (int i = 0; i < 40; ++i) {
        std::string phone = "+37529111";
        std::string number = std::to_string(5 + i);
        phone += std::string(4 - number.size(), '0') + number;
        std::shared_ptr<Client> extraClient = std::make_shared<Client>(names[i], phone);
        shop += extraClient;
        allClients.push_back(extraClient);
    }

    addPaidVisit(shop, owner, dmitry, alex, haircut, 35);
    addPaidVisit(shop, owner, dmitry, ivan, haircut, 28);
    addPaidVisit(shop, owner, dmitry, alex, combo, 21);
    addPaidVisit(shop, owner, dmitry, max, beard, 14);
    addPaidVisit(shop, owner, dmitry, ivan, haircut, 7);

    addFutureAppointment(shop, dmitry, ivan, haircut, 1, 12);
    addFutureAppointment(shop, pavel, alex, haircut, 1, 14);
    addFutureAppointment(shop, sergey, max, kids, 3, 16);

    std::shared_ptr<Appointment> earlier = addFutureAppointment(shop, sergey, max, haircut, -1, 12);
    shop.changeStatus(earlier, AppointmentStatus::Confirmed, owner);

    oleg->registerNoShow();
    oleg->registerNoShow();
    oleg->registerNoShow();
    shop.addClientNote(dmitry, "Предпочитает короткую стрижку, приходит по субботам", owner);
    shop.addClientNote(dmitry, "Любит кофе без сахара, не любит разговоры во время стрижки", owner);
    shop.addClientNote(sergey, "Аллергия на некоторые средства для укладки, уточнять перед процедурой", owner);
    shop.addClientNote(allClients[6], "Постоянно просит стрижку машинкой под насадку 3", owner);
    shop.addClientNote(allClients[11], "Приводит сына на детскую стрижку", owner);
    shop.addClientNote(allClients[17], "Часто опаздывает, лучше записывать с запасом", owner);
    shop.addClientNote(oleg, "Три раза не пришёл без звонка", owner);
    shop.banClient(oleg, "Повторные неявки", owner);

    addWorkload(shop, owner, allServices, allClients);
}
