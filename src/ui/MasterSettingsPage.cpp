#include "MasterSettingsPage.h"
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QTimeEdit>
#include <QVBoxLayout>
#include "EntityDialogs.h"
#include "UiUtils.h"
#include "config.h"

MasterSettingsPage::MasterSettingsPage(AppContext* contextValue, QWidget* parent)
    : PageBase(contextValue, parent), loading(false) {
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(28, 20, 20, 20);
    QScrollArea* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    outer->addWidget(scroll);
    QWidget* inner = new QWidget;
    scroll->setWidget(inner);
    QVBoxLayout* root = new QVBoxLayout(inner);
    root->setContentsMargins(0, 0, 8, 0);
    root->setSpacing(16);

    QHBoxLayout* top = new QHBoxLayout;
    top->setSpacing(12);
    masterCombo = new QComboBox;
    masterCombo->setMinimumWidth(300);
    masterInfo = makeLabel(QString(), "muted");
    if (ctx()->isAdmin()) top->addWidget(masterCombo);
    top->addWidget(masterInfo, 1);
    root->addLayout(top);

    QFrame* offersCard = makeCard();
    QVBoxLayout* offersBox = new QVBoxLayout(offersCard);
    offersBox->setContentsMargins(22, 20, 22, 20);
    offersBox->setSpacing(12);
    QHBoxLayout* offersHead = new QHBoxLayout;
    offersHead->addWidget(makeLabel(QString::fromUtf8("Услуги и цены"), "sectionTitle"));
    offersHead->addStretch(1);
    QPushButton* addButton = makeButton(QString::fromUtf8("+ Добавить услугу"), "primary");
    editButton = makeButton(QString::fromUtf8("Изменить"), "ghost");
    removeButton = makeButton(QString::fromUtf8("Убрать"), "danger");
    offersHead->addWidget(addButton);
    offersHead->addWidget(editButton);
    offersHead->addWidget(removeButton);
    offersBox->addLayout(offersHead);
    offers = makeTable({ QString::fromUtf8("Услуга"), QString::fromUtf8("Базовая цена"),
        QString::fromUtf8("Цена мастера"), QString::fromUtf8("Длительность") });
    offers->setMinimumHeight(260);
    offersBox->addWidget(offers);
    root->addWidget(offersCard);

    QFrame* scheduleCard = makeCard();
    QGridLayout* grid = new QGridLayout(scheduleCard);
    grid->setContentsMargins(22, 20, 22, 20);
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(8);
    grid->addWidget(makeLabel(QString::fromUtf8("График работы"), "sectionTitle"), 0, 0, 1, 4);
    grid->addWidget(makeLabel(QString::fromUtf8("НАЧАЛО"), "fieldLabel"), 1, 0);
    grid->addWidget(makeLabel(QString::fromUtf8("КОНЕЦ"), "fieldLabel"), 1, 1);
    grid->addWidget(makeLabel(QString::fromUtf8("ВЫХОДНОЙ"), "fieldLabel"), 1, 2);
    startEdit = new QTimeEdit;
    startEdit->setDisplayFormat("HH:mm");
    endEdit = new QTimeEdit;
    endEdit->setDisplayFormat("HH:mm");
    dayOffCombo = new QComboBox;
    dayOffCombo->addItem(QString::fromUtf8("Без выходных"), -1);
    for (int day = 0; day < 7; ++day) dayOffCombo->addItem(weekdayName(day), day);
    QPushButton* saveButton = makeButton(QString::fromUtf8("Сохранить график"), "primary");
    grid->addWidget(startEdit, 2, 0);
    grid->addWidget(endEdit, 2, 1);
    grid->addWidget(dayOffCombo, 2, 2);
    grid->addWidget(saveButton, 2, 3);
    grid->setColumnStretch(2, 1);
    root->addWidget(scheduleCard);
    root->addStretch(1);

    connect(masterCombo, &QComboBox::currentIndexChanged, this, &MasterSettingsPage::onMasterChanged);
    connect(offers, &QTableWidget::itemSelectionChanged, this, &MasterSettingsPage::onSelectionChanged);
    connect(addButton, &QPushButton::clicked, this, &MasterSettingsPage::onAddOffer);
    connect(editButton, &QPushButton::clicked, this, &MasterSettingsPage::onEditOffer);
    connect(removeButton, &QPushButton::clicked, this, &MasterSettingsPage::onRemoveOffer);
    connect(saveButton, &QPushButton::clicked, this, &MasterSettingsPage::onSaveSchedule);
}

QString MasterSettingsPage::title() const {
    return ctx()->isAdmin() ? QString::fromUtf8("Мастера и график") : QString::fromUtf8("Мои услуги и график");
}

std::shared_ptr<Master> MasterSettingsPage::currentMaster() const {
    if (ctx()->isMaster()) return ctx()->master();
    if (masterCombo->currentIndex() < 0) return nullptr;
    return findMasterById(ctx()->shop(), masterCombo->currentData().toInt());
}

std::shared_ptr<Service> MasterSettingsPage::selectedService() const {
    int row = offers->currentRow();
    if (row < 0 || offers->item(row, 0) == nullptr) return nullptr;
    return findServiceById(ctx()->shop(), offers->item(row, 0)->data(Qt::UserRole).toInt());
}

void MasterSettingsPage::refresh() {
    fillMasters();
    fillOffers();
    fillSchedule();
}

void MasterSettingsPage::fillMasters() {
    loading = true;
    int keep = masterCombo->currentIndex() >= 0 ? masterCombo->currentData().toInt() : -1;
    masterCombo->clear();
    for (const auto& master : shop().getMasters()) {
        masterCombo->addItem(qs(master->getName()) + "  ·  " + levelText(master->classify()), master->getId());
    }
    for (int i = 0; i < masterCombo->count(); ++i) {
        if (masterCombo->itemData(i).toInt() == keep) masterCombo->setCurrentIndex(i);
    }
    loading = false;
}

void MasterSettingsPage::onMasterChanged() {
    if (loading) return;
    fillOffers();
    fillSchedule();
}

void MasterSettingsPage::fillOffers() {
    offers->setRowCount(0);
    std::shared_ptr<Master> master = currentMaster();
    if (!master) {
        masterInfo->setText(QString());
        onSelectionChanged();
        return;
    }
    masterInfo->setText(QString::fromUtf8("%1  ·  коэффициент уровня ×%2  ·  рейтинг ★ %3")
        .arg(qs(master->getName())).arg(master->getPriceFactor(), 0, 'f', 2).arg(master->getRating(), 0, 'f', 1));
    for (const auto& offer : master->getOffers()) {
        std::shared_ptr<Service> service = offer->getService();
        int row = offers->rowCount();
        offers->insertRow(row);
        QTableWidgetItem* name = makeItem(qs(service->getName()));
        name->setData(Qt::UserRole, service->getId());
        offers->setItem(row, 0, name);
        offers->setItem(row, 1, makeItem(money(service->getPrice())));
        offers->setItem(row, 2, makeItem(money(master->calculatePrice(*service))
            + (offer->getCustomPrice() > 0 ? QString::fromUtf8("  (своя)") : QString::fromUtf8("  (авто)"))));
        offers->setItem(row, 3, makeItem(QString::fromUtf8("%1 мин").arg(master->calculateDuration(*service))));
    }
    onSelectionChanged();
}

void MasterSettingsPage::fillSchedule() {
    std::shared_ptr<Master> master = currentMaster();
    if (!master) return;
    startEdit->setTime(QTime(master->getWorkStart() / 60, master->getWorkStart() % 60));
    endEdit->setTime(QTime(master->getWorkEnd() / 60, master->getWorkEnd() % 60));
    for (int i = 0; i < dayOffCombo->count(); ++i) {
        if (dayOffCombo->itemData(i).toInt() == master->getDayOff()) dayOffCombo->setCurrentIndex(i);
    }
}

void MasterSettingsPage::onSelectionChanged() {
    bool has = selectedService() != nullptr;
    editButton->setEnabled(has);
    removeButton->setEnabled(has);
}

void MasterSettingsPage::onAddOffer() {
    std::shared_ptr<Master> master = currentMaster();
    if (!master) return;
    OfferDialog dialog(ctx(), master, nullptr, this);
    if (dialog.exec() == QDialog::Accepted) {
        emit dataChanged();
        fillOffers();
    }
}

void MasterSettingsPage::onEditOffer() {
    std::shared_ptr<Master> master = currentMaster();
    std::shared_ptr<Service> service = selectedService();
    if (!master || !service) return;
    OfferDialog dialog(ctx(), master, service, this);
    if (dialog.exec() == QDialog::Accepted) {
        emit dataChanged();
        fillOffers();
    }
}

void MasterSettingsPage::onRemoveOffer() {
    std::shared_ptr<Master> master = currentMaster();
    std::shared_ptr<Service> service = selectedService();
    if (!master || !service) return;
    if (!askConfirm(this, QString::fromUtf8("Удаление услуги"),
        QString::fromUtf8("Убрать услугу «%1» у мастера %2?").arg(qs(service->getName()), qs(master->getName())),
        QString::fromUtf8("Убрать"))) {
        return;
    }
    try {
        shop().removeOffer(master, service, ctx()->user());
        emit dataChanged();
        fillOffers();
    }
    catch (...) {
        showError(this, shop());
    }
}

void MasterSettingsPage::onSaveSchedule() {
    std::shared_ptr<Master> master = currentMaster();
    if (!master) return;
    try {
        int start = startEdit->time().hour() * 60 + startEdit->time().minute();
        int end = endEdit->time().hour() * 60 + endEdit->time().minute();
        if (!askConfirm(this, QString::fromUtf8("Изменение графика"),
            QString::fromUtf8("Вы точно хотите изменить график мастера «%1»: %2–%3, выходной: %4?")
                .arg(qs(master->getName()), formatClock(start), formatClock(end),
                    dayOffText(dayOffCombo->currentData().toInt())),
            QString::fromUtf8("Да, сохранить"))) {
            fillSchedule();
            return;
        }
        shop().changeSchedule(master, start, end, dayOffCombo->currentData().toInt(), ctx()->user());
        showInfo(this, QString::fromUtf8("График сохранён"),
            QString::fromUtf8("%1: %2–%3, выходной: %4").arg(qs(master->getName()), formatClock(start),
                formatClock(end), dayOffText(master->getDayOff())));
        emit dataChanged();
        fillSchedule();
    }
    catch (...) {
        showError(this, shop());
        fillSchedule();
    }
}
