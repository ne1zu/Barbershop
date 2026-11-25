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
            throw AccessDeniedException(action + " другого мастера");
        }
    }

    void checkClientTimeLimit(const std::shared_ptr<Person>& actor, const Appointment& appointment,
        const std::string& operation) {
        if (isStaff(actor)) return;
        long long minutesLeft = appointment.getStart().toMinutes() - DateTime::now().toMinutes();
        if (minutesLeft < CANCEL_LIMIT_HOURS * 60) {
            throw InvalidOperationException(operation, "клиент может сделать это не позже чем за "
                + std::to_string(CANCEL_LIMIT_HOURS) + " ч до начала, позвоните в барбершоп");
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
    if (isBlank(nameValue)) throw InvalidDataException("название барбершопа", "не может быть пустым");
    if (!isStorableText(nameValue)) {
        throw InvalidDataException("название барбершопа", "не должно содержать «|» и переводов строки");
    }
    name = nameValue;
}

void Barbershop::setAddress(std::string_view addressValue) {
    if (isBlank(addressValue)) throw InvalidDataException("адрес", "не может быть пустым");
    if (!isStorableText(addressValue)) throw InvalidDataException("адрес", "не должно содержать «|» и переводов строки");
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
        throw LinkViolationException("записи №" + std::to_string(appointment->getId()),
            "клиент, мастер и услуга должны быть зарегистрированы в барбершопе");
    }
    if (appointments.contains(*appointment)) {
        throw DuplicateObjectException("Запись №", std::to_string(appointment->getId()));
    }
    if (appointment->occupiesTime()) {
        std::shared_ptr<Appointment> masterConflict = findMasterConflict(*appointment->getMaster(),
            appointment->getStart(), appointment->getDuration(), nullptr);
        if (masterConflict) {
            throw TimeAlreadyBookedException("Мастер", appointment->getMaster()->getName(),
                masterConflict->getTimeRange());
        }
        std::shared_ptr<Appointment> clientConflict = findClientConflict(*appointment->getClient(),
            appointment->getStart(), appointment->getDuration(), nullptr);
        if (clientConflict) {
            throw TimeAlreadyBookedException("Клиент", appointment->getClient()->getName(),
                clientConflict->getTimeRange());
        }
    }
    appointments.add(appointment);
}

void Barbershop::restorePayment(std::shared_ptr<Payment> payment) {
    if (!payment) throw NullObjectException("Barbershop::restorePayment", "payment");
    if (!appointments.contains(*payment->getAppointment())) {
        throw LinkViolationException("платежа №" + std::to_string(payment->getId()),
            "запись не зарегистрирована в барбершопе");
    }
    if (payments.contains(*payment)) {
        throw DuplicateObjectException("Платёж №", std::to_string(payment->getId()));
    }
    if (!payment->getAppointment()->isPaid()) {
        throw InvalidOperationException("восстановление платежа", "запись №"
            + std::to_string(payment->getAppointment()->getId()) + " не отмечена как оплаченная");
    }
    for (const auto& other : payments) {
        if (other->getAppointment() == payment->getAppointment()) {
            throw DuplicateObjectException("Платёж по записи №", std::to_string(payment->getAppointment()->getId()));
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
    if (isBlank(client->getName())) throw InvalidDataException("клиент", "у клиента нет имени или телефона");
    if (clients.contains(*client)) throw DuplicateObjectException("Клиент с телефоном", client->getPhone());
    clients.add(client);
    log("ADD", "Client '" + client->getName() + "'");
    return *this;
}

Barbershop& Barbershop::operator+=(std::shared_ptr<Master> master) {
    if (!master) throw NullObjectException("Barbershop::operator+=", "master");
    if (isBlank(master->getName())) throw InvalidDataException("мастер", "у мастера нет имени или логина");
    if (masters.contains(*master)) throw DuplicateObjectException("Мастер с логином", master->getLogin());
    masters.add(master);
    log("ADD", "Master '" + master->getName() + "'");
    return *this;
}

Barbershop& Barbershop::operator+=(std::shared_ptr<Administrator> administrator) {
    if (!administrator) throw NullObjectException("Barbershop::operator+=", "administrator");
    if (isBlank(administrator->getName())) {
        throw InvalidDataException("администратор", "у администратора нет имени или логина");
    }
    if (administrators.contains(*administrator)) {
        throw DuplicateObjectException("Администратор с логином", administrator->getLogin());
    }
    administrators.add(administrator);
    log("ADD", "Administrator '" + administrator->getName() + "'");
    return *this;
}

Barbershop& Barbershop::operator+=(std::shared_ptr<Service> service) {
    if (!service) throw NullObjectException("Barbershop::operator+=", "service");
    if (isBlank(service->getName())) throw InvalidDataException("услуга", "у услуги нет названия");
    if (services.contains(*service)) throw DuplicateObjectException("Услуга", service->getName());
    services.add(service);
    log("ADD", "Service '" + service->getName() + "'");
    return *this;
}

Barbershop& Barbershop::operator+=(std::shared_ptr<Appointment> appointment) {
    if (!appointment) throw NullObjectException("Barbershop::operator+=", "appointment");
    if (!clients.contains(*appointment->getClient()) || !masters.contains(*appointment->getMaster())
        || !services.contains(*appointment->getService())) {
        throw LinkViolationException("записи клиента «" + appointment->getClient()->getName() + "»",
            "клиент, мастер и услуга должны быть зарегистрированы в барбершопе");
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
    if (index < 0) throw ObjectNotFoundException("Клиент", client->getPhone());
    int used = countIf(appointments, isOfClient, *client);
    if (used > 0) {
        throw LinkViolationException("клиента «" + client->getName() + "»",
            std::to_string(used) + " записей в истории связаны с этим клиентом");
    }
    clients.removeAt(index);
    log("REMOVE", "Client '" + client->getName() + "'");
    return *this;
}

Barbershop& Barbershop::operator-=(std::shared_ptr<Master> master) {
    if (!master) throw NullObjectException("Barbershop::operator-=", "master");
    int index = masters.indexOf(*master);
    if (index < 0) throw ObjectNotFoundException("Мастер", master->getLogin());
    int used = countIf(appointments, isOfMaster, *master);
    if (used > 0) {
        throw LinkViolationException("мастера «" + master->getName() + "»",
            std::to_string(used) + " записей в истории связаны с этим мастером");
    }
    masters.removeAt(index);
    log("REMOVE", "Master '" + master->getName() + "'");
    return *this;
}

Barbershop& Barbershop::operator-=(std::shared_ptr<Administrator> administrator) {
    if (!administrator) throw NullObjectException("Barbershop::operator-=", "administrator");
    int index = administrators.indexOf(*administrator);
    if (index < 0) throw ObjectNotFoundException("Администратор", administrator->getLogin());
    if (administrators.getCount() == 1) {
        throw InvalidOperationException("удаление администратора", "нельзя удалить последнего администратора");
    }
    administrators.removeAt(index);
    log("REMOVE", "Administrator '" + administrator->getName() + "'");
    return *this;
}

Barbershop& Barbershop::operator-=(std::shared_ptr<Service> service) {
    if (!service) throw NullObjectException("Barbershop::operator-=", "service");
    int index = services.indexOf(*service);
    if (index < 0) throw ObjectNotFoundException("Услуга", service->getName());
    int used = countIf(appointments, isOfService, *service);
    if (used > 0) {
        throw LinkViolationException("услуги «" + service->getName() + "»",
            std::to_string(used) + " записей в истории используют эту услугу");
    }
    for (const auto& master : masters) master->removeOffer(service->getId());
    services.removeAt(index);
    log("REMOVE", "Service '" + service->getName() + "'");
    return *this;
}

Barbershop& Barbershop::operator-=(std::shared_ptr<Appointment> appointment) {
    if (!appointment) throw NullObjectException("Barbershop::operator-=", "appointment");
    int index = appointments.indexOf(*appointment);
    if (index < 0) throw ObjectNotFoundException("Запись №", std::to_string(appointment->getId()));
    if (appointment->isPaid()) {
        throw LinkViolationException("записи №" + std::to_string(appointment->getId()),
            "к ней привязан платёж");
    }
    appointments.removeAt(index);
    log("DELETE", "Appointment #" + std::to_string(appointment->getId()));
    return *this;
}

std::shared_ptr<Client> Barbershop::loginClient(std::string_view phone) const {
    for (const auto& client : clients) {
        if (client->getPhone() == phone) return client;
    }
    throw ObjectNotFoundException("Клиент с телефоном", std::string(phone));
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
        throw TimeAlreadyBookedException("Мастер", master.getName(), masterConflict->getTimeRange());
    }
    std::shared_ptr<Appointment> clientConflict = findClientConflict(client, start, duration, ignored);
    if (clientConflict) {
        throw TimeAlreadyBookedException("Клиент", client.getName(), clientConflict->getTimeRange());
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
        throw AccessDeniedException("записывать другого клиента");
    }
    if (dynamic_cast<const Master*>(actor.get()) != nullptr && actor.get() != master.get()) {
        throw AccessDeniedException("записываться к мастеру от имени другого мастера");
    }
    if (start < DateTime::now()) {
        throw InvalidDataException("дата и время", "нельзя записать на прошедшее время");
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
        throw AccessDeniedException("переносить запись другого клиента");
    }
    if (dynamic_cast<const Master*>(actor.get()) != nullptr
        && actor.get() != appointment->getMaster().get()) {
        throw AccessDeniedException("переносить запись другого мастера");
    }
    if (appointment->isPaid()) {
        throw InvalidOperationException("перенос", "оплаченную запись перенести нельзя");
    }
    if (!appointment->isActive()) {
        throw InvalidStatusTransitionException(statusTitle(appointment->getStatus()), "перенесена");
    }
    checkClientTimeLimit(actor, *appointment, "reschedule");
    if (newStart < DateTime::now()) {
        throw InvalidDataException("дата и время", "нельзя записать на прошедшее время");
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
        throw AccessDeniedException("ставить статус «" + statusTitle(newStatus) + "»");
    }
    if (dynamic_cast<const Master*>(actor.get()) != nullptr
        && actor.get() != appointment->getMaster().get()) {
        throw AccessDeniedException("менять статус записи другого мастера");
    }
    if (!appointment->canTransitionTo(newStatus)) {
        throw InvalidStatusTransitionException(statusTitle(appointment->getStatus()), statusTitle(newStatus));
    }

    DateTime now = DateTime::now();
    if (newStatus == AppointmentStatus::Cancelled) {
        checkClientTimeLimit(actor, *appointment, "cancel");
        appointment->getClient()->registerCancel();
    }
    else if (newStatus == AppointmentStatus::Arrived) {
        if (appointment->getStart().toMinutes() - now.toMinutes() > ARRIVAL_EARLY_MINUTES) {
            throw InvalidOperationException("отметка «Пришёл»", "это можно сделать не раньше чем за "
                + std::to_string(ARRIVAL_EARLY_MINUTES) + " минут до начала");
        }
    }
    else if (newStatus == AppointmentStatus::NoShow) {
        if (appointment->getStart() > now) {
            throw InvalidOperationException("отметка «Неявка»", "время записи ещё не наступило");
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
    if (!isAdministrator(actor)) throw AccessDeniedException("принимать оплату");
    if (appointment->getStatus() != AppointmentStatus::Arrived) {
        throw InvalidOperationException("оплата", "сначала отметьте, что клиент пришёл");
    }
    if (appointment->isPaid()) throw InvalidOperationException("оплата", "запись уже оплачена");

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
    requireStaff(actor, "блокировать клиентов");
    if (!client) throw NullObjectException("Barbershop::banClient", "client");
    if (client->isBanned()) throw InvalidOperationException("блокировка", "клиент уже заблокирован");
    client->ban(reason);
    log("BAN", "Client '" + client->getName() + "' by " + actor->getName());
}

void Barbershop::unbanClient(std::shared_ptr<Client> client, std::shared_ptr<Person> actor) {
    requireActor(actor);
    if (!isAdministrator(actor)) throw AccessDeniedException("разблокировать клиентов (только администратор)");
    if (!client) throw NullObjectException("Barbershop::unbanClient", "client");
    if (!client->isBanned()) throw InvalidOperationException("разблокировка", "клиент не заблокирован");
    client->unban();
    log("UNBAN", "Client '" + client->getName() + "' by " + actor->getName());
}

void Barbershop::addClientNote(std::shared_ptr<Client> client, std::string_view text, std::shared_ptr<Person> actor) {
    requireStaff(actor, "писать приватные заметки");
    if (!client) throw NullObjectException("Barbershop::addClientNote", "client");
    if (isBlank(text)) throw InvalidDataException("заметка", "не может быть пустым");
    client->addNote("[" + DateTime::now().dateString() + "] " + actor->getName() + ": " + std::string(text));
    log("NOTE", "Private note added to client '" + client->getName() + "' by " + actor->getName());
}

void Barbershop::changeSchedule(std::shared_ptr<Master> master, int startMinute, int endMinute, int dayOff,
    std::shared_ptr<Person> actor) {
    requireCanManage(actor, master, "менять рабочий график");
    Master trial(*master);
    trial.setWorkHours(startMinute, endMinute);
    trial.setDayOff(dayOff);

    DateTime now = DateTime::now();
    int outside = static_cast<int>(std::count_if(appointments.begin(), appointments.end(),
        [&](const std::shared_ptr<Appointment>& item) {
            return item->isActive() && *item->getMaster() == *master
                && item->getStart().toMinutes() + item->getDuration() > now.toMinutes()
                && !trial.worksAt(item->getStart(), item->getDuration());
        }));
    if (outside > 0) {
        throw LinkViolationException("мастера «" + master->getName() + "»", std::to_string(outside)
            + " текущих и будущих записей выходят за новый график, сначала перенесите или отмените их");
    }
    master->setWorkHours(startMinute, endMinute);
    master->setDayOff(dayOff);
    log("SCHEDULE", "Master '" + master->getName() + "' now works " + formatMinutes(startMinute) + "-"
        + formatMinutes(endMinute) + ", day off " + std::to_string(dayOff));
}

void Barbershop::setOffer(std::shared_ptr<Master> master, std::shared_ptr<Service> service, double customPrice,
    int customDuration, std::shared_ptr<Person> actor) {
    requireCanManage(actor, master, "менять услуги");
    if (!service) throw NullObjectException("Barbershop::setOffer", "service");
    if (!services.contains(*service)) {
        throw LinkViolationException("услуги «" + service->getName() + "»", "её нет в каталоге");
    }
    master->setOffer(service, customPrice, customDuration);
    log("OFFER", "Master '" + master->getName() + "' offers '" + service->getName() + "'");
}

void Barbershop::removeOffer(std::shared_ptr<Master> master, std::shared_ptr<Service> service,
    std::shared_ptr<Person> actor) {
    requireCanManage(actor, master, "менять услуги");
    if (!service) throw NullObjectException("Barbershop::removeOffer", "service");
    if (!master->removeOffer(service->getId())) {
        throw ObjectNotFoundException("Услуга мастера", service->getName());
    }
    log("OFFER", "Master '" + master->getName() + "' no longer offers '" + service->getName() + "'");
}

std::ostream& operator<<(std::ostream& os, const Barbershop& barbershop) {
    os << barbershop.name << " (" << barbershop.address << ") | Masters: " << barbershop.masters.getCount()
        << " | Services: " << barbershop.services.getCount() << " | Clients: " << barbershop.clients.getCount()
        << " | Appointments: " << barbershop.appointments.getCount();
    return os;
}
