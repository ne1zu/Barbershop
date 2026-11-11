#include "payment.h"
#include "utils.h"
#include "exceptions/null_object_exception.h"
#include "exceptions/invalid_data_exception.h"

int Payment::counter = 0;

std::string paymentMethodToString(PaymentMethod method) {
    return method == PaymentMethod::Cash ? "Cash" : "Card";
}

Payment::Payment(std::shared_ptr<Appointment> appointmentValue, double amountValue, int discountValue,
    PaymentMethod methodValue, const DateTime& paidAtValue)
    : id(0), appointment(appointmentValue), amount(amountValue), discountPercent(discountValue),
    method(methodValue), paidAt(paidAtValue) {
    if (!appointment) throw NullObjectException("Payment", "appointment");
    if (amount < 0) throw InvalidDataException("amount", "must not be negative");
    id = ++counter;
}

int Payment::getId() const { return id; }
std::shared_ptr<Appointment> Payment::getAppointment() const { return appointment; }
double Payment::getAmount() const { return amount; }
int Payment::getDiscountPercent() const { return discountPercent; }
PaymentMethod Payment::getMethod() const { return method; }
DateTime Payment::getPaidAt() const { return paidAt; }

bool Payment::operator==(const Payment& other) const { return id == other.id; }
bool Payment::operator<(const Payment& other) const { return amount < other.amount; }
bool Payment::operator>(const Payment& other) const { return amount > other.amount; }

void Payment::printInfo(std::ostream& os) const {
    os << "Payment #" << id << " | " << paidAt << " | " << appointment->getClient()->getName()
        << " | " << formatMoney(amount) << " | " << paymentMethodToString(method);
}

std::ostream& operator<<(std::ostream& os, const Payment& payment) {
    const Appointment& visit = *payment.appointment;
    os << "========== RECEIPT #" << payment.id << " ==========\n";
    os << "Date:     " << payment.paidAt << "\n";
    os << "Client:   " << visit.getClient()->getName() << "\n";
    os << "Master:   " << visit.getMaster()->getName() << "\n";
    os << "Service:  " << visit.getService()->getName() << "\n";
    os << "Price:    " << formatMoney(visit.getPrice()) << "\n";
    os << "Discount: " << payment.discountPercent << "%\n";
    os << "TOTAL:    " << formatMoney(payment.amount) << " (" << paymentMethodToString(payment.method) << ")\n";
    os << "==================================\n";
    return os;
}
