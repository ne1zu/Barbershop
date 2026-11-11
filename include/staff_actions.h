#pragma once
#include <memory>
#include "barbershop.h"

std::shared_ptr<Client> pickOrCreateClient(Barbershop& shop);
void runAppointmentActions(Barbershop& shop, const std::shared_ptr<Person>& actor,
    const Collection<Appointment>& list);
void runStaffNewAppointment(Barbershop& shop, const std::shared_ptr<Person>& actor);
void runStaffAppointments(Barbershop& shop, const std::shared_ptr<Person>& actor);
void runClientCards(Barbershop& shop, const std::shared_ptr<Person>& actor);
void runDayJournal(const Barbershop& shop);
void runOffersEditor(Barbershop& shop, const std::shared_ptr<Master>& master);
void runScheduleEditor(const std::shared_ptr<Master>& master);
void printMasterStatistics(const Barbershop& shop, const Master& master);
