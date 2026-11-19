#include "AppointmentActions.h"
#include <QMessageBox>
#include <QPushButton>
#include "RescheduleDialog.h"
#include "UiUtils.h"
#include "config.h"
#include "exceptions/exceptions.h"
#include "report_writer.h"
#include "utils.h"

namespace {
    QString describe(const Appointment& appointment) {
        return qs(appointment.getClient()->getName()) + " · " + qs(appointment.getService()->getName())
            + "\n" + formatDayTime(appointment.getStart()) + " · " + qs(appointment.getMaster()->getName());
    }

    void checkRescheduleAllowed(const AppContext* ctx, const Appointment& appointment) {
        const std::shared_ptr<Master> ownMaster = ctx->master();
        if (ownMaster && ownMaster.get() != appointment.getMaster().get()) {
            throw AccessDeniedException("reschedule an appointment of another master");
        }
        if (appointment.isPaid()) {
            throw InvalidOperationException("reschedule", "a paid appointment cannot be rescheduled");
        }
        if (!appointment.isActive()) {
            throw InvalidStatusTransitionException(statusToString(appointment.getStatus()), "Rescheduled");
        }
        if (appointment.getClient()->isBanned()) {
            throw ClientBannedException(appointment.getClient()->getName());
        }
        if (ctx->isClient()) {
            long long minutesLeft = appointment.getStart().toMinutes() - DateTime::now().toMinutes();
            if (minutesLeft < CANCEL_LIMIT_HOURS * 60) {
                throw InvalidOperationException("reschedule", "clients can do this at least "
                    + std::to_string(CANCEL_LIMIT_HOURS) + " hours before the start, please call the barbershop");
            }
        }
    }

    bool changeStatus(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment,
        AppointmentStatus status) {
        try {
            ctx->shop().changeStatus(appointment, status, ctx->user());
            return true;
        }
        catch (...) {
            showError(parent, ctx->shop());
            return false;
        }
    }
}

namespace actions {

ActionState stateFor(const AppContext* ctx, const std::shared_ptr<Appointment>& appointment) {
    ActionState state = { false, false, false, false, false, false, false };
    if (!appointment) return state;

    bool mine = true;
    if (ctx->isMaster()) mine = appointment->getMaster().get() == ctx->master().get();
    if (ctx->isClient()) mine = appointment->getClient().get() == ctx->client().get();
    if (!mine) return state;

    state.cancel = appointment->canTransitionTo(AppointmentStatus::Cancelled);
    state.reschedule = appointment->isActive() && !appointment->isPaid();
    if (ctx->isStaff()) {
        state.confirm = appointment->canTransitionTo(AppointmentStatus::Confirmed);
        state.arrived = appointment->canTransitionTo(AppointmentStatus::Arrived);
        state.noShow = appointment->canTransitionTo(AppointmentStatus::NoShow);
    }
    state.pay = ctx->isAdmin() && appointment->getStatus() == AppointmentStatus::Arrived && !appointment->isPaid();
    state.remove = ctx->isAdmin();
    return state;
}

bool confirm(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment) {
    try {
        if (appointment->getStart() < DateTime::now()) {
            throw InvalidOperationException("confirm", "the appointment time has already passed");
        }
        ctx->shop().changeStatus(appointment, AppointmentStatus::Confirmed, ctx->user());
        return true;
    }
    catch (...) {
        showError(parent, ctx->shop());
        return false;
    }
}

bool markArrived(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment) {
    return changeStatus(ctx, parent, appointment, AppointmentStatus::Arrived);
}

bool markNoShow(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment) {
    return changeStatus(ctx, parent, appointment, AppointmentStatus::NoShow);
}

bool cancel(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment) {
    if (!askConfirm(parent, QString::fromUtf8("Отмена записи"),
        QString::fromUtf8("Отменить эту запись?\n\n") + describe(*appointment), QString::fromUtf8("Отменить запись"))) {
        return false;
    }
    return changeStatus(ctx, parent, appointment, AppointmentStatus::Cancelled);
}

bool reschedule(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment) {
    try {
        checkRescheduleAllowed(ctx, *appointment);
    }
    catch (...) {
        showError(parent, ctx->shop());
        return false;
    }
    RescheduleDialog dialog(ctx, appointment, parent);
    return dialog.exec() == QDialog::Accepted;
}

bool pay(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment) {
    Barbershop& shop = ctx->shop();
    try {
        std::shared_ptr<Client> client = appointment->getClient();
        double total = roundMoney(appointment->getPrice() * client->getPriceFactor());
        QMessageBox box(QMessageBox::Question, QString::fromUtf8("Оплата визита"), QString(), QMessageBox::NoButton,
            parent);
        box.setText(QString::fromUtf8("%1\n\nЦена: %2\nСкидка клиента: %3%\nК оплате: %4")
            .arg(describe(*appointment), money(appointment->getPrice()))
            .arg(client->getDiscountPercent())
            .arg(money(total)));
        QPushButton* cash = box.addButton(QString::fromUtf8("Наличные"), QMessageBox::AcceptRole);
        QPushButton* card = box.addButton(QString::fromUtf8("Карта"), QMessageBox::AcceptRole);
        box.addButton(QString::fromUtf8("Отмена"), QMessageBox::RejectRole);
        box.setDefaultButton(cash);
        box.exec();
        PaymentMethod method = PaymentMethod::Cash;
        if (box.clickedButton() == card) method = PaymentMethod::Card;
        else if (box.clickedButton() != cash) return false;

        std::shared_ptr<Payment> payment = shop.pay(appointment, method, ctx->user());
        std::string filename = "receipt_" + std::to_string(payment->getId()) + ".txt";
        QString receiptNote;
        try {
            ReportWriter(filename).writeReceipt(*payment);
            shop.log("RECEIPT", "Receipt saved to " + filename);
            receiptNote = QString::fromUtf8("Чек сохранён в файл %1.").arg(qs(filename));
        }
        catch (const BarbershopException&) {
            receiptNote = QString::fromUtf8("Оплата проведена, но файл чека не записан.");
            showError(parent, shop);
        }
        showInfo(parent, QString::fromUtf8("Оплата проведена"),
            QString::fromUtf8("Платёж №%1: %2 (%3).\n%4")
            .arg(payment->getId()).arg(money(payment->getAmount()), paymentMethodText(method), receiptNote));
        return true;
    }
    catch (...) {
        showError(parent, shop);
        return false;
    }
}

bool remove(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment) {
    if (!askConfirm(parent, QString::fromUtf8("Удаление записи"),
        QString::fromUtf8("Удалить запись из базы без возможности восстановления?\n\n") + describe(*appointment),
        QString::fromUtf8("Удалить"))) {
        return false;
    }
    try {
        ctx->shop() -= appointment;
        return true;
    }
    catch (...) {
        showError(parent, ctx->shop());
        return false;
    }
}

}
