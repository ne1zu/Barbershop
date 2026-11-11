#pragma once
#include <string>
#include <string_view>
#include <vector>
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

    Collection<Client> getClients() const;
    Collection<Master> getMasters() const;
    Collection<Administrator> getAdministrators() const;
    Collection<Service> getServices() const;
    Collection<Appointment> getAppointments() const;
    Collection<Payment> getPayments() const;
    std::vector<std::shared_ptr<Person>> getAllPeople() const;

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

    bool isMasterFree(const Master& master, const DateTime& start, int duration,
        const Appointment* ignored = nullptr) const;

    std::shared_ptr<Appointment> book(std::shared_ptr<Client> client, std::shared_ptr<Master> master,
        std::shared_ptr<Service> service, const DateTime& start, std::shared_ptr<Person> actor);
    void reschedule(std::shared_ptr<Appointment> appointment, const DateTime& newStart,
        std::shared_ptr<Person> actor);
    void changeStatus(std::shared_ptr<Appointment> appointment, AppointmentStatus newStatus,
        std::shared_ptr<Person> actor);
    std::shared_ptr<Payment> pay(std::shared_ptr<Appointment> appointment, PaymentMethod method,
        std::shared_ptr<Person> actor);

    void banClient(std::shared_ptr<Client> client, std::string_view reason, std::shared_ptr<Person> actor);
    void unbanClient(std::shared_ptr<Client> client, std::shared_ptr<Person> actor);
    void addClientNote(std::shared_ptr<Client> client, std::string_view text, std::shared_ptr<Person> actor);

    friend std::ostream& operator<<(std::ostream& os, const Barbershop& barbershop);
};
