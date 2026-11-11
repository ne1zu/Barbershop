#include "master.h"
#include "config.h"
#include "utils.h"
#include "exceptions/exceptions.h"

int Master::counter = 0;

Master::Master()
    : Person(0, "", ""), experience(0), rating(0.0), workStart(DEFAULT_WORK_START),
    workEnd(DEFAULT_WORK_END), dayOff(DEFAULT_DAY_OFF) {
}

Master::Master(std::string_view nameValue, std::string_view phoneValue, std::string_view loginValue,
    std::string_view password, int experienceValue, double ratingValue)
    : Person(++counter, "", ""), experience(0), rating(0.0), workStart(DEFAULT_WORK_START),
    workEnd(DEFAULT_WORK_END), dayOff(DEFAULT_DAY_OFF) {
    setName(nameValue);
    setPhone(phoneValue);
    setLogin(loginValue);
    setPassword(password);
    setExperience(experienceValue);
    setRating(ratingValue);
}

std::string Master::getLogin() const { return login; }
int Master::getExperience() const { return experience; }
double Master::getRating() const { return rating; }
int Master::getWorkStart() const { return workStart; }
int Master::getWorkEnd() const { return workEnd; }
int Master::getDayOff() const { return dayOff; }
Collection<MasterService> Master::getOffers() const { return offers; }

void Master::setLogin(std::string_view loginValue) {
    if (!isValidLogin(loginValue)) {
        throw InvalidDataException("login", "3-20 characters: letters, digits or '_'");
    }
    login = loginValue;
}

void Master::setPassword(std::string_view password) {
    if (static_cast<int>(password.size()) < MIN_PASSWORD_LENGTH) {
        throw InvalidDataException("password", "must be at least " + std::to_string(MIN_PASSWORD_LENGTH) + " characters");
    }
    passwordHash = hashPassword(password);
}

void Master::setExperience(int experienceValue) {
    if (experienceValue < 0) {
        throw InvalidDataException("experience", "must not be negative, got " + std::to_string(experienceValue));
    }
    experience = experienceValue;
}

void Master::setRating(double ratingValue) {
    if (ratingValue < 0.0 || ratingValue > 5.0) {
        throw InvalidDataException("rating", "must be between 0 and 5");
    }
    rating = ratingValue;
}

void Master::setWorkHours(int startMinute, int endMinute) {
    if (startMinute < 0 || endMinute > 24 * 60 || startMinute >= endMinute) {
        throw InvalidDataException("work hours", "the start must be earlier than the end within one day");
    }
    if (startMinute % GRID_STEP_MINUTES != 0 || endMinute % GRID_STEP_MINUTES != 0) {
        throw InvalidDataException("work hours", "must be a multiple of " + std::to_string(GRID_STEP_MINUTES) + " minutes");
    }
    workStart = startMinute;
    workEnd = endMinute;
}

void Master::setDayOff(int dayValue) {
    if (dayValue < -1 || dayValue > 6) {
        throw InvalidDataException("day off", "use -1 for none or 0 (Monday) .. 6 (Sunday)");
    }
    dayOff = dayValue;
}

bool Master::checkPassword(std::string_view password) const {
    return passwordHash == hashPassword(password);
}

bool Master::worksAt(const DateTime& start, int duration) const {
    if (dayOff >= 0 && start.dayOfWeek() == dayOff) return false;
    int begin = start.minuteOfDay();
    return begin >= workStart && begin + duration <= workEnd;
}

void Master::setOffer(std::shared_ptr<Service> service, double customPrice, int customDuration) {
    if (!service) throw NullObjectException("Master::setOffer", "service");
    std::shared_ptr<MasterService> existing = findOffer(service->getId());
    if (existing) {
        existing->setCustomPrice(customPrice);
        existing->setCustomDuration(customDuration);
        return;
    }
    offers.add(std::make_shared<MasterService>(service, customPrice, customDuration));
}

bool Master::removeOffer(int serviceId) {
    int count = offers.getCount();
    for (int i = 0; i < count; ++i) {
        if (offers.get(i)->getService()->getId() == serviceId) return offers.removeAt(i);
    }
    return false;
}

std::shared_ptr<MasterService> Master::findOffer(int serviceId) const {
    for (const auto& offer : offers) {
        if (offer->getService()->getId() == serviceId) return offer;
    }
    return nullptr;
}

bool Master::canPerform(const Service& service) const {
    return findOffer(service.getId()) != nullptr;
}

double Master::calculatePrice(const Service& service) const {
    std::shared_ptr<MasterService> offer = findOffer(service.getId());
    if (!offer) throw ServiceNotOfferedException(name, service.getName());
    if (offer->hasCustomPrice()) return offer->getCustomPrice();
    return roundMoney(service.getPrice() * getPriceFactor());
}

int Master::calculateDuration(const Service& service) const {
    std::shared_ptr<MasterService> offer = findOffer(service.getId());
    if (!offer) throw ServiceNotOfferedException(name, service.getName());
    if (offer->hasCustomDuration()) return offer->getCustomDuration();
    return service.getLength();
}

bool Master::operator==(const Master& other) const { return login == other.login; }
bool Master::operator<(const Master& other) const { return rating < other.rating; }
bool Master::operator>(const Master& other) const { return rating > other.rating; }

std::string Master::getEntityType() const { return "Master"; }

std::string Master::classify() const {
    if (experience >= TOP_EXPERIENCE) return "Top";
    if (experience >= STANDARD_EXPERIENCE) return "Standard";
    return "Junior";
}

double Master::getPriceFactor() const {
    if (experience >= TOP_EXPERIENCE) return TOP_FACTOR;
    if (experience >= STANDARD_EXPERIENCE) return STANDARD_FACTOR;
    return JUNIOR_FACTOR;
}

void Master::printInfo(std::ostream& os) const {
    os << getEntityType() << " ";
    Person::printInfo(os);
    os << " | Login: " << login << " | Level: " << classify() << " | Experience: " << experience
        << " years | Rating: " << rating;
}

std::istream& operator>>(std::istream& is, Master& master) {
    std::string nameValue;
    std::string phoneValue;
    std::string loginValue;
    std::string passwordValue;
    int experienceValue = 0;
    double ratingValue = 0.0;

    std::cout << "Enter Master Name: ";
    std::getline(is >> std::ws, nameValue);

    std::cout << "Enter Phone (+375XXXXXXXXX): ";
    std::getline(is >> std::ws, phoneValue);

    std::cout << "Enter Login: ";
    std::getline(is >> std::ws, loginValue);

    std::cout << "Enter Password: ";
    std::getline(is >> std::ws, passwordValue);

    std::cout << "Enter Experience (years): ";
    is >> experienceValue;
    if (!is) return is;

    std::cout << "Enter Rating (0-5): ";
    is >> ratingValue;
    if (!is) return is;

    master = Master(nameValue, phoneValue, loginValue, passwordValue, experienceValue, ratingValue);
    return is;
}
