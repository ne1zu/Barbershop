#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <memory>
#include <iostream>
#include "person.h"
#include "client.h"
#include "master.h"
#include "administrator.h"
#include "service.h"
#include "appointment.h"
#include "payment.h"
#include "datetime.h"
#include "collection.h"
#include "logger.h"

class Barbershop {
private:
    std::string name;
    std::string address;

    Collection<Client> clients;
    Collection<Master> masters;
    Collection<Administrator> administrators;
    Collection<Service> services;
    Collection<Appointment> appointments;
    Collection<Payment> payments;

    std::shared_ptr<Logger> journal;
    mutable std::deque<std::string> recentEvents;

    std::shared_ptr<Appointment> findMasterConflict(const Master& master, const DateTime& start,
        int duration, const Appointment* ignored) const;
    std::shared_ptr<Appointment> findClientConflict(const Client& client, const DateTime& start,
        int duration, const Appointment* ignored) const;
    void checkAvailability(const Master& master, const Client& client, const DateTime& start,
        int duration, const Appointment* ignored) const;

public:
    Barbershop(std::string_view nameValue, std::string_view addressValue);

    std::string getName() const;
    std::string getAddress() const;
    void setName(std::string_view nameValue);
    void setAddress(std::string_view addressValue);

    const Collection<Client>& getClients() const;
    const Collection<Master>& getMasters() const;
    const Collection<Administrator>& getAdministrators() const;
    const Collection<Service>& getServices() const;
    const Collection<Appointment>& getAppointments() const;
    const Collection<Payment>& getPayments() const;
    std::vector<std::shared_ptr<Person>> getAllPeople() const;

    void setJournal(const std::string& filename);
    void log(const std::string& event, const std::string& details = "") const;
    const std::deque<std::string>& getRecentEvents() const;
    void replaceContent(Barbershop&& other);
    void restoreAppointment(std::shared_ptr<Appointment> appointment);
    void restorePayment(std::shared_ptr<Payment> payment);

    Barbershop& operator+=(std::shared_ptr<Client> client);
    Barbershop& operator+=(std::shared_ptr<Master> master);
    Barbershop& operator+=(std::shared_ptr<Administrator> administrator);
    Barbershop& operator+=(std::shared_ptr<Service> service);
    Barbershop& operator+=(std::shared_ptr<Appointment> appointment);

    Barbershop& operator-=(std::shared_ptr<Client> client);
    Barbershop& operator-=(std::shared_ptr<Master> master);
    Barbershop& operator-=(std::shared_ptr<Administrator> administrator);
    Barbershop& operator-=(std::shared_ptr<Service> service);
    Barbershop& operator-=(std::shared_ptr<Appointment> appointment);

    std::shared_ptr<Client> loginClient(std::string_view phone) const;
    std::shared_ptr<Person> loginStaff(std::string_view login, std::string_view password) const;

    bool isSlotFree(const Master& master, const Client* client, const DateTime& start, int duration) const;

    std::shared_ptr<Appointment> book(std::shared_ptr<Client> client, std::shared_ptr<Master> master,
        std::shared_ptr<Service> service, const DateTime& start, std::shared_ptr<Person> actor);
    void reschedule(std::shared_ptr<Appointment> appointment, const DateTime& newStart,
        std::shared_ptr<Person> actor);
    void changeStatus(std::shared_ptr<Appointment> appointment, AppointmentStatus newStatus,
        std::shared_ptr<Person> actor);
    std::shared_ptr<Payment> pay(std::shared_ptr<Appointment> appointment, PaymentMethod method,
        std::shared_ptr<Person> actor);

    void changeSchedule(std::shared_ptr<Master> master, int startMinute, int endMinute, int dayOff,
        std::shared_ptr<Person> actor);
    void setOffer(std::shared_ptr<Master> master, std::shared_ptr<Service> service, double customPrice,
        int customDuration, std::shared_ptr<Person> actor);
    void removeOffer(std::shared_ptr<Master> master, std::shared_ptr<Service> service,
        std::shared_ptr<Person> actor);

    void banClient(std::shared_ptr<Client> client, std::string_view reason, std::shared_ptr<Person> actor);
    void unbanClient(std::shared_ptr<Client> client, std::shared_ptr<Person> actor);
    void addClientNote(std::shared_ptr<Client> client, std::string_view text, std::shared_ptr<Person> actor);

    friend std::ostream& operator<<(std::ostream& os, const Barbershop& barbershop);
};
