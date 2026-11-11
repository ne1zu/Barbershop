#pragma once
#include <memory>
#include <vector>
#include "barbershop.h"

struct SlotRequest {
    std::shared_ptr<Service> service;
    std::shared_ptr<Master> master;
    DateTime firstDay;
    int days;
    int preferredMinute;
};

struct SlotOption {
    std::shared_ptr<Master> master;
    DateTime start;
    double price;
    int score;
};

std::vector<DateTime> findFreeStartTimes(const Barbershop& shop, const Master& master,
    const DateTime& day, int duration);
std::vector<SlotOption> suggestSlots(const Barbershop& shop, const SlotRequest& request);
