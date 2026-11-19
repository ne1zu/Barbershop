#include "slot_finder.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <unordered_map>
#include "config.h"
#include "predicates.h"
#include "exceptions/null_object_exception.h"
#include "exceptions/invalid_data_exception.h"

namespace {
    typedef std::vector<std::shared_ptr<Appointment>> AppointmentList;
    typedef std::unordered_map<int, AppointmentList> AppointmentsByMaster;

    AppointmentsByMaster groupByMaster(const Collection<Appointment>& appointments) {
        AppointmentsByMaster groups;
        std::for_each(appointments.begin(), appointments.end(), [&groups](const std::shared_ptr<Appointment>& item) {
            if (item->occupiesTime()) groups[item->getMaster()->getId()].push_back(item);
        });
        return groups;
    }

    bool touchesNeighbour(const AppointmentList& masterAppointments, const Master& master,
        const DateTime& start, int duration) {
        DateTime end = start.addMinutes(duration);
        if (start.minuteOfDay() == master.getWorkStart() || end.minuteOfDay() == master.getWorkEnd()) return true;
        return std::any_of(masterAppointments.begin(), masterAppointments.end(),
            [&](const std::shared_ptr<Appointment>& item) { return item->getEnd() == start || item->getStart() == end; });
    }

    int calculateScore(const SlotRequest& request, const SlotOption& option, double averagePrice,
        const AppointmentList& masterAppointments, int duration) {
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

    void collectOptions(const Barbershop& shop, const SlotRequest& request,
        const std::shared_ptr<Master>& master, std::vector<SlotOption>& options) {
        double price = master->calculatePrice(*request.service);
        int duration = master->calculateDuration(*request.service);
        for (int dayIndex = 0; dayIndex < request.days; ++dayIndex) {
            DateTime day = request.firstDay.addDays(dayIndex);
            std::vector<DateTime> times = findFreeStartTimes(shop, *master, request.client.get(), day, duration);
            for (const DateTime& time : times) options.push_back({ master, time, price, 0 });
        }
    }
}

std::vector<DateTime> findFreeStartTimes(const Barbershop& shop, const Master& master, const Client* client,
    const DateTime& day, int duration) {
    std::vector<DateTime> result;
    DateTime now = DateTime::now();
    for (int minute = master.getWorkStart(); minute + duration <= master.getWorkEnd();
        minute += GRID_STEP_MINUTES) {
        DateTime start = day.withMinuteOfDay(minute);
        if (start < now) continue;
        if (shop.isSlotFree(master, client, start, duration)) result.push_back(start);
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

    std::vector<SlotOption> options;
    for (const auto& master : candidates) collectOptions(shop, request, master, options);
    if (options.empty()) return options;

    double priceSum = std::accumulate(options.begin(), options.end(), 0.0,
        [](double sum, const SlotOption& option) { return sum + option.price; });
    double averagePrice = priceSum / static_cast<double>(options.size());

    AppointmentsByMaster byMaster = groupByMaster(shop.getAppointments());
    for (SlotOption& option : options) {
        int duration = option.master->calculateDuration(*request.service);
        option.score = calculateScore(request, option, averagePrice, byMaster[option.master->getId()], duration);
    }

    std::stable_sort(options.begin(), options.end(), isBetter);
    if (static_cast<int>(options.size()) > SUGGESTION_LIMIT) options.resize(SUGGESTION_LIMIT);
    return options;
}
