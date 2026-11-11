#include "slot_finder.h"
#include <cmath>
#include "config.h"
#include "predicates.h"
#include "exceptions/null_object_exception.h"
#include "exceptions/invalid_data_exception.h"

namespace {
    bool touchesNeighbour(const Collection<Appointment>& masterAppointments, const Master& master,
        const DateTime& start, int duration) {
        DateTime end = start.addMinutes(duration);
        if (start.minuteOfDay() == master.getWorkStart() || end.minuteOfDay() == master.getWorkEnd()) return true;
        for (const auto& item : masterAppointments) {
            if (item->isActive() && (item->getEnd() == start || item->getStart() == end)) return true;
        }
        return false;
    }

    int calculateScore(const SlotRequest& request, const SlotOption& option, double averagePrice,
        const Collection<Appointment>& masterAppointments, int duration) {
        double score = 0.0;

        if (request.preferredMinute < 0) {
            score += WEIGHT_TIME;
        }
        else {
            int distance = std::abs(option.start.minuteOfDay() - request.preferredMinute);
            if (distance < TIME_TOLERANCE_MINUTES) {
                score += WEIGHT_TIME * (1.0 - static_cast<double>(distance) / TIME_TOLERANCE_MINUTES);
            }
        }

        long long dayIndex = option.start.dayNumber() - request.firstDay.dayNumber();
        score += WEIGHT_SPEED * (1.0 - static_cast<double>(dayIndex) / request.days);
        score += WEIGHT_RATING * option.master->getRating() / 5.0;

        if (touchesNeighbour(masterAppointments, *option.master, option.start, duration)) score += WEIGHT_NO_GAPS;

        if (option.price <= averagePrice) score += WEIGHT_PRICE;
        else score += WEIGHT_PRICE * averagePrice / option.price;

        return static_cast<int>(std::lround(score));
    }

    bool isBetter(const SlotOption& first, const SlotOption& second) {
        if (first.score != second.score) return first.score > second.score;
        return first.start < second.start;
    }

    void sortOptions(std::vector<SlotOption>& options) {
        int count = static_cast<int>(options.size());
        for (int i = 1; i < count; ++i) {
            SlotOption current = options[i];
            int j = i - 1;
            while (j >= 0 && isBetter(current, options[j])) {
                options[j + 1] = options[j];
                j--;
            }
            options[j + 1] = current;
        }
    }
}

std::vector<DateTime> findFreeStartTimes(const Barbershop& shop, const Master& master,
    const DateTime& day, int duration) {
    std::vector<DateTime> result;
    DateTime now = DateTime::now();
    for (int minute = master.getWorkStart(); minute + duration <= master.getWorkEnd();
        minute += GRID_STEP_MINUTES) {
        DateTime start = day.withMinuteOfDay(minute);
        if (start < now) continue;
        if (shop.isMasterFree(master, start, duration)) result.push_back(start);
    }
    return result;
}

std::vector<SlotOption> suggestSlots(const Barbershop& shop, const SlotRequest& request) {
    if (!request.service) throw NullObjectException("suggestSlots", "service");
    if (request.days < 1 || request.days > MAX_SEARCH_DAYS) {
        throw InvalidDataException("days", "must be 1-" + std::to_string(MAX_SEARCH_DAYS));
    }

    Collection<Master> candidates;
    if (request.master) candidates.add(request.master);
    else candidates = findAll(shop.getMasters(), canPerformService, *request.service);

    Collection<Appointment> allAppointments = shop.getAppointments();
    std::vector<SlotOption> options;
    double priceSum = 0.0;

    for (const auto& master : candidates) {
        double price = master->calculatePrice(*request.service);
        int duration = master->calculateDuration(*request.service);
        for (int dayIndex = 0; dayIndex < request.days; ++dayIndex) {
            DateTime day = request.firstDay.addDays(dayIndex);
            std::vector<DateTime> times = findFreeStartTimes(shop, *master, day, duration);
            for (const DateTime& time : times) {
                SlotOption option = { master, time, price, 0 };
                options.push_back(option);
                priceSum += price;
            }
        }
    }
    if (options.empty()) return options;

    double averagePrice = priceSum / static_cast<double>(options.size());
    for (SlotOption& option : options) {
        Collection<Appointment> ofMaster = findAll(allAppointments, isOfMaster, *option.master);
        int duration = option.master->calculateDuration(*request.service);
        option.score = calculateScore(request, option, averagePrice, ofMaster, duration);
    }

    sortOptions(options);
    if (static_cast<int>(options.size()) > SUGGESTION_LIMIT) options.resize(SUGGESTION_LIMIT);
    return options;
}
