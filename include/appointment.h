#pragma once
#include <string>
#include <iostream>
#include <memory>
#include "client.h"
#include "master.h"
#include "service.h"
#include "datetime.h"

enum class AppointmentStatus { Pending, Confirmed, Arrived, NoShow, Cancelled };

std::string statusToString(AppointmentStatus status);

class Appointment {
private:
    int id;
    std::shared_ptr<Client> client;
    std::shared_ptr<Master> master;
    std::shared_ptr<Service> service;
    double price;
    int duration;
    DateTime start;
    AppointmentStatus status;
    bool firstVisit;
    bool paid;
    static int counter;

    friend class Barbershop;
    void setStatus(AppointmentStatus statusValue);
    void setStart(const DateTime& startValue);
    void markPaid();

public:
    Appointment(std::shared_ptr<Client> clientValue, std::shared_ptr<Master> masterValue,
        std::shared_ptr<Service> serviceValue, const DateTime& startValue);

    int getId() const;
    std::shared_ptr<Client> getClient() const;
    std::shared_ptr<Master> getMaster() const;
    std::shared_ptr<Service> getService() const;
    double getPrice() const;
    int getDuration() const;
    DateTime getStart() const;
    DateTime getEnd() const;
    AppointmentStatus getStatus() const;
    std::string getTimeRange() const;

    bool isActive() const;
    bool isFirstVisit() const;
    bool isPaid() const;
    bool canTransitionTo(AppointmentStatus newStatus) const;
    bool overlapsInterval(const DateTime& otherStart, int otherDuration) const;

    bool operator==(const Appointment& other) const;
    bool operator<(const Appointment& other) const;
    bool operator>(const Appointment& other) const;

    std::string getEntityType() const;
    void printInfo(std::ostream& os) const;
    std::string classify() const;

    friend std::ostream& operator<<(std::ostream& os, const Appointment& appointment);
};
