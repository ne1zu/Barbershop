#include "barbershop.h"
#include "config.h"
#include "utils.h"
#include "predicates.h"
#include "exceptions/exceptions.h"
#include <algorithm>

namespace {
    bool isStaff(const std::shared_ptr<Person>& actor) {
        return dynamic_cast<const Master*>(actor.get()) != nullptr
            || dynamic_cast<const Administrator*>(actor.get()) != nullptr;
    }

    bool isAdministrator(const std::shared_ptr<Person>& actor) {
        return dynamic_cast<const Administrator*>(actor.get()) != nullptr;
    }

    bool isOwnerOf(const std::shared_ptr<Person>& actor, const Appointment& appointment) {
        const Person* owner = appointment.getClient().get();
        return actor.get() == owner;
    }

    void requireActor(const std::shared_ptr<Person>& actor) {
        if (!actor) throw NullObjectException("Barbershop", "actor");
    }

    void requireStaff(const std::shared_ptr<Person>& actor, const std::string& action) {
        requireActor(actor);
        if (!isStaff(actor)) throw AccessDeniedException(action);
    }

    void requireCanManage(const std::shared_ptr<Person>& actor, const std::shared_ptr<Master>& master,
        const std::string& action) {
        requireStaff(actor, action);
        if (!master) throw NullObjectException("Barbershop", "master");
        if (!isAdministrator(actor) && actor.get() != master.get()) {
            throw AccessDeniedException(action + " of another master");
        }
    }

    void checkClientTimeLimit(const std::shared_ptr<Person>& actor, const Appointment& appointment,
        const std::string& operation) {
        if (isStaff(actor)) return;
        long long minutesLeft = appointment.getStart().toMinutes() - DateTime::now().toMinutes();
        if (minutesLeft < CANCEL_LIMIT_HOURS * 60) {
            throw InvalidOperationException(operation, "clients can do this at least "
                + std::to_string(CANCEL_LIMIT_HOURS) + " hours before the start, please call the barbershop");
        }
    }
}

Barbershop::Barbershop(std::string_view nameValue, std::string_view addressValue) {
    setName(nameValue);
    setAddress(addressValue);
}

std::string Barbershop::getName() const { return name; }
std::string Barbershop::getAddress() const { return address; }

void Barbershop::setName(std::string_view nameValue) {
    if (isBlank(nameValue)) throw InvalidDataException("barbershop name", "must not be empty");
    if (!isStorableText(nameValue)) {
        throw InvalidDataException("barbershop name", "must not contain '|' or a line break");
    }
    name = nameValue;
}

void Barbershop::setAddress(std::string_view addressValue) {
    if (isBlank(addressValue)) throw InvalidDataException("address", "must not be empty");
    if (!isStorableText(addressValue)) throw InvalidDataException("address", "must not contain '|' or a line break");
    address = addressValue;
}

const Collection<Client>& Barbershop::getClients() const { return clients; }
const Collection<Master>& Barbershop::getMasters() const { return masters; }
const Collection<Administrator>& Barbershop::getAdministrators() const { return administrators; }
const Collection<Service>& Barbershop::getServices() const { return services; }
const Collection<Appointment>& Barbershop::getAppointments() const { return appointments; }
const Collection<Payment>& Barbershop::getPayments() const { return payments; }

void Barbershop::setJournal(const std::string& filename) {
    journal = std::make_shared<Logger>(filename);
}

void Barbershop::log(const std::string& event, const std::string& details) const {
    if (!journal) return;
    recentEvents.push_back(Logger::makeLine(event, details));
    if (static_cast<int>(recentEvents.size()) > RECENT_EVENTS_LIMIT) recentEvents.pop_front();
    journal->log(event, details);
}

const std::deque<std::string>& Barbershop::getRecentEvents() const { return recentEvents; }

void Barbershop::replaceContent(Barbershop&& other) {
    name = std::move(other.name);
    address = std::move(other.address);
    clients = std::move(other.clients);
    masters = std::move(other.masters);
    administrators = std::move(other.administrators);
    services = std::move(other.services);
    appointments = std::move(other.appointments);
    payments = std::move(other.payments);
}

void Barbershop::restoreAppointment(std::shared_ptr<Appointment> appointment) {
    if (!appointment) throw NullObjectException("Barbershop::restoreAppointment", "appointment");
    if (!clients.contains(*appointment->getClient()) || !masters.contains(*appointment->getMaster())
        || !services.contains(*appointment->getService())) {
        throw LinkViolationException("appointment #" + std::to_string(appointment->getId()),
            "the client, the master and the service must be registered in the barbershop");
    }
    if (appointments.contains(*appointment)) {
        throw DuplicateObjectException("Appointment", std::to_string(appointment->getId()));
    }
    if (appointment->occupiesTime()) {
        std::shared_ptr<Appointment> masterConflict = findMasterConflict(*appointment->getMaster(),
            appointment->getStart(), appointment->getDuration(), nullptr);
        if (masterConflict) {
            throw TimeAlreadyBookedException("Master", appointment->getMaster()->getName(),
                masterConflict->getTimeRange());
        }
        std::shared_ptr<Appointment> clientConflict = findClientConflict(*appointment->getClient(),
            appointment->getStart(), appointment->getDuration(), nullptr);
        if (clientConflict) {
            throw TimeAlreadyBookedException("Client", appointment->getClient()->getName(),
                clientConflict->getTimeRange());
        }
    }
    appointments.add(appointment);
}

void Barbershop::restorePayment(std::shared_ptr<Payment> payment) {
    if (!payment) throw NullObjectException("Barbershop::restorePayment", "payment");
    if (!appointments.contains(*payment->getAppointment())) {
        throw LinkViolationException("payment #" + std::to_string(payment->getId()),
            "the appointment is not registered in the barbershop");
    }
    if (payments.contains(*payment)) {
        throw DuplicateObjectException("Payment", std::to_string(payment->getId()));
    }
    if (!payment->getAppointment()->isPaid()) {
        throw InvalidOperationException("restore payment", "the appointment #"
            + std::to_string(payment->getAppointment()->getId()) + " is not marked as paid");
    }
    for (const auto& other : payments) {
        if (other->getAppointment() == payment->getAppointment()) {
            throw DuplicateObjectException("Payment of appointment", std::to_string(payment->getAppointment()->getId()));
        }
    }
    payments.add(payment);
}

std::vector<std::shared_ptr<Person>> Barbershop::getAllPeople() const {
    std::vector<std::shared_ptr<Person>> result;
    for (const auto& administrator : administrators) result.push_back(administrator);
    for (const auto& master : masters) result.push_back(master);
    for (const auto& client : clients) result.push_back(client);
    return result;
}

Barbershop& Barbershop::operator+=(std::shared_ptr<Client> client) {
    if (!client) throw NullObjectException("Barbershop::operator+=", "client");
    if (isBlank(client->getName())) throw InvalidDataException("client", "the client has no name or phone");
    if (clients.contains(*client)) throw DuplicateObjectException("Client with phone", client->getPhone());
    clients.add(client);
    log("ADD", "Client '" + client->getName() + "'");
    return *this;
}

Barbershop& Barbershop::operator+=(std::shared_ptr<Master> master) {
    if (!master) throw NullObjectException("Barbershop::operator+=", "master");
    if (isBlank(master->getName())) throw InvalidDataException("master", "the master has no name or login");
    if (masters.contains(*master)) throw DuplicateObjectException("Master with login", master->getLogin());
    masters.add(master);
    log("ADD", "Master '" + master->getName() + "'");
    return *this;
}

Barbershop& Barbershop::operator+=(std::shared_ptr<Administrator> administrator) {
    if (!administrator) throw NullObjectException("Barbershop::operator+=", "administrator");
    if (isBlank(administrator->getName())) {
        throw InvalidDataException("administrator", "the administrator has no name or login");
    }
    if (administrators.contains(*administrator)) {
        throw DuplicateObjectException("Administrator with login", administrator->getLogin());
    }
    administrators.add(administrator);
    log("ADD", "Administrator '" + administrator->getName() + "'");
    return *this;
}

Barbershop& Barbershop::operator+=(std::shared_ptr<Service> service) {
    if (!service) throw NullObjectException("Barbershop::operator+=", "service");
    if (isBlank(service->getName())) throw InvalidDataException("service", "the service has no name");
    if (services.contains(*service)) throw DuplicateObjectException("Service", service->getName());
    services.add(service);
    log("ADD", "Service '" + service->getName() + "'");
    return *this;
}

Barbershop& Barbershop::operator+=(std::shared_ptr<Appointment> appointment) {
    if (!appointment) throw NullObjectException("Barbershop::operator+=", "appointment");
    if (!clients.contains(*appointment->getClient()) || !masters.contains(*appointment->getMaster())
        || !services.contains(*appointment->getService())) {
        throw LinkViolationException("the appointment of '" + appointment->getClient()->getName() + "'",
            "the client, the master and the service must be registered in the barbershop");
    }
    if (appointment->getClient()->isBanned()) {
        throw ClientBannedException(appointment->getClient()->getName());
    }
    checkAvailability(*appointment->getMaster(), *appointment->getClient(), appointment->getStart(),
        appointment->getDuration(), nullptr);
    appointment->assignId();
    appointments.add(appointment);
    return *this;
}

Barbershop& Barbershop::operator-=(std::shared_ptr<Client> client) {
    if (!client) throw NullObjectException("Barbershop::operator-=", "client");
    int index = clients.indexOf(*client);
    if (index < 0) throw ObjectNotFoundException("Client", client->getPhone());
    int used = countIf(appointments, isOfClient, *client);
    if (used > 0) {
        throw LinkViolationException("client '" + client->getName() + "'",
            std::to_string(used) + " appointment(s) in the history refer to this client");
    }
    clients.removeAt(index);
    log("REMOVE", "Client '" + client->getName() + "'");
    return *this;
}

Barbershop& Barbershop::operator-=(std::shared_ptr<Master> master) {
    if (!master) throw NullObjectException("Barbershop::operator-=", "master");
    int index = masters.indexOf(*master);
    if (index < 0) throw ObjectNotFoundException("Master", master->getLogin());
    int used = countIf(appointments, isOfMaster, *master);
    if (used > 0) {
        throw LinkViolationException("master '" + master->getName() + "'",
            std::to_string(used) + " appointment(s) in the history refer to this master");
    }
    masters.removeAt(index);
    log("REMOVE", "Master '" + master->getName() + "'");
    return *this;
}

Barbershop& Barbershop::operator-=(std::shared_ptr<Administrator> administrator) {
    if (!administrator) throw NullObjectException("Barbershop::operator-=", "administrator");
    int index = administrators.indexOf(*administrator);
    if (index < 0) throw ObjectNotFoundException("Administrator", administrator->getLogin());
    if (administrators.getCount() == 1) {
        throw InvalidOperationException("remove administrator", "the last administrator cannot be removed");
    }
    administrators.removeAt(index);
    log("REMOVE", "Administrator '" + administrator->getName() + "'");
    return *this;
}

Barbershop& Barbershop::operator-=(std::shared_ptr<Service> service) {
    if (!service) throw NullObjectException("Barbershop::operator-=", "service");
    int index = services.indexOf(*service);
    if (index < 0) throw ObjectNotFoundException("Service", service->getName());
    int used = countIf(appointments, isOfService, *service);
    if (used > 0) {
        throw LinkViolationException("service '" + service->getName() + "'",
            std::to_string(used) + " appointment(s) in the history use this service");
    }
    for (const auto& master : masters) master->removeOffer(service->getId());
    services.removeAt(index);
    log("REMOVE", "Service '" + service->getName() + "'");
    return *this;
}

Barbershop& Barbershop::operator-=(std::shared_ptr<Appointment> appointment) {
    if (!appointment) throw NullObjectException("Barbershop::operator-=", "appointment");
    int index = appointments.indexOf(*appointment);
    if (index < 0) throw ObjectNotFoundException("Appointment", std::to_string(appointment->getId()));
    if (appointment->isPaid()) {
        throw LinkViolationException("appointment #" + std::to_string(appointment->getId()),
            "a payment is attached to it");
    }
    appointments.removeAt(index);
    log("DELETE", "Appointment #" + std::to_string(appointment->getId()));
    return *this;
}

std::shared_ptr<Client> Barbershop::loginClient(std::string_view phone) const {
    for (const auto& client : clients) {
        if (client->getPhone() == phone) return client;
    }
    throw ObjectNotFoundException("Client with phone", std::string(phone));
}

std::shared_ptr<Person> Barbershop::loginStaff(std::string_view login, std::string_view password) const {
    for (const auto& master : masters) {
        if (master->getLogin() == login && master->checkPassword(password)) return master;
    }
    for (const auto& administrator : administrators) {
        if (administrator->getLogin() == login && administrator->checkPassword(password)) return administrator;
    }
    throw AuthenticationException();
}

std::shared_ptr<Appointment> Barbershop::findMasterConflict(const Master& master, const DateTime& start,
    int duration, const Appointment* ignored) const {
    for (const auto& item : appointments) {
        if (item.get() == ignored || !item->occupiesTime()) continue;
        if (*item->getMaster() == master && item->overlapsInterval(start, duration)) return item;
    }
    return nullptr;
}

std::shared_ptr<Appointment> Barbershop::findClientConflict(const Client& client, const DateTime& start,
    int duration, const Appointment* ignored) const {
    for (const auto& item : appointments) {
        if (item.get() == ignored || !item->occupiesTime()) continue;
        if (*item->getClient() == client && item->overlapsInterval(start, duration)) return item;
    }
    return nullptr;
}

void Barbershop::checkAvailability(const Master& master, const Client& client, const DateTime& start,
    int duration, const Appointment* ignored) const {
    if (!master.worksAt(start, duration)) {
        throw OutsideWorkingHoursException(master.getName(), start.toString());
    }
    std::shared_ptr<Appointment> masterConflict = findMasterConflict(master, start, duration, ignored);
    if (masterConflict) {
        throw TimeAlreadyBookedException("Master", master.getName(), masterConflict->getTimeRange());
    }
    std::shared_ptr<Appointment> clientConflict = findClientConflict(client, start, duration, ignored);
    if (clientConflict) {
        throw TimeAlreadyBookedException("Client", client.getName(), clientConflict->getTimeRange());
    }
}

bool Barbershop::isSlotFree(const Master& master, const Client* client, const DateTime& start,
    int duration) const {
    if (!master.worksAt(start, duration)) return false;
    if (findMasterConflict(master, start, duration, nullptr)) return false;
    return client == nullptr || findClientConflict(*client, start, duration, nullptr) == nullptr;
}

std::shared_ptr<Appointment> Barbershop::book(std::shared_ptr<Client> client, std::shared_ptr<Master> master,
    std::shared_ptr<Service> service, const DateTime& start, std::shared_ptr<Person> actor) {
    requireActor(actor);
    if (!client) throw NullObjectException("Barbershop::book", "client");
    if (!master) throw NullObjectException("Barbershop::book", "master");
    if (!isStaff(actor) && actor.get() != client.get()) {
        throw AccessDeniedException("book an appointment for another client");
    }
    if (dynamic_cast<const Master*>(actor.get()) != nullptr && actor.get() != master.get()) {
        throw AccessDeniedException("book an appointment to another master");
    }
    if (start < DateTime::now()) {
        throw InvalidDataException("date and time", "the appointment must not be in the past");
    }
    std::shared_ptr<Appointment> appointment = std::make_shared<Appointment>(client, master, service, start);
    *this += appointment;
    log("BOOK", "Appointment #" + std::to_string(appointment->getId()) + " " + appointment->getTimeRange()
        + ", client '" + client->getName() + "', master '" + master->getName() + "', by " + actor->getName());
    return appointment;
}

void Barbershop::reschedule(std::shared_ptr<Appointment> appointment, const DateTime& newStart,
    std::shared_ptr<Person> actor) {
    requireActor(actor);
    if (!appointment) throw NullObjectException("Barbershop::reschedule", "appointment");
    if (!isStaff(actor) && !isOwnerOf(actor, *appointment)) {
        throw AccessDeniedException("reschedule an appointment of another client");
    }
    if (dynamic_cast<const Master*>(actor.get()) != nullptr
        && actor.get() != appointment->getMaster().get()) {
        throw AccessDeniedException("reschedule an appointment of another master");
    }
    if (appointment->isPaid()) {
        throw InvalidOperationException("reschedule", "a paid appointment cannot be rescheduled");
    }
    if (!appointment->isActive()) {
        throw InvalidStatusTransitionException(statusToString(appointment->getStatus()), "Rescheduled");
    }
    checkClientTimeLimit(actor, *appointment, "reschedule");
    if (newStart < DateTime::now()) {
        throw InvalidDataException("date and time", "the appointment must not be in the past");
    }
    checkAvailability(*appointment->getMaster(), *appointment->getClient(), newStart,
        appointment->getDuration(), appointment.get());
    DateTime oldStart = appointment->getStart();
    appointment->setStart(newStart);
    log("RESCHEDULE", "Appointment #" + std::to_string(appointment->getId()) + " from " + oldStart.toString()
        + " to " + newStart.toString() + " by " + actor->getName());
}

void Barbershop::changeStatus(std::shared_ptr<Appointment> appointment, AppointmentStatus newStatus,
    std::shared_ptr<Person> actor) {
    requireActor(actor);
    if (!appointment) throw NullObjectException("Barbershop::changeStatus", "appointment");

    bool isCancel = newStatus == AppointmentStatus::Cancelled;
    if (!isStaff(actor) && !(isCancel && isOwnerOf(actor, *appointment))) {
        throw AccessDeniedException("set the status '" + statusToString(newStatus) + "'");
    }
    if (dynamic_cast<const Master*>(actor.get()) != nullptr
        && actor.get() != appointment->getMaster().get()) {
        throw AccessDeniedException("change the status of an appointment of another master");
    }
    if (!appointment->canTransitionTo(newStatus)) {
        throw InvalidStatusTransitionException(statusToString(appointment->getStatus()), statusToString(newStatus));
    }

    DateTime now = DateTime::now();
    if (newStatus == AppointmentStatus::Cancelled) {
        checkClientTimeLimit(actor, *appointment, "cancel");
        appointment->getClient()->registerCancel();
    }
    else if (newStatus == AppointmentStatus::Arrived) {
        if (appointment->getStart().dayNumber() > now.dayNumber()) {
            throw InvalidOperationException("mark as arrived", "the appointment is on a later day");
        }
    }
    else if (newStatus == AppointmentStatus::NoShow) {
        if (appointment->getStart() > now) {
            throw InvalidOperationException("mark as no-show", "the appointment has not started yet");
        }
        appointment->getClient()->registerNoShow();
    }
    std::string oldStatus = statusToString(appointment->getStatus());
    appointment->setStatus(newStatus);
    log("STATUS", "Appointment #" + std::to_string(appointment->getId()) + ": " + oldStatus + " -> "
        + statusToString(newStatus) + " by " + actor->getName());
}

std::shared_ptr<Payment> Barbershop::pay(std::shared_ptr<Appointment> appointment, PaymentMethod method,
    std::shared_ptr<Person> actor) {
    requireActor(actor);
    if (!appointment) throw NullObjectException("Barbershop::pay", "appointment");
    if (!isAdministrator(actor)) throw AccessDeniedException("take payments");
    if (appointment->getStatus() != AppointmentStatus::Arrived) {
        throw InvalidOperationException("payment", "the visit must be marked as arrived first");
    }
    if (appointment->isPaid()) throw InvalidOperationException("payment", "this appointment is already paid");

    std::shared_ptr<Client> client = appointment->getClient();
    int discount = client->getDiscountPercent();
    double amount = roundMoney(appointment->getPrice() * client->getPriceFactor());
    std::shared_ptr<Payment> payment = std::make_shared<Payment>(appointment, amount, discount, method,
        DateTime::now());
    payments.add(payment);
    appointment->markPaid();
    client->registerVisit(amount);
    log("PAY", "Payment #" + std::to_string(payment->getId()) + " for appointment #"
        + std::to_string(appointment->getId()) + ", " + formatMoney(amount) + " (" + paymentMethodToString(method) + ")");
    return payment;
}

void Barbershop::banClient(std::shared_ptr<Client> client, std::string_view reason, std::shared_ptr<Person> actor) {
    requireStaff(actor, "ban clients");
    if (!client) throw NullObjectException("Barbershop::banClient", "client");
    if (client->isBanned()) throw InvalidOperationException("ban", "the client is already banned");
    client->ban(reason);
    log("BAN", "Client '" + client->getName() + "' by " + actor->getName());
}

void Barbershop::unbanClient(std::shared_ptr<Client> client, std::shared_ptr<Person> actor) {
    requireActor(actor);
    if (!isAdministrator(actor)) throw AccessDeniedException("unban clients (administrator only)");
    if (!client) throw NullObjectException("Barbershop::unbanClient", "client");
    if (!client->isBanned()) throw InvalidOperationException("unban", "the client is not banned");
    client->unban();
    log("UNBAN", "Client '" + client->getName() + "' by " + actor->getName());
}

void Barbershop::addClientNote(std::shared_ptr<Client> client, std::string_view text, std::shared_ptr<Person> actor) {
    requireStaff(actor, "write private notes");
    if (!client) throw NullObjectException("Barbershop::addClientNote", "client");
    if (isBlank(text)) throw InvalidDataException("note", "must not be empty");
    client->addNote("[" + DateTime::now().dateString() + "] " + actor->getName() + ": " + std::string(text));
    log("NOTE", "Private note added to client '" + client->getName() + "' by " + actor->getName());
}

void Barbershop::changeSchedule(std::shared_ptr<Master> master, int startMinute, int endMinute, int dayOff,
    std::shared_ptr<Person> actor) {
    requireCanManage(actor, master, "change the work schedule");
    Master trial(*master);
    trial.setWorkHours(startMinute, endMinute);
    trial.setDayOff(dayOff);

    DateTime now = DateTime::now();
    int outside = static_cast<int>(std::count_if(appointments.begin(), appointments.end(),
        [&](const std::shared_ptr<Appointment>& item) {
            return item->occupiesTime() && *item->getMaster() == *master && item->getStart() >= now
                && !trial.worksAt(item->getStart(), item->getDuration());
        }));
    if (outside > 0) {
        throw LinkViolationException("master '" + master->getName() + "'", std::to_string(outside)
            + " upcoming appointment(s) would be outside the new schedule, move or cancel them first");
    }
    master->setWorkHours(startMinute, endMinute);
    master->setDayOff(dayOff);
    log("SCHEDULE", "Master '" + master->getName() + "' now works " + formatMinutes(startMinute) + "-"
        + formatMinutes(endMinute) + ", day off " + std::to_string(dayOff));
}

void Barbershop::setOffer(std::shared_ptr<Master> master, std::shared_ptr<Service> service, double customPrice,
    int customDuration, std::shared_ptr<Person> actor) {
    requireCanManage(actor, master, "change the services");
    if (!service) throw NullObjectException("Barbershop::setOffer", "service");
    if (!services.contains(*service)) {
        throw LinkViolationException("service '" + service->getName() + "'", "it is not in the catalog");
    }
    master->setOffer(service, customPrice, customDuration);
    log("OFFER", "Master '" + master->getName() + "' offers '" + service->getName() + "'");
}

void Barbershop::removeOffer(std::shared_ptr<Master> master, std::shared_ptr<Service> service,
    std::shared_ptr<Person> actor) {
    requireCanManage(actor, master, "change the services");
    if (!service) throw NullObjectException("Barbershop::removeOffer", "service");
    if (!master->removeOffer(service->getId())) {
        throw ObjectNotFoundException("Offer", service->getName());
    }
    log("OFFER", "Master '" + master->getName() + "' no longer offers '" + service->getName() + "'");
}

std::ostream& operator<<(std::ostream& os, const Barbershop& barbershop) {
    os << barbershop.name << " (" << barbershop.address << ") | Masters: " << barbershop.masters.getCount()
        << " | Services: " << barbershop.services.getCount() << " | Clients: " << barbershop.clients.getCount()
        << " | Appointments: " << barbershop.appointments.getCount();
    return os;
}
