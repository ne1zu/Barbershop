#pragma once
#include <map>
#include <string>
#include "barbershop.h"

struct ShopStatistics {
    double revenue;
    int paymentsCount;
    double averageCheck;
    double smallestPayment;
    double largestPayment;
    int pending;
    int confirmed;
    int arrived;
    int noShow;
    int cancelled;
    int clients;
    int newClients;
    int bannedClients;
    int servedClients;
    std::map<std::string, int> clientsByLevel;
    std::map<std::string, int> visitsByService;
    std::map<std::string, double> revenueByMaster;
    std::string topService;
    int topServiceVisits;
    std::string topMaster;
    double topMasterRevenue;
};

struct MasterStatistics {
    double revenue;
    int paymentsCount;
    double averageCheck;
    int arrived;
    int noShow;
    int cancelled;
    int upcoming;
    int uniqueClients;
};

ShopStatistics calculateStatistics(const Barbershop& shop);
MasterStatistics calculateMasterStatistics(const Barbershop& shop, const Master& master);
