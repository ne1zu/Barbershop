#include "demo_data.h"
#include <memory>
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
}

void fillDemoData(Barbershop& shop) {
    std::shared_ptr<Administrator> owner =
        std::make_shared<Administrator>("Olga Ivanova", "+375291230000", "admin", "admin123", "Owner");
    shop += owner;

    std::shared_ptr<Service> haircut = std::make_shared<Service>("Haircut", 25.0, 45);
    std::shared_ptr<Service> beard = std::make_shared<Service>("Beard trim", 15.0, 30);
    std::shared_ptr<Service> combo = std::make_shared<Service>("Haircut and beard", 35.0, 75);
    std::shared_ptr<Service> kids = std::make_shared<Service>("Kids haircut", 18.0, 30);
    std::shared_ptr<Service> shave = std::make_shared<Service>("Hot towel shave", 28.0, 45);
    std::shared_ptr<Service> coloring = std::make_shared<Service>("Hair coloring", 45.0, 90);
    shop += haircut;
    shop += beard;
    shop += combo;
    shop += kids;
    shop += shave;
    shop += coloring;

    std::shared_ptr<Master> alex = std::make_shared<Master>("Alex Petrov", "+375291230001", "alex", "alex123", 10, 4.9);
    std::shared_ptr<Master> ivan = std::make_shared<Master>("Ivan Sidorov", "+375291230002", "ivan", "ivan123", 5, 4.6);
    std::shared_ptr<Master> max = std::make_shared<Master>("Max Orlov", "+375291230003", "max", "max12345", 1, 4.2);
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

    std::shared_ptr<Client> dmitry = std::make_shared<Client>("Dmitry Kuznetsov", "+375291110001");
    std::shared_ptr<Client> sergey = std::make_shared<Client>("Sergey Volkov", "+375291110002");
    std::shared_ptr<Client> oleg = std::make_shared<Client>("Oleg Smirnov", "+375291110003");
    std::shared_ptr<Client> pavel = std::make_shared<Client>("Pavel Lebedev", "+375291110004");
    shop += dmitry;
    shop += sergey;
    shop += oleg;
    shop += pavel;

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
    shop.addClientNote(oleg, "Did not come three times without a call", owner);
    shop.banClient(oleg, "Repeated no-shows", owner);
}
