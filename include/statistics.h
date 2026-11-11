#pragma once
#include "barbershop.h"

struct ShopStatistics {
    double revenue;
    int paymentsCount;
    double averageCheck;
    int pending;
    int confirmed;
    int arrived;
    int noShow;
    int cancelled;
    int clients;
    int newClients;
    int bannedClients;
};

struct MasterStatistics {
    double revenue;
    int paymentsCount;
    double averageCheck;
    int arrived;
    int noShow;
    int cancelled;
    int upcoming;
};

ShopStatistics calculateStatistics(const Barbershop& shop);
MasterStatistics calculateMasterStatistics(const Barbershop& shop, const Master& master);
