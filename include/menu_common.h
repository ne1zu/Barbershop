#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "barbershop.h"
#include "collection.h"
#include "console_input.h"
#include "exceptions/exceptions.h"

template <typename T>
std::shared_ptr<T> pickFromCollection(const std::string& title, const Collection<T>& collection) {
    if (collection.isEmpty()) {
        throw InvalidOperationException("select " + title, "the list is empty");
    }
    std::cout << "Select " << title << ":\n";
    collection.print(std::cout, "  ");
    int number = readInteger("Number (0 = cancel): ");
    if (number == 0) throw InvalidOperationException("select " + title, "cancelled");
    return collection.at(number - 1);
}

void reportCurrentException();
bool isClientActor(const std::shared_ptr<Person>& actor);
bool isAdministratorActor(const std::shared_ptr<Person>& actor);
std::string dayOffName(int dayOff);
void printToday();
void printDayJournal(const Barbershop& shop, const DateTime& day);
void printOffers(const Master& master);
void printMastersCatalog(const Barbershop& shop);
void printTimes(const std::vector<DateTime>& times);
void printBooked(const Appointment& appointment);
std::shared_ptr<Service> pickOfferedService(const Master& master);
