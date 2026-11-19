#pragma once
#include <string>
#include "barbershop.h"

class DataStorage {
private:
    std::string filename;

public:
    explicit DataStorage(const std::string& filenameValue);

    void save(const Barbershop& shop) const;
    Barbershop load() const;
};
