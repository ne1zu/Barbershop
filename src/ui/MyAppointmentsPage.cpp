#include "MyAppointmentsPage.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include "AppointmentActions.h"
#include "UiUtils.h"
#include "config.h"

MyAppointmentsPage::MyAppointmentsPage(AppContext* contextValue, QWidget* parent) : PageBase(contextValue, parent) {
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 20, 28, 24);
    root->setSpacing(14);

    QLabel* hint = makeLabel(QString::fromUtf8("Предстоящие записи. Перенос и отмена доступны не позднее чем за %1 ч до начала.")
        .arg(CANCEL_LIMIT_HOURS), "muted");
    hint->setWordWrap(true);
    root->addWidget(hint);

    table = makeTable({ QString::fromUtf8("Дата"), QString::fromUtf8("Время"), QString::fromUtf8("Мастер"),
        QString::fromUtf8("Услуга"), QString::fromUtf8("Цена"), QString::fromUtf8("Статус") });
    root->addWidget(table, 1);

    emptyLabel = makeLabel(QString::fromUtf8("У вас нет предстоящих записей. Перейдите в раздел «Записаться»."), "emptyState");
    emptyLabel->setAlignment(Qt::AlignCenter);
    root->addWidget(emptyLabel, 1);

    QHBoxLayout* buttons = new QHBoxLayout;
    buttons->setSpacing(10);
    rescheduleButton = makeButton(QString::fromUtf8("Перенести"), "ghost");
    cancelButton = makeButton(QString::fromUtf8("Отменить запись"), "danger");
    buttons->addWidget(rescheduleButton);
    buttons->addWidget(cancelButton);
    buttons->addStretch(1);
    root->addLayout(buttons);

    connect(table, &QTableWidget::itemSelectionChanged, this, &MyAppointmentsPage::onSelectionChanged);
    connect(rescheduleButton, &QPushButton::clicked, this, &MyAppointmentsPage::onReschedule);
    connect(cancelButton, &QPushButton::clicked, this, &MyAppointmentsPage::onCancel);
}

QString MyAppointmentsPage::title() const { return QString::fromUtf8("Мои записи"); }

std::shared_ptr<Appointment> MyAppointmentsPage::selected() const {
    int row = table->currentRow();
    if (row < 0 || table->item(row, 0) == nullptr) return nullptr;
    return findAppointmentById(ctx()->shop(), table->item(row, 0)->data(Qt::UserRole).toInt());
}

void MyAppointmentsPage::refresh() {
    table->setRowCount(0);
    std::shared_ptr<Client> me = ctx()->client();
    for (const auto& appointment : sortedAppointments(shop())) {
        if (appointment->getClient().get() != me.get() || !appointment->isActive()) continue;
        int row = table->rowCount();
        table->insertRow(row);
        QTableWidgetItem* first = makeItem(formatDay(appointment->getStart()));
        first->setData(Qt::UserRole, appointment->getId());
        table->setItem(row, 0, first);
        table->setItem(row, 1, makeItem(qs(appointment->getTimeRange())));
        table->setItem(row, 2, makeItem(qs(appointment->getMaster()->getName())));
        table->setItem(row, 3, makeItem(qs(appointment->getService()->getName())));
        table->setItem(row, 4, makeItem(money(appointment->getPrice())));
        table->setCellWidget(row, 5, makeBadgeCell(statusText(appointment->getStatus()),
            statusKind(appointment->getStatus())));
    }
    bool has = table->rowCount() > 0;
    table->setVisible(has);
    emptyLabel->setVisible(!has);
    onSelectionChanged();
}

void MyAppointmentsPage::onSelectionChanged() {
    ActionState state = actions::stateFor(ctx(), selected());
    rescheduleButton->setEnabled(state.reschedule);
    cancelButton->setEnabled(state.cancel);
}

void MyAppointmentsPage::onReschedule() {
    std::shared_ptr<Appointment> appointment = selected();
    if (appointment && actions::reschedule(ctx(), this, appointment)) {
        emit dataChanged();
        refresh();
    }
}

void MyAppointmentsPage::onCancel() {
    std::shared_ptr<Appointment> appointment = selected();
    if (appointment && actions::cancel(ctx(), this, appointment)) {
        emit dataChanged();
        refresh();
    }
}
