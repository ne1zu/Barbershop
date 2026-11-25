#include "payment.h"
#include "utils.h"
#include "exceptions/null_object_exception.h"
#include "exceptions/invalid_data_exception.h"

int Payment::counter = 1;

std::string paymentMethodToString(PaymentMethod method) {
    return method == PaymentMethod::Cash ? "Cash" : "Card";
}

bool parsePaymentMethod(const std::string& text, PaymentMethod& method) {
    if (text == "Cash") method = PaymentMethod::Cash;
    else if (text == "Card") method = PaymentMethod::Card;
    else return false;
    return true;
}

Payment::Payment(std::shared_ptr<Appointment> appointmentValue, double amountValue, int discountValue,
    PaymentMethod methodValue, const DateTime& paidAtValue)
    : Payment(counter, appointmentValue, amountValue, discountValue, methodValue, paidAtValue) {
}

Payment::Payment(int idValue, std::shared_ptr<Appointment> appointmentValue, double amountValue,
    int discountValue, PaymentMethod methodValue, const DateTime& paidAtValue)
    : id(idValue), appointment(appointmentValue), amount(amountValue), discountPercent(discountValue),
    method(methodValue), paidAt(paidAtValue) {
    if (!appointment) throw NullObjectException("Payment", "appointment");
    if (amount < 0) throw InvalidDataException("сумма", "не может быть отрицательным");
    if (discountPercent < 0 || discountPercent > 100) {
        throw InvalidDataException("скидка", "от 0 до 100");
    }
    if (idValue <= 0) throw InvalidDataException("номер платежа", "должно быть положительным");
    if (counter <= idValue) counter = idValue + 1;
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
    os << "========== ЧЕК №" << payment.id << " ==========\n";
    os << "Дата:     " << payment.paidAt << "\n";
    os << "Клиент:   " << visit.getClient()->getName() << "\n";
    os << "Мастер:   " << visit.getMaster()->getName() << "\n";
    os << "Услуга:   " << visit.getService()->getName() << "\n";
    os << "Цена:     " << formatMoney(visit.getPrice()) << "\n";
    os << "Скидка:   " << payment.discountPercent << "%\n";
    os << "ИТОГО:    " << formatMoney(payment.amount) << " (" << (payment.method == PaymentMethod::Cash ? "наличные" : "карта") << ")\n";
    os << "==================================\n";
    return os;
}
