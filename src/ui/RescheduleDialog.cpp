#include "RescheduleDialog.h"
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "UiUtils.h"
#include "config.h"
#include "exceptions/exceptions.h"

namespace {
    bool conflictsWithOthers(const Barbershop& shop, const Appointment& self, const DateTime& start) {
        int duration = self.getDuration();
        for (const auto& item : shop.getAppointments()) {
            if (item.get() == &self || !item->occupiesTime()) continue;
            bool sameMaster = *item->getMaster() == *self.getMaster();
            bool sameClient = *item->getClient() == *self.getClient();
            if ((sameMaster || sameClient) && item->overlapsInterval(start, duration)) return true;
        }
        return false;
    }
}

RescheduleDialog::RescheduleDialog(AppContext* contextValue, const std::shared_ptr<Appointment>& appointmentValue,
    QWidget* parent)
    : QDialog(parent), context(contextValue), appointment(appointmentValue) {
    setWindowTitle(QString::fromUtf8("Перенос записи"));
    setMinimumWidth(460);
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(26, 24, 26, 22);
    root->setSpacing(14);
    root->addWidget(makeLabel(QString::fromUtf8("Перенос записи"), "loginTitle"));

    QLabel* info = makeLabel(QString::fromUtf8("%1 · %2\nМастер: %3 · сейчас %4")
        .arg(qs(appointment->getClient()->getName()), qs(appointment->getService()->getName()),
            qs(appointment->getMaster()->getName()), formatDayTime(appointment->getStart())), "muted");
    info->setWordWrap(true);
    root->addWidget(info);

    QFormLayout* form = new QFormLayout;
    form->setSpacing(10);
    dateEdit = new QDateEdit(toQDate(appointment->getStart()));
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("dd.MM.yyyy");
    dateEdit->setMinimumDate(QDate::currentDate());
    timeCombo = new QComboBox;
    form->addRow(makeLabel(QString::fromUtf8("НОВАЯ ДАТА"), "fieldLabel"), dateEdit);
    form->addRow(makeLabel(QString::fromUtf8("НОВОЕ ВРЕМЯ"), "fieldLabel"), timeCombo);
    root->addLayout(form);
    root->addStretch(1);

    QDialogButtonBox* buttons = new QDialogButtonBox;
    okButton = makeButton(QString::fromUtf8("Перенести"), "primary");
    QPushButton* cancel = makeButton(QString::fromUtf8("Отмена"), "ghost");
    buttons->addButton(okButton, QDialogButtonBox::AcceptRole);
    buttons->addButton(cancel, QDialogButtonBox::RejectRole);
    root->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(dateEdit, &QDateEdit::dateChanged, this, &RescheduleDialog::onDateChanged);

    fillTimes();
}

void RescheduleDialog::onDateChanged() {
    fillTimes();
}

void RescheduleDialog::fillTimes() {
    timeCombo->clear();
    const Master& master = *appointment->getMaster();
    int duration = appointment->getDuration();
    DateTime day = toDateTime(dateEdit->date());
    DateTime now = DateTime::now();
    int currentMinute = appointment->getStart().minuteOfDay();
    int preferred = -1;
    for (int minute = master.getWorkStart(); minute + duration <= master.getWorkEnd(); minute += GRID_STEP_MINUTES) {
        DateTime start = day.withMinuteOfDay(minute);
        if (start < now) continue;
        if (!master.worksAt(start, duration)) continue;
        if (conflictsWithOthers(context->shop(), *appointment, start)) continue;
        timeCombo->addItem(formatClock(minute), minute);
        if (minute == currentMinute) preferred = timeCombo->count() - 1;
    }
    if (timeCombo->count() == 0) {
        timeCombo->addItem(QString::fromUtf8("Нет свободного времени"), -1);
        okButton->setEnabled(false);
        return;
    }
    okButton->setEnabled(true);
    if (preferred >= 0) timeCombo->setCurrentIndex(preferred);
}

void RescheduleDialog::accept() {
    Barbershop& shop = context->shop();
    try {
        int minute = timeCombo->currentData().toInt();
        if (minute < 0) return;
        shop.reschedule(appointment, toDateTime(dateEdit->date(), minute), context->user());
        QDialog::accept();
    }
    catch (const TimeAlreadyBookedException&) {
        showError(this, shop);
        fillTimes();
    }
    catch (const OutsideWorkingHoursException&) {
        showError(this, shop);
        fillTimes();
    }
    catch (const InvalidDataException&) {
        showError(this, shop);
        fillTimes();
    }
    catch (...) {
        showError(this, shop);
        QDialog::reject();
    }
}
