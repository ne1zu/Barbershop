#include "SchedulePage.h"
#include <QCheckBox>
#include <QDateEdit>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "BookingPanel.h"
#include "ScheduleGrid.h"
#include "UiUtils.h"

SchedulePage::SchedulePage(AppContext* contextValue, QWidget* parent)
    : PageBase(contextValue, parent), selectedId(-1) {
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 20, 28, 24);
    root->setSpacing(14);

    QHBoxLayout* toolbar = new QHBoxLayout;
    toolbar->setSpacing(8);
    QPushButton* prev = makeButton("‹", "ghost");
    prev->setObjectName("arrowButton");
    QPushButton* next = makeButton("›", "ghost");
    next->setObjectName("arrowButton");
    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("dd.MM.yyyy");
    dateEdit->setMinimumWidth(140);
    QPushButton* today = makeButton(QString::fromUtf8("Сегодня"), "ghost");
    weekdayLabel = makeLabel(QString(), "sectionTitle");
    cancelledCheck = new QCheckBox(QString::fromUtf8("Показывать отменённые и неявки"));
    toolbar->addWidget(prev);
    toolbar->addWidget(dateEdit);
    toolbar->addWidget(next);
    toolbar->addWidget(today);
    toolbar->addSpacing(12);
    toolbar->addWidget(weekdayLabel);
    toolbar->addStretch(1);
    toolbar->addWidget(cancelledCheck);
    root->addLayout(toolbar);

    QHBoxLayout* legend = new QHBoxLayout;
    legend->setSpacing(8);
    legend->addWidget(makeLabel(QString::fromUtf8("Статусы:"), "muted"));
    legend->addWidget(makeBadge(statusText(AppointmentStatus::Pending), "pending"));
    legend->addWidget(makeBadge(statusText(AppointmentStatus::Confirmed), "confirmed"));
    legend->addWidget(makeBadge(statusText(AppointmentStatus::Arrived), "arrived"));
    legend->addWidget(makeBadge(statusText(AppointmentStatus::NoShow), "noshow"));
    legend->addWidget(makeBadge(statusText(AppointmentStatus::Cancelled), "cancelled"));
    legend->addSpacing(16);
    legend->addWidget(makeLabel(QString::fromUtf8("Двойной клик по свободному времени создаёт запись"), "mutedSmall"));
    legend->addStretch(1);
    root->addLayout(legend);

    QHBoxLayout* content = new QHBoxLayout;
    content->setSpacing(16);
    grid = new ScheduleGrid(shop());
    content->addWidget(grid, 1);
    buildDetails();
    content->addWidget(detailsCard);
    root->addLayout(content, 1);

    connect(prev, &QPushButton::clicked, this, &SchedulePage::onPrevDay);
    connect(next, &QPushButton::clicked, this, &SchedulePage::onNextDay);
    connect(today, &QPushButton::clicked, this, &SchedulePage::onToday);
    connect(dateEdit, &QDateEdit::dateChanged, this, &SchedulePage::onDateChanged);
    connect(cancelledCheck, &QCheckBox::toggled, this, &SchedulePage::onShowCancelled);
    connect(grid, &ScheduleGrid::appointmentSelected, this, &SchedulePage::onSelected);
    connect(grid, &ScheduleGrid::emptySlotActivated, this, &SchedulePage::onEmptySlot);

    grid->setDay(toDateTime(dateEdit->date()));
    weekdayLabel->setText(fullDayTitle(toDateTime(dateEdit->date())));
    updateDetails();
}

QString SchedulePage::title() const { return QString::fromUtf8("Журнал записей"); }

void SchedulePage::buildDetails() {
    detailsCard = makeCard();
    detailsCard->setFixedWidth(330);
    QVBoxLayout* outer = new QVBoxLayout(detailsCard);
    outer->setContentsMargins(20, 20, 20, 20);
    outer->setSpacing(10);

    emptyLabel = makeLabel(QString::fromUtf8("Выберите запись в журнале,\nчтобы увидеть детали и быстрые действия."),
        "emptyState");
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setWordWrap(true);
    outer->addWidget(emptyLabel, 1);

    detailsBox = new QWidget;
    QVBoxLayout* box = new QVBoxLayout(detailsBox);
    box->setContentsMargins(0, 0, 0, 0);
    box->setSpacing(8);

    QHBoxLayout* badges = new QHBoxLayout;
    statusBadge = makeBadge(QString(), "pending");
    paidBadge = makeBadge(QString::fromUtf8("Оплачено"), "paid");
    badges->addWidget(statusBadge);
    badges->addWidget(paidBadge);
    badges->addStretch(1);
    box->addLayout(badges);

    clientLabel = makeLabel(QString(), "bigName");
    clientLabel->setWordWrap(true);
    phoneLabel = makeLabel(QString(), "muted");
    box->addWidget(clientLabel);
    box->addWidget(phoneLabel);
    box->addSpacing(6);
    box->addWidget(makeInfoRow(QString::fromUtf8("Услуга"), &serviceValue));
    box->addWidget(makeInfoRow(QString::fromUtf8("Мастер"), &masterValue));
    box->addWidget(makeInfoRow(QString::fromUtf8("Время"), &timeValue));
    box->addWidget(makeInfoRow(QString::fromUtf8("Длительность"), &durationValue));
    box->addWidget(makeInfoRow(QString::fromUtf8("Цена"), &priceValue));
    box->addWidget(makeInfoRow(QString::fromUtf8("Скидка клиента"), &discountValue));
    box->addSpacing(6);
    box->addWidget(makeLabel(QString::fromUtf8("ЗАМЕТКИ О КЛИЕНТЕ · ТОЛЬКО ДЛЯ ПЕРСОНАЛА"), "statTitle"));
    QWidget* notesHost = new QWidget;
    notesLayout = new QVBoxLayout(notesHost);
    notesLayout->setContentsMargins(0, 0, 0, 0);
    notesLayout->setSpacing(6);
    box->addWidget(notesHost);
    box->addStretch(1);

    QGridLayout* buttons = new QGridLayout;
    buttons->setSpacing(8);
    confirmButton = makeButton(QString::fromUtf8("Подтвердить"), "success");
    arrivedButton = makeButton(QString::fromUtf8("Пришёл"), "success");
    noShowButton = makeButton(QString::fromUtf8("Неявка"), "danger");
    cancelButton = makeButton(QString::fromUtf8("Отменить"), "danger");
    rescheduleButton = makeButton(QString::fromUtf8("Перенести"), "ghost");
    payButton = makeButton(QString::fromUtf8("Оплатить"), "primary");
    buttons->addWidget(confirmButton, 0, 0);
    buttons->addWidget(arrivedButton, 0, 1);
    buttons->addWidget(noShowButton, 1, 0);
    buttons->addWidget(cancelButton, 1, 1);
    buttons->addWidget(rescheduleButton, 2, 0);
    buttons->addWidget(payButton, 2, 1);
    box->addLayout(buttons);
    payButton->setVisible(ctx()->isAdmin());
    outer->addWidget(detailsBox, 1);

    connect(confirmButton, &QPushButton::clicked, this, &SchedulePage::onConfirm);
    connect(arrivedButton, &QPushButton::clicked, this, &SchedulePage::onArrived);
    connect(noShowButton, &QPushButton::clicked, this, &SchedulePage::onNoShow);
    connect(cancelButton, &QPushButton::clicked, this, &SchedulePage::onCancel);
    connect(rescheduleButton, &QPushButton::clicked, this, &SchedulePage::onReschedule);
    connect(payButton, &QPushButton::clicked, this, &SchedulePage::onPay);
}

std::shared_ptr<Appointment> SchedulePage::current() const {
    if (selectedId < 0) return nullptr;
    return findAppointmentById(ctx()->shop(), selectedId);
}

void SchedulePage::refresh() {
    grid->reload();
    updateDetails();
}

void SchedulePage::updateDetails() {
    std::shared_ptr<Appointment> appointment = current();
    if (!appointment) {
        selectedId = -1;
        grid->setSelectedAppointmentId(-1);
        emptyLabel->setVisible(true);
        detailsBox->setVisible(false);
        return;
    }
    emptyLabel->setVisible(false);
    detailsBox->setVisible(true);

    std::shared_ptr<Client> client = appointment->getClient();
    setBadge(statusBadge, statusText(appointment->getStatus()), statusKind(appointment->getStatus()));
    paidBadge->setVisible(appointment->isPaid());
    clientLabel->setText(qs(client->getName()) + (appointment->isFirstVisit() ? QString::fromUtf8("  · новый") : QString()));
    phoneLabel->setText(qs(client->getPhone()));
    serviceValue->setText(qs(appointment->getService()->getName()));
    masterValue->setText(qs(appointment->getMaster()->getName()));
    timeValue->setText(formatDay(appointment->getStart()) + "  " + appointment->getStart().timeString().c_str()
        + "–" + appointment->getEnd().timeString().c_str());
    durationValue->setText(QString::fromUtf8("%1 мин").arg(appointment->getDuration()));
    priceValue->setText(money(appointment->getPrice()));
    discountValue->setText(QString("%1%").arg(client->getDiscountPercent()));

    clearLayout(notesLayout);
    const std::vector<std::string>& notes = client->getNotes();
    int shown = 0;
    for (int i = static_cast<int>(notes.size()) - 1; i >= 0 && shown < 3; --i, ++shown) {
        QLabel* note = makeLabel(qs(notes[i]), "noteLine");
        note->setWordWrap(true);
        notesLayout->addWidget(note);
    }
    if (notes.empty()) notesLayout->addWidget(makeLabel(QString::fromUtf8("Заметок нет"), "mutedSmall"));
    if (client->isBanned()) {
        QLabel* banned = makeLabel(QString::fromUtf8("Клиент заблокирован: ") + qs(client->getBanReason()), "dangerBanner");
        banned->setWordWrap(true);
        notesLayout->addWidget(banned);
    }

    ActionState state = actions::stateFor(ctx(), appointment);
    confirmButton->setEnabled(state.confirm);
    arrivedButton->setEnabled(state.arrived);
    noShowButton->setEnabled(state.noShow);
    cancelButton->setEnabled(state.cancel);
    rescheduleButton->setEnabled(state.reschedule);
    payButton->setEnabled(state.pay);
    grid->setSelectedAppointmentId(selectedId);
}

void SchedulePage::afterAction(bool changed) {
    if (!changed) return;
    emit dataChanged();
    refresh();
}

void SchedulePage::onPrevDay() { dateEdit->setDate(dateEdit->date().addDays(-1)); }
void SchedulePage::onNextDay() { dateEdit->setDate(dateEdit->date().addDays(1)); }
void SchedulePage::onToday() { dateEdit->setDate(QDate::currentDate()); }

void SchedulePage::onDateChanged() {
    DateTime day = toDateTime(dateEdit->date());
    weekdayLabel->setText(fullDayTitle(day));
    selectedId = -1;
    grid->setDay(day);
    updateDetails();
}

void SchedulePage::onShowCancelled() {
    grid->setShowCancelled(cancelledCheck->isChecked());
}

void SchedulePage::onSelected(int appointmentId) {
    selectedId = appointmentId;
    updateDetails();
}

void SchedulePage::onEmptySlot(int masterId, int minuteOfDay) {
    NewBookingDialog dialog(ctx(), this);
    dialog.presetSlot(masterId, dateEdit->date(), minuteOfDay);
    if (dialog.exec() == QDialog::Accepted) {
        emit dataChanged();
        refresh();
    }
}

void SchedulePage::onConfirm() {
    std::shared_ptr<Appointment> appointment = current();
    if (appointment) afterAction(actions::confirm(ctx(), this, appointment));
}

void SchedulePage::onArrived() {
    std::shared_ptr<Appointment> appointment = current();
    if (appointment) afterAction(actions::markArrived(ctx(), this, appointment));
}

void SchedulePage::onNoShow() {
    std::shared_ptr<Appointment> appointment = current();
    if (appointment) afterAction(actions::markNoShow(ctx(), this, appointment));
}

void SchedulePage::onCancel() {
    std::shared_ptr<Appointment> appointment = current();
    if (appointment) afterAction(actions::cancel(ctx(), this, appointment));
}

void SchedulePage::onReschedule() {
    std::shared_ptr<Appointment> appointment = current();
    if (appointment) afterAction(actions::reschedule(ctx(), this, appointment));
}

void SchedulePage::onPay() {
    std::shared_ptr<Appointment> appointment = current();
    if (appointment) afterAction(actions::pay(ctx(), this, appointment));
}
