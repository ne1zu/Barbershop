#include "AppointmentsPage.h"
#include <QComboBox>
#include <QDateEdit>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include "AppointmentActions.h"
#include "UiUtils.h"

AppointmentsPage::AppointmentsPage(AppContext* contextValue, QWidget* parent)
    : PageBase(contextValue, parent), loading(false) {
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 20, 28, 24);
    root->setSpacing(14);

    QHBoxLayout* filters = new QHBoxLayout;
    filters->setSpacing(10);
    modeCombo = new QComboBox;
    modeCombo->addItem(QString::fromUtf8("Активные записи"), 0);
    modeCombo->addItem(QString::fromUtf8("Все записи"), 1);
    modeCombo->addItem(QString::fromUtf8("На выбранную дату"), 2);
    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("dd.MM.yyyy");
    dateEdit->setEnabled(false);
    dateEdit->setMinimumWidth(150);
    masterCombo = new QComboBox;
    masterCombo->setMinimumWidth(220);
    filters->addWidget(modeCombo);
    filters->addWidget(dateEdit);
    filters->addWidget(masterCombo);
    filters->addStretch(1);
    root->addLayout(filters);

    table = makeTable({ "№", QString::fromUtf8("Дата"), QString::fromUtf8("Время"), QString::fromUtf8("Клиент"),
        QString::fromUtf8("Мастер"), QString::fromUtf8("Услуга"), QString::fromUtf8("Цена"),
        QString::fromUtf8("Статус"), QString::fromUtf8("Оплата") });
    fixColumn(table, 0, 84);
    fixColumn(table, 1, 124);
    fixColumn(table, 2, 134);
    fixColumn(table, 6, 118);
    fixColumn(table, 7, 150);
    fixColumn(table, 8, 130);
    root->addWidget(table, 1);

    QHBoxLayout* buttons = new QHBoxLayout;
    buttons->setSpacing(8);
    confirmButton = makeButton(QString::fromUtf8("Подтвердить"), "success");
    arrivedButton = makeButton(QString::fromUtf8("Пришёл"), "success");
    noShowButton = makeButton(QString::fromUtf8("Неявка"), "danger");
    rescheduleButton = makeButton(QString::fromUtf8("Перенести"), "ghost");
    cancelButton = makeButton(QString::fromUtf8("Отменить"), "danger");
    payButton = makeButton(QString::fromUtf8("Оплатить"), "primary");
    removeButton = makeButton(QString::fromUtf8("Удалить"), "ghost");
    buttons->addWidget(confirmButton);
    buttons->addWidget(arrivedButton);
    buttons->addWidget(noShowButton);
    buttons->addWidget(rescheduleButton);
    buttons->addWidget(cancelButton);
    buttons->addStretch(1);
    buttons->addWidget(payButton);
    buttons->addWidget(removeButton);
    payButton->setVisible(ctx()->isAdmin());
    removeButton->setVisible(ctx()->isAdmin());
    root->addLayout(buttons);

    connect(modeCombo, &QComboBox::currentIndexChanged, this, &AppointmentsPage::onFilterChanged);
    connect(dateEdit, &QDateEdit::dateChanged, this, &AppointmentsPage::onFilterChanged);
    connect(masterCombo, &QComboBox::currentIndexChanged, this, &AppointmentsPage::onFilterChanged);
    connect(table, &QTableWidget::itemSelectionChanged, this, &AppointmentsPage::onSelectionChanged);
    connect(confirmButton, &QPushButton::clicked, this, &AppointmentsPage::onConfirm);
    connect(arrivedButton, &QPushButton::clicked, this, &AppointmentsPage::onArrived);
    connect(noShowButton, &QPushButton::clicked, this, &AppointmentsPage::onNoShow);
    connect(rescheduleButton, &QPushButton::clicked, this, &AppointmentsPage::onReschedule);
    connect(cancelButton, &QPushButton::clicked, this, &AppointmentsPage::onCancel);
    connect(payButton, &QPushButton::clicked, this, &AppointmentsPage::onPay);
    connect(removeButton, &QPushButton::clicked, this, &AppointmentsPage::onRemove);
}

QString AppointmentsPage::title() const { return QString::fromUtf8("Записи"); }

std::shared_ptr<Appointment> AppointmentsPage::selected() const {
    int row = table->currentRow();
    if (row < 0 || table->item(row, 0) == nullptr) return nullptr;
    return findAppointmentById(ctx()->shop(), table->item(row, 0)->data(Qt::UserRole).toInt());
}

void AppointmentsPage::refresh() {
    loading = true;
    int keep = masterCombo->currentIndex() >= 0 ? masterCombo->currentData().toInt() : -1;
    masterCombo->clear();
    masterCombo->addItem(QString::fromUtf8("Все мастера"), -1);
    for (const auto& master : shop().getMasters()) masterCombo->addItem(qs(master->getName()), master->getId());
    for (int i = 0; i < masterCombo->count(); ++i) {
        if (masterCombo->itemData(i).toInt() == keep) masterCombo->setCurrentIndex(i);
    }
    loading = false;
    fillTable();
}

void AppointmentsPage::onFilterChanged() {
    if (loading) return;
    dateEdit->setEnabled(modeCombo->currentData().toInt() == 2);
    fillTable();
}

void AppointmentsPage::fillTable() {
    int keepId = -1;
    std::shared_ptr<Appointment> previous = selected();
    if (previous) keepId = previous->getId();

    table->setRowCount(0);
    int mode = modeCombo->currentData().toInt();
    int masterId = masterCombo->currentIndex() >= 0 ? masterCombo->currentData().toInt() : -1;
    DateTime day = toDateTime(dateEdit->date());
    int restoreRow = -1;
    for (const auto& appointment : sortedAppointments(shop())) {
        if (mode == 0 && !appointment->isActive()) continue;
        if (mode == 2 && appointment->getStart().dayNumber() != day.dayNumber()) continue;
        if (masterId >= 0 && appointment->getMaster()->getId() != masterId) continue;
        int row = table->rowCount();
        table->insertRow(row);
        QTableWidgetItem* number = makeItem(QString::number(appointment->getId()));
        number->setData(Qt::UserRole, appointment->getId());
        table->setItem(row, 0, number);
        table->setItem(row, 1, makeItem(formatDay(appointment->getStart())));
        table->setItem(row, 2, makeItem(timeRangeText(*appointment)));
        table->setItem(row, 3, makeItem(qs(appointment->getClient()->getName())));
        table->setItem(row, 4, makeItem(qs(appointment->getMaster()->getName())));
        table->setItem(row, 5, makeItem(qs(appointment->getService()->getName())));
        table->setItem(row, 6, makeItem(money(appointment->getPrice())));
        table->setCellWidget(row, 7, makeBadgeCell(statusText(appointment->getStatus()),
            statusKind(appointment->getStatus())));
        if (appointment->isPaid()) table->setCellWidget(row, 8, makeBadgeCell(QString::fromUtf8("Оплачено"), "paid"));
        else table->setItem(row, 8, makeItem("—"));
        if (appointment->getId() == keepId) restoreRow = row;
    }
    if (restoreRow >= 0) table->selectRow(restoreRow);
    onSelectionChanged();
}

void AppointmentsPage::onSelectionChanged() {
    ActionState state = actions::stateFor(ctx(), selected());
    setActionEnabled(confirmButton, state.confirm, state.confirmHint);
    setActionEnabled(arrivedButton, state.arrived, state.arrivedHint);
    setActionEnabled(noShowButton, state.noShow, state.noShowHint);
    setActionEnabled(rescheduleButton, state.reschedule, state.rescheduleHint);
    setActionEnabled(cancelButton, state.cancel, state.cancelHint);
    setActionEnabled(payButton, state.pay, state.payHint);
    removeButton->setEnabled(state.remove);
}

void AppointmentsPage::afterAction(bool changed) {
    if (!changed) return;
    emit dataChanged();
    fillTable();
}

void AppointmentsPage::onConfirm() {
    std::shared_ptr<Appointment> appointment = selected();
    if (appointment) afterAction(actions::confirm(ctx(), this, appointment));
}

void AppointmentsPage::onArrived() {
    std::shared_ptr<Appointment> appointment = selected();
    if (appointment) afterAction(actions::markArrived(ctx(), this, appointment));
}

void AppointmentsPage::onNoShow() {
    std::shared_ptr<Appointment> appointment = selected();
    if (appointment) afterAction(actions::markNoShow(ctx(), this, appointment));
}

void AppointmentsPage::onReschedule() {
    std::shared_ptr<Appointment> appointment = selected();
    if (appointment) afterAction(actions::reschedule(ctx(), this, appointment));
}

void AppointmentsPage::onCancel() {
    std::shared_ptr<Appointment> appointment = selected();
    if (appointment) afterAction(actions::cancel(ctx(), this, appointment));
}

void AppointmentsPage::onPay() {
    std::shared_ptr<Appointment> appointment = selected();
    if (appointment) afterAction(actions::pay(ctx(), this, appointment));
}

void AppointmentsPage::onRemove() {
    std::shared_ptr<Appointment> appointment = selected();
    if (appointment) afterAction(actions::remove(ctx(), this, appointment));
}
