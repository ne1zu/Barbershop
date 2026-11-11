#pragma once
#include <memory>
#include "barbershop.h"

void runManualBooking(Barbershop& shop, const std::shared_ptr<Person>& actor,
    const std::shared_ptr<Client>& client);
void runSmartBooking(Barbershop& shop, const std::shared_ptr<Person>& actor,
    const std::shared_ptr<Client>& client);
