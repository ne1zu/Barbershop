#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include "client.h"
#include "master.h"
#include "service.h"
#include "appointment.h"
#include "datetime.h"

inline bool isActiveAppointment(const Appointment& appointment) { return appointment.isActive(); }
inline bool isUnpaidArrived(const Appointment& appointment) {
    return appointment.getStatus() == AppointmentStatus::Arrived && !appointment.isPaid();
}
inline bool isBannedClient(const Client& client) { return client.isBanned(); }
inline bool isNewClient(const Client& client) { return client.isNew(); }

inline bool hasStatus(const Appointment& appointment, const AppointmentStatus& status) {
    return appointment.getStatus() == status;
}
inline bool isOfMaster(const Appointment& appointment, const Master& master) {
    return *appointment.getMaster() == master;
}
inline bool isOfClient(const Appointment& appointment, const Client& client) {
    return *appointment.getClient() == client;
}
inline bool isOfService(const Appointment& appointment, const Service& service) {
    return *appointment.getService() == service;
}
inline bool isOnDay(const Appointment& appointment, const DateTime& day) {
    return appointment.getStart().dayNumber() == day.dayNumber();
}
inline bool canPerformService(const Master& master, const Service& service) {
    return master.canPerform(service);
}

inline bool containsIgnoringCase(const std::string& text, const std::string& part) {
    std::string lowerText = text;
    std::string lowerPart = part;
    std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(),
        [](unsigned char symbol) { return static_cast<char>(std::tolower(symbol)); });
    std::transform(lowerPart.begin(), lowerPart.end(), lowerPart.begin(),
        [](unsigned char symbol) { return static_cast<char>(std::tolower(symbol)); });
    return lowerText.find(lowerPart) != std::string::npos;
}

inline bool matchesClientText(const Client& client, const std::string& part) {
    return containsIgnoringCase(client.getName(), part) || containsIgnoringCase(client.getPhone(), part);
}
