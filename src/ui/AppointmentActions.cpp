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
            throw AccessDeniedException("переносить записи другого мастера");
        }
        if (appointment.isPaid()) {
            throw InvalidOperationException("перенос", "оплаченную запись перенести нельзя");
        }
        if (!appointment.isActive()) {
            throw InvalidStatusTransitionException(statusText(appointment.getStatus()).toStdString(), "перенесена");
        }
        if (appointment.getClient()->isBanned()) {
            throw ClientBannedException(appointment.getClient()->getName());
        }
        if (ctx->isClient()) {
            long long minutesLeft = appointment.getStart().toMinutes() - DateTime::now().toMinutes();
            if (minutesLeft < CANCEL_LIMIT_HOURS * 60) {
                throw InvalidOperationException("перенос", "клиент может сделать это не позже чем за "
                    + std::to_string(CANCEL_LIMIT_HOURS) + " ч до начала, позвоните в барбершоп");
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
    ActionState state;
    if (!appointment) return state;

    bool mine = true;
    if (ctx->isMaster()) mine = appointment->getMaster().get() == ctx->master().get();
    if (ctx->isClient()) mine = appointment->getClient().get() == ctx->client().get();
    if (!mine) return state;

    const AppointmentStatus status = appointment->getStatus();
    const long long minutesToStart = appointment->getStart().toMinutes() - DateTime::now().toMinutes();
    const QString startText = formatClock(appointment->getStart().minuteOfDay());
    const QString finished = QString::fromUtf8("Запись уже завершена (статус «%1»).").arg(statusText(status));

    if (ctx->isClient() && minutesToStart < CANCEL_LIMIT_HOURS * 60 && appointment->isActive()) {
        state.cancelHint = QString::fromUtf8("Клиент может отменить запись не позже чем за %1 ч до начала. "
            "Позвоните в барбершоп.").arg(CANCEL_LIMIT_HOURS);
        state.rescheduleHint = QString::fromUtf8("Клиент может перенести запись не позже чем за %1 ч до начала. "
            "Позвоните в барбершоп.").arg(CANCEL_LIMIT_HOURS);
    }
    else if (!appointment->isActive()) {
        state.cancelHint = finished;
        state.rescheduleHint = finished;
    }
    else {
        state.cancel = appointment->canTransitionTo(AppointmentStatus::Cancelled);
        state.reschedule = !appointment->isPaid();
        if (!state.cancel) state.cancelHint = finished;
        if (!state.reschedule) state.rescheduleHint = QString::fromUtf8("Оплаченную запись перенести нельзя.");
    }

    if (ctx->isStaff()) {
        if (status == AppointmentStatus::Pending && minutesToStart >= 0) {
            state.confirm = true;
        }
        else if (status == AppointmentStatus::Pending) {
            state.confirmHint = QString::fromUtf8("Время записи уже прошло, подтвердить её нельзя.");
        }
        else {
            state.confirmHint = QString::fromUtf8("Подтвердить можно только запись в статусе «Ожидает».");
        }

        if (status == AppointmentStatus::Confirmed && minutesToStart <= ARRIVAL_EARLY_MINUTES) {
            state.arrived = true;
        }
        else if (status == AppointmentStatus::Pending) {
            state.arrivedHint = QString::fromUtf8("Сначала подтвердите запись.");
        }
        else if (status == AppointmentStatus::Confirmed) {
            state.arrivedHint = QString::fromUtf8("Приход можно отметить за %1 минут до начала (с %2).")
                .arg(ARRIVAL_EARLY_MINUTES).arg(formatClock(appointment->getStart().minuteOfDay() - ARRIVAL_EARLY_MINUTES));
        }
        else {
            state.arrivedHint = finished;
        }

        if (status == AppointmentStatus::Confirmed && minutesToStart <= 0) {
            state.noShow = true;
        }
        else if (status == AppointmentStatus::Pending) {
            state.noShowHint = QString::fromUtf8("Сначала подтвердите запись.");
        }
        else if (status == AppointmentStatus::Confirmed) {
            state.noShowHint = QString::fromUtf8("Неявку можно отметить, когда наступило время записи (с %1).").arg(startText);
        }
        else {
            state.noShowHint = finished;
        }
    }

    if (ctx->isAdmin()) {
        if (status == AppointmentStatus::Arrived && !appointment->isPaid()) state.pay = true;
        else if (appointment->isPaid()) state.payHint = QString::fromUtf8("Запись уже оплачена.");
        else state.payHint = QString::fromUtf8("Оплатить можно после отметки «Пришёл».");
        state.remove = true;
    }
    return state;
}

bool confirm(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment) {
    if (!askConfirm(parent, QString::fromUtf8("Подтверждение записи"),
        QString::fromUtf8("Вы точно хотите подтвердить эту запись?\n\n") + describe(*appointment),
        QString::fromUtf8("Да, подтвердить"))) {
        return false;
    }
    try {
        if (appointment->getStart() < DateTime::now()) {
            throw InvalidOperationException("подтверждение", "время записи уже прошло");
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
    if (!askConfirm(parent, QString::fromUtf8("Клиент пришёл"),
        QString::fromUtf8("Вы точно хотите отметить, что клиент пришёл?\n\n") + describe(*appointment),
        QString::fromUtf8("Да, пришёл"))) {
        return false;
    }
    return changeStatus(ctx, parent, appointment, AppointmentStatus::Arrived);
}

bool markNoShow(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment) {
    if (!askConfirm(parent, QString::fromUtf8("Неявка клиента"),
        QString::fromUtf8("Вы точно хотите отметить неявку? Это попадёт в статистику клиента.\n\n")
            + describe(*appointment), QString::fromUtf8("Да, неявка"))) {
        return false;
    }
    return changeStatus(ctx, parent, appointment, AppointmentStatus::NoShow);
}

bool cancel(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment) {
    if (!askConfirm(parent, QString::fromUtf8("Отмена записи"),
        QString::fromUtf8("Вы точно хотите отменить эту запись?\n\n") + describe(*appointment), QString::fromUtf8("Да, отменить запись"))) {
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
        box.setText(QString::fromUtf8("Вы точно хотите принять оплату? Выберите способ.\n\n%1\n\nЦена: %2\nСкидка клиента: %3%\nК оплате: %4")
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
        QString::fromUtf8("Вы точно хотите удалить запись из базы? Восстановить её будет нельзя.\n\n") + describe(*appointment),
        QString::fromUtf8("Да, удалить"))) {
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
