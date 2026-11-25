#include "AdminPages.h"
#include <QDir>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include "AppointmentActions.h"
#include "EntityDialogs.h"
#include "UiUtils.h"
#include "config.h"
#include "data_storage.h"
#include "exceptions/exceptions.h"
#include "report_writer.h"
#include "statistics.h"

StaffPage::StaffPage(AppContext* contextValue, QWidget* parent) : PageBase(contextValue, parent) {
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 20, 28, 24);
    root->setSpacing(14);

    QHBoxLayout* buttons = new QHBoxLayout;
    buttons->setSpacing(10);
    QPushButton* addMaster = makeButton(QString::fromUtf8("+ Мастер"), "primary");
    QPushButton* addAdmin = makeButton(QString::fromUtf8("+ Администратор"), "ghost");
    removeButton = makeButton(QString::fromUtf8("Удалить сотрудника"), "danger");
    editButton = makeButton(QString::fromUtf8("Стаж и рейтинг мастера"), "ghost");
    buttons->addWidget(addMaster);
    buttons->addWidget(addAdmin);
    buttons->addWidget(editButton);
    buttons->addStretch(1);
    buttons->addWidget(removeButton);
    root->addLayout(buttons);

    table = makeTable({ QString::fromUtf8("Имя"), QString::fromUtf8("Телефон"), QString::fromUtf8("Роль"),
        QString::fromUtf8("Логин"), QString::fromUtf8("Уровень / должность"), QString::fromUtf8("Опыт"),
        QString::fromUtf8("Рейтинг") });
    root->addWidget(table, 1);

    connect(table, &QTableWidget::itemSelectionChanged, this, &StaffPage::onSelectionChanged);
    connect(addMaster, &QPushButton::clicked, this, &StaffPage::onAddMaster);
    connect(addAdmin, &QPushButton::clicked, this, &StaffPage::onAddAdmin);
    connect(removeButton, &QPushButton::clicked, this, &StaffPage::onRemove);
    connect(editButton, &QPushButton::clicked, this, &StaffPage::onEdit);
    connect(table, &QTableWidget::cellDoubleClicked, this, &StaffPage::onEdit);
}

QString StaffPage::title() const { return QString::fromUtf8("Персонал"); }

void StaffPage::refresh() {
    table->setRowCount(0);
    for (const auto& master : shop().getMasters()) {
        int row = table->rowCount();
        table->insertRow(row);
        QTableWidgetItem* name = makeItem(qs(master->getName()));
        name->setData(Qt::UserRole, master->getId());
        name->setData(Qt::UserRole + 1, 0);
        table->setItem(row, 0, name);
        table->setItem(row, 1, makeItem(qs(master->getPhone())));
        table->setCellWidget(row, 2, makeBadgeCell(QString::fromUtf8("Мастер"), "role"));
        table->setItem(row, 3, makeItem(qs(master->getLogin())));
        table->setCellWidget(row, 4, makeBadgeCell(levelText(master->classify()), levelKind(master->classify())));
        table->setItem(row, 5, makeItem(QString::fromUtf8("%1 лет").arg(master->getExperience())));
        table->setItem(row, 6, makeItem(QString::fromUtf8("★ ") + QString::number(master->getRating(), 'f', 1)));
    }
    for (const auto& admin : shop().getAdministrators()) {
        int row = table->rowCount();
        table->insertRow(row);
        QTableWidgetItem* name = makeItem(qs(admin->getName()));
        name->setData(Qt::UserRole, admin->getId());
        name->setData(Qt::UserRole + 1, 1);
        table->setItem(row, 0, name);
        table->setItem(row, 1, makeItem(qs(admin->getPhone())));
        table->setCellWidget(row, 2, makeBadgeCell(QString::fromUtf8("Администратор"), "role"));
        table->setItem(row, 3, makeItem(qs(admin->getLogin())));
        table->setItem(row, 4, makeItem(qs(admin->getPosition())));
        table->setItem(row, 5, makeItem("—"));
        table->setItem(row, 6, makeItem("—"));
    }
    onSelectionChanged();
}

void StaffPage::onSelectionChanged() {
    int row = table->currentRow();
    removeButton->setEnabled(row >= 0);
    editButton->setEnabled(row >= 0 && table->item(row, 0)->data(Qt::UserRole + 1).toInt() == 0);
}

void StaffPage::onEdit() {
    int row = table->currentRow();
    if (row < 0 || table->item(row, 0)->data(Qt::UserRole + 1).toInt() != 0) return;
    int id = table->item(row, 0)->data(Qt::UserRole).toInt();
    std::shared_ptr<Master> master = findMasterById(shop(), id);
    if (!master) return;
    MasterProfileDialog dialog(shop(), master, this);
    if (dialog.exec() == QDialog::Accepted) {
        emit dataChanged();
        refresh();
        for (int i = 0; i < table->rowCount(); ++i) {
            if (table->item(i, 0)->data(Qt::UserRole).toInt() == id
                && table->item(i, 0)->data(Qt::UserRole + 1).toInt() == 0) table->selectRow(i);
        }
    }
}

void StaffPage::onAddMaster() {
    MasterDialog dialog(shop(), this);
    if (dialog.exec() == QDialog::Accepted) {
        showInfo(this, QString::fromUtf8("Мастер добавлен"),
            QString::fromUtf8("Теперь назначьте ему услуги и график на странице «Мастера и график»."));
        emit dataChanged();
        refresh();
    }
}

void StaffPage::onAddAdmin() {
    AdminDialog dialog(shop(), this);
    if (dialog.exec() == QDialog::Accepted) {
        emit dataChanged();
        refresh();
    }
}

void StaffPage::onRemove() {
    int row = table->currentRow();
    if (row < 0) return;
    int id = table->item(row, 0)->data(Qt::UserRole).toInt();
    int kind = table->item(row, 0)->data(Qt::UserRole + 1).toInt();
    QString name = table->item(row, 0)->text();
    if (!askConfirm(this, QString::fromUtf8("Удаление сотрудника"),
        QString::fromUtf8("Удалить сотрудника «%1»?").arg(name), QString::fromUtf8("Удалить"))) {
        return;
    }
    try {
        if (kind == 0) {
            shop() -= findMasterById(shop(), id);
        }
        else {
            std::shared_ptr<Administrator> victim;
            for (const auto& admin : shop().getAdministrators()) {
                if (admin->getId() == id) victim = admin;
            }
            if (victim.get() == ctx()->admin().get()) {
                throw InvalidOperationException("удаление администратора", "нельзя удалить самого себя");
            }
            shop() -= victim;
        }
        emit dataChanged();
        refresh();
    }
    catch (...) {
        showError(this, shop());
    }
}

ServicesCatalogPage::ServicesCatalogPage(AppContext* contextValue, QWidget* parent) : PageBase(contextValue, parent) {
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 20, 28, 24);
    root->setSpacing(14);
    QHBoxLayout* buttons = new QHBoxLayout;
    QPushButton* add = makeButton(QString::fromUtf8("+ Новая услуга"), "primary");
    removeButton = makeButton(QString::fromUtf8("Удалить услугу"), "danger");
    editButton = makeButton(QString::fromUtf8("Изменить услугу"), "ghost");
    buttons->addWidget(add);
    buttons->addWidget(editButton);
    buttons->addStretch(1);
    buttons->addWidget(removeButton);
    root->addLayout(buttons);
    table = makeTable({ "№", QString::fromUtf8("Название"), QString::fromUtf8("Базовая цена"),
        QString::fromUtf8("Длительность"), QString::fromUtf8("Выполняют мастера") });
    table->setColumnWidth(0, 60);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    root->addWidget(table, 1);
    connect(table, &QTableWidget::itemSelectionChanged, this, &ServicesCatalogPage::onSelectionChanged);
    connect(add, &QPushButton::clicked, this, &ServicesCatalogPage::onAdd);
    connect(removeButton, &QPushButton::clicked, this, &ServicesCatalogPage::onRemove);
    connect(editButton, &QPushButton::clicked, this, &ServicesCatalogPage::onEdit);
    connect(table, &QTableWidget::cellDoubleClicked, this, &ServicesCatalogPage::onEdit);
}

QString ServicesCatalogPage::title() const { return QString::fromUtf8("Каталог услуг"); }

void ServicesCatalogPage::refresh() {
    table->setRowCount(0);
    for (const auto& service : shop().getServices()) {
        QStringList masters;
        for (const auto& master : shop().getMasters()) {
            if (master->canPerform(*service)) masters << qs(master->getName());
        }
        int row = table->rowCount();
        table->insertRow(row);
        QTableWidgetItem* number = makeItem(QString::number(service->getId()));
        number->setData(Qt::UserRole, service->getId());
        table->setItem(row, 0, number);
        table->setItem(row, 1, makeItem(qs(service->getName())));
        table->setItem(row, 2, makeItem(money(service->getPrice())));
        table->setItem(row, 3, makeItem(QString::fromUtf8("%1 мин").arg(service->getLength())));
        table->setItem(row, 4, makeItem(masters.isEmpty() ? QString::fromUtf8("никто") : masters.join(", ")));
    }
    onSelectionChanged();
}

void ServicesCatalogPage::onSelectionChanged() {
    removeButton->setEnabled(table->currentRow() >= 0);
    editButton->setEnabled(table->currentRow() >= 0);
}

void ServicesCatalogPage::onEdit() {
    int row = table->currentRow();
    if (row < 0) return;
    int id = table->item(row, 0)->data(Qt::UserRole).toInt();
    std::shared_ptr<Service> service = findServiceById(shop(), id);
    if (!service) return;
    ServiceDialog dialog(shop(), service, this);
    if (dialog.exec() == QDialog::Accepted) {
        emit dataChanged();
        refresh();
        for (int i = 0; i < table->rowCount(); ++i) {
            if (table->item(i, 0)->data(Qt::UserRole).toInt() == id) table->selectRow(i);
        }
    }
}

void ServicesCatalogPage::onAdd() {
    ServiceDialog dialog(shop(), this);
    if (dialog.exec() == QDialog::Accepted) {
        emit dataChanged();
        refresh();
    }
}

void ServicesCatalogPage::onRemove() {
    int row = table->currentRow();
    if (row < 0) return;
    std::shared_ptr<Service> service = findServiceById(shop(), table->item(row, 0)->data(Qt::UserRole).toInt());
    if (!service) return;
    if (!askConfirm(this, QString::fromUtf8("Удаление услуги"),
        QString::fromUtf8("Удалить услугу «%1» из каталога?").arg(qs(service->getName())),
        QString::fromUtf8("Удалить"))) {
        return;
    }
    try {
        shop() -= service;
        emit dataChanged();
        refresh();
    }
    catch (...) {
        showError(this, shop());
    }
}

PaymentsPage::PaymentsPage(AppContext* contextValue, QWidget* parent) : PageBase(contextValue, parent) {
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 20, 28, 24);
    root->setSpacing(14);

    QHBoxLayout* stats = new QHBoxLayout;
    stats->setSpacing(14);
    stats->addWidget(makeStatCard(QString::fromUtf8("Выручка"), "0", QString(), true, &revenueValue));
    stats->addWidget(makeStatCard(QString::fromUtf8("Платежей"), "0", QString(), false, &countValue));
    stats->addWidget(makeStatCard(QString::fromUtf8("Ждут оплаты"), "0", QString(), false, &pendingValue));
    root->addLayout(stats);

    QHBoxLayout* head = new QHBoxLayout;
    head->addWidget(makeLabel(QString::fromUtf8("Ожидают оплаты (клиент пришёл)"), "sectionTitle"));
    head->addStretch(1);
    payButton = makeButton(QString::fromUtf8("Принять оплату"), "primary");
    head->addWidget(payButton);
    root->addLayout(head);
    unpaidTable = makeTable({ "№", QString::fromUtf8("Дата"), QString::fromUtf8("Клиент"),
        QString::fromUtf8("Услуга"), QString::fromUtf8("Мастер"), QString::fromUtf8("Цена"),
        QString::fromUtf8("Скидка") });
    unpaidTable->setMinimumHeight(170);
    root->addWidget(unpaidTable, 1);

    root->addWidget(makeLabel(QString::fromUtf8("История платежей"), "sectionTitle"));
    paymentsTable = makeTable({ "№", QString::fromUtf8("Дата и время"), QString::fromUtf8("Клиент"),
        QString::fromUtf8("Услуга"), QString::fromUtf8("Сумма"), QString::fromUtf8("Скидка"),
        QString::fromUtf8("Способ") });
    paymentsTable->setSelectionMode(QAbstractItemView::NoSelection);
    paymentsTable->setMinimumHeight(200);
    root->addWidget(paymentsTable, 1);

    connect(unpaidTable, &QTableWidget::itemSelectionChanged, this, &PaymentsPage::onSelectionChanged);
    connect(payButton, &QPushButton::clicked, this, &PaymentsPage::onPay);
}

QString PaymentsPage::title() const { return QString::fromUtf8("Платежи"); }

std::shared_ptr<Appointment> PaymentsPage::selectedUnpaid() const {
    int row = unpaidTable->currentRow();
    if (row < 0 || unpaidTable->item(row, 0) == nullptr) return nullptr;
    return findAppointmentById(ctx()->shop(), unpaidTable->item(row, 0)->data(Qt::UserRole).toInt());
}

void PaymentsPage::refresh() {
    ShopStatistics stats = calculateStatistics(shop());
    revenueValue->setText(money(stats.revenue));
    countValue->setText(QString::number(stats.paymentsCount));

    unpaidTable->setRowCount(0);
    for (const auto& appointment : sortedAppointments(shop())) {
        if (appointment->getStatus() != AppointmentStatus::Arrived || appointment->isPaid()) continue;
        int row = unpaidTable->rowCount();
        unpaidTable->insertRow(row);
        QTableWidgetItem* number = makeItem(QString::number(appointment->getId()));
        number->setData(Qt::UserRole, appointment->getId());
        unpaidTable->setItem(row, 0, number);
        unpaidTable->setItem(row, 1, makeItem(formatDayTime(appointment->getStart())));
        unpaidTable->setItem(row, 2, makeItem(qs(appointment->getClient()->getName())));
        unpaidTable->setItem(row, 3, makeItem(qs(appointment->getService()->getName())));
        unpaidTable->setItem(row, 4, makeItem(qs(appointment->getMaster()->getName())));
        unpaidTable->setItem(row, 5, makeItem(money(appointment->getPrice())));
        unpaidTable->setItem(row, 6, makeItem(QString("%1%").arg(appointment->getClient()->getDiscountPercent())));
    }
    pendingValue->setText(QString::number(unpaidTable->rowCount()));

    paymentsTable->setRowCount(0);
    for (const auto& payment : shop().getPayments()) {
        std::shared_ptr<Appointment> appointment = payment->getAppointment();
        int row = paymentsTable->rowCount();
        paymentsTable->insertRow(row);
        paymentsTable->setItem(row, 0, makeItem(QString::number(payment->getId())));
        paymentsTable->setItem(row, 1, makeItem(formatDayTime(payment->getPaidAt())));
        paymentsTable->setItem(row, 2, makeItem(qs(appointment->getClient()->getName())));
        paymentsTable->setItem(row, 3, makeItem(qs(appointment->getService()->getName())));
        paymentsTable->setItem(row, 4, makeItem(money(payment->getAmount())));
        paymentsTable->setItem(row, 5, makeItem(QString("%1%").arg(payment->getDiscountPercent())));
        paymentsTable->setItem(row, 6, makeItem(paymentMethodText(payment->getMethod())));
    }
    onSelectionChanged();
}

void PaymentsPage::onSelectionChanged() {
    payButton->setEnabled(selectedUnpaid() != nullptr);
}

void PaymentsPage::onPay() {
    std::shared_ptr<Appointment> appointment = selectedUnpaid();
    if (appointment && actions::pay(ctx(), this, appointment)) {
        emit dataChanged();
        refresh();
    }
}

DataPage::DataPage(AppContext* contextValue, QWidget* parent) : PageBase(contextValue, parent) {
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 20, 28, 24);
    root->setSpacing(16);

    QString folder = QDir::toNativeSeparators(QDir::currentPath());
    QGridLayout* grid = new QGridLayout;
    grid->setSpacing(16);

    QFrame* saveCard = makeCard();
    QVBoxLayout* saveBox = new QVBoxLayout(saveCard);
    saveBox->setContentsMargins(22, 20, 22, 20);
    saveBox->setSpacing(10);
    saveBox->addWidget(makeLabel(QString::fromUtf8("Сохранить состояние"), "sectionTitle"));
    QLabel* saveText = makeLabel(QString::fromUtf8("Клиенты, мастера, услуги, записи и платежи записываются в файл %1. "
        "Программа также сохраняет состояние автоматически при выходе.").arg(STATE_FILE_NAME), "muted");
    saveText->setWordWrap(true);
    saveBox->addWidget(saveText);
    saveBox->addStretch(1);
    QPushButton* saveButton = makeButton(QString::fromUtf8("Сохранить сейчас"), "primary");
    saveBox->addWidget(saveButton);

    QFrame* loadCard = makeCard();
    QVBoxLayout* loadBox = new QVBoxLayout(loadCard);
    loadBox->setContentsMargins(22, 20, 22, 20);
    loadBox->setSpacing(10);
    loadBox->addWidget(makeLabel(QString::fromUtf8("Загрузить из файла"), "sectionTitle"));
    QLabel* loadText = makeLabel(QString::fromUtf8("Текущие данные будут заменены содержимым файла %1. "
        "Если файл повреждён, программа сообщит об ошибке и оставит данные без изменений. После загрузки потребуется войти заново.")
        .arg(STATE_FILE_NAME), "muted");
    loadText->setWordWrap(true);
    loadBox->addWidget(loadText);
    loadBox->addStretch(1);
    QPushButton* loadButton = makeButton(QString::fromUtf8("Загрузить"), "ghost");
    loadBox->addWidget(loadButton);

    QFrame* reportCard = makeCard();
    QVBoxLayout* reportBox = new QVBoxLayout(reportCard);
    reportBox->setContentsMargins(22, 20, 22, 20);
    reportBox->setSpacing(10);
    reportBox->addWidget(makeLabel(QString::fromUtf8("Текстовый отчёт"), "sectionTitle"));
    QLabel* reportText = makeLabel(QString::fromUtf8("Сводная аналитика барбершопа записывается в файл %1.")
        .arg(REPORT_FILE_NAME), "muted");
    reportText->setWordWrap(true);
    reportBox->addWidget(reportText);
    reportBox->addStretch(1);
    QPushButton* reportButton = makeButton(QString::fromUtf8("Сформировать отчёт"), "ghost");
    reportBox->addWidget(reportButton);

    grid->addWidget(saveCard, 0, 0);
    grid->addWidget(loadCard, 0, 1);
    grid->addWidget(reportCard, 0, 2);
    root->addLayout(grid);
    QLabel* where = makeLabel(QString::fromUtf8("Папка с файлами: ") + folder, "mutedSmall");
    where->setWordWrap(true);
    root->addWidget(where);
    root->addStretch(1);

    connect(saveButton, &QPushButton::clicked, this, &DataPage::onSave);
    connect(loadButton, &QPushButton::clicked, this, &DataPage::onLoad);
    connect(reportButton, &QPushButton::clicked, this, &DataPage::onReport);
}

QString DataPage::title() const { return QString::fromUtf8("Данные и отчёты"); }
void DataPage::refresh() {}

void DataPage::onSave() {
    if (!askConfirm(this, QString::fromUtf8("Сохранение состояния"),
        QString::fromUtf8("Вы точно хотите сохранить состояние? Файл %1 будет перезаписан.").arg(STATE_FILE_NAME),
        QString::fromUtf8("Да, сохранить"))) {
        return;
    }
    try {
        DataStorage(STATE_FILE_NAME).save(shop());
        shop().log("SAVE", "State saved to " + std::string(STATE_FILE_NAME));
        showInfo(this, QString::fromUtf8("Готово"), QString::fromUtf8("Состояние сохранено в файл %1.").arg(STATE_FILE_NAME));
    }
    catch (...) {
        showError(this, shop());
    }
}

void DataPage::onLoad() {
    if (!askConfirm(this, QString::fromUtf8("Загрузка состояния"),
        QString::fromUtf8("Текущие данные будут заменены данными из файла. Продолжить?"),
        QString::fromUtf8("Загрузить"))) {
        return;
    }
    try {
        Barbershop loaded = DataStorage(STATE_FILE_NAME).load();
        shop().replaceContent(std::move(loaded));
        shop().log("LOAD", "State loaded from " + std::string(STATE_FILE_NAME));
        showInfo(this, QString::fromUtf8("Готово"),
            QString::fromUtf8("Состояние загружено. Выполните вход заново."));
        emit reloginRequired();
    }
    catch (...) {
        showError(this, shop());
    }
}

void DataPage::onReport() {
    try {
        ReportWriter(REPORT_FILE_NAME).writeReport(shop());
        shop().log("REPORT", "Report saved to " + std::string(REPORT_FILE_NAME));
        showInfo(this, QString::fromUtf8("Готово"), QString::fromUtf8("Отчёт записан в файл %1.").arg(REPORT_FILE_NAME));
    }
    catch (...) {
        showError(this, shop());
    }
}
