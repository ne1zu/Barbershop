#pragma once
#include <string>
#include <iostream>
#include <memory>
#include "appointment.h"
#include "datetime.h"

enum class PaymentMethod { Cash, Card };

std::string paymentMethodToString(PaymentMethod method);

class Payment {
private:
    int id;
    std::shared_ptr<Appointment> appointment;
    double amount;
    int discountPercent;
    PaymentMethod method;
    DateTime paidAt;
    static int counter;

public:
    Payment(std::shared_ptr<Appointment> appointmentValue, double amountValue, int discountValue,
        PaymentMethod methodValue, const DateTime& paidAtValue);

    int getId() const;
    std::shared_ptr<Appointment> getAppointment() const;
    double getAmount() const;
    int getDiscountPercent() const;
    PaymentMethod getMethod() const;
    DateTime getPaidAt() const;

    bool operator==(const Payment& other) const;
    bool operator<(const Payment& other) const;
    bool operator>(const Payment& other) const;

    void printInfo(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, const Payment& payment);
};
