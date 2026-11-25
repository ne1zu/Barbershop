#include "ClientsPage.h"
#include <QAction>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QTableWidget>
#include <QVBoxLayout>
#include "BookingPanel.h"
#include "EntityDialogs.h"
#include "UiUtils.h"
#include "config.h"

ClientsPage::ClientsPage(AppContext* contextValue, QWidget* parent)
    : PageBase(contextValue, parent), loading(false) {
    QHBoxLayout* root = new QHBoxLayout(this);
    root->setContentsMargins(28, 20, 28, 24);
    root->setSpacing(18);

    QVBoxLayout* left = new QVBoxLayout;
    left->setSpacing(10);
    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText(QString::fromUtf8("Поиск по имени или телефону"));
    searchEdit->setClearButtonEnabled(true);
    left->addWidget(searchEdit);
    filterCombo = new QComboBox;
    filterCombo->addItem(QString::fromUtf8("Все клиенты"), 0);
    filterCombo->addItem(QString::fromUtf8("Только активные"), 1);
    filterCombo->addItem(QString::fromUtf8("Только заблокированные"), 2);
    left->addWidget(filterCombo);
    countLabel = makeLabel(QString(), "mutedSmall");
    left->addWidget(countLabel);
    list = new QListWidget;
    left->addWidget(list, 1);
    QPushButton* newClient = makeButton(QString::fromUtf8("+ Новый клиент"), "primary");
    left->addWidget(newClient);
    QWidget* leftHost = new QWidget;
    leftHost->setLayout(left);
    leftHost->setFixedWidth(320);
    root->addWidget(leftHost);

    QScrollArea* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    root->addWidget(scroll, 1);
    QWidget* inner = new QWidget;
    scroll->setWidget(inner);
    QVBoxLayout* rightOuter = new QVBoxLayout(inner);
    rightOuter->setContentsMargins(0, 0, 8, 0);

    emptyLabel = makeLabel(QString::fromUtf8("Выберите клиента в списке слева"), "emptyState");
    emptyLabel->setAlignment(Qt::AlignCenter);
    rightOuter->addWidget(emptyLabel, 1);

    detailsHost = new QWidget;
    QVBoxLayout* right = new QVBoxLayout(detailsHost);
    right->setContentsMargins(0, 0, 0, 0);
    right->setSpacing(14);
    rightOuter->addWidget(detailsHost);
    rightOuter->addStretch(1);

    QFrame* head = makeCard();
    QHBoxLayout* headLayout = new QHBoxLayout(head);
    headLayout->setContentsMargins(22, 18, 22, 18);
    headLayout->setSpacing(16);
    avatar = makeLabel(QString(), "avatarBig");
    avatar->setFixedSize(60, 60);
    avatar->setAlignment(Qt::AlignCenter);
    headLayout->addWidget(avatar);
    QVBoxLayout* names = new QVBoxLayout;
    names->setSpacing(4);
    nameLabel = makeLabel(QString(), "bigName");
    phoneLabel = makeLabel(QString(), "muted");
    names->addWidget(nameLabel);
    names->addWidget(phoneLabel);
    headLayout->addLayout(names, 1);
    levelBadge = makeBadge(QString(), "regular");
    bannedBadge = makeBadge(QString::fromUtf8("Заблокирован"), "banned");
    headLayout->addWidget(levelBadge);
    headLayout->addWidget(bannedBadge);
    QPushButton* book = makeButton(QString::fromUtf8("Новая запись"), "primary");
    actionsButton = makeButton(QString::fromUtf8("Действия  ▾"), "ghost");
    actionsButton->setMinimumWidth(130);
    QMenu* menu = new QMenu(actionsButton);
    banAction = menu->addAction(QString::fromUtf8("Заблокировать клиента…"));
    unbanAction = menu->addAction(QString::fromUtf8("Разблокировать клиента"));
    visitsAction = menu->addAction(QString::fromUtf8("Изменить число визитов…"));
    unbanAction->setVisible(false);
    visitsAction->setVisible(ctx()->isAdmin());
    actionsButton->setMenu(menu);
    headLayout->addWidget(actionsButton);
    headLayout->addWidget(book);
    right->addWidget(head);

    QHBoxLayout* stats = new QHBoxLayout;
    stats->setSpacing(12);
    stats->addWidget(makeStatCard(QString::fromUtf8("Визитов"), "0", QString(), false, &visitsValue));
    stats->addWidget(makeStatCard(QString::fromUtf8("Потрачено"), "0", QString(), false, &spentValue));
    stats->addWidget(makeStatCard(QString::fromUtf8("Неявок"), "0", QString(), false, &noShowValue));
    stats->addWidget(makeStatCard(QString::fromUtf8("Отмен"), "0", QString(), false, &cancelValue));
    stats->addWidget(makeStatCard(QString::fromUtf8("Скидка"), "0%", QString(), true, &discountValue));
    right->addLayout(stats);

    banInfo = makeLabel(QString(), "banLine");
    banInfo->setWordWrap(true);
    banInfo->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    right->addWidget(banInfo);

    QHBoxLayout* middle = new QHBoxLayout;
    middle->setSpacing(12);

    QFrame* notesCard = makeCard();
    QVBoxLayout* notesBox = new QVBoxLayout(notesCard);
    notesBox->setContentsMargins(18, 14, 18, 14);
    notesBox->setSpacing(8);
    notesTitle = makeLabel(QString::fromUtf8("Приватные заметки"), "sectionTitle");
    notesBox->addWidget(notesTitle);
    notesBox->addWidget(makeLabel(QString::fromUtf8("Видны только персоналу, клиент их не видит."), "mutedSmall"));
    notesList = new QListWidget;
    notesList->setObjectName("compactList");
    notesList->setFixedHeight(104);
    notesList->setSelectionMode(QAbstractItemView::NoSelection);
    notesList->setWordWrap(true);
    notesList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    notesList->setTextElideMode(Qt::ElideNone);
    notesBox->addWidget(notesList);
    QHBoxLayout* noteRow = new QHBoxLayout;
    noteRow->setSpacing(8);
    noteEdit = new QLineEdit;
    noteEdit->setPlaceholderText(QString::fromUtf8("Новая заметка о клиенте"));
    QPushButton* addNote = makeButton(QString::fromUtf8("Добавить"), "primary");
    noteRow->addWidget(noteEdit, 1);
    noteRow->addWidget(addNote);
    notesBox->addLayout(noteRow);
    middle->addWidget(notesCard, 1);

    QFrame* loyaltyCard = makeCard();
    loyaltyLayout = new QVBoxLayout(loyaltyCard);
    loyaltyLayout->setContentsMargins(18, 14, 18, 14);
    loyaltyLayout->setSpacing(6);
    middle->addWidget(loyaltyCard, 1);
    right->addLayout(middle);

    right->addWidget(makeLabel(QString::fromUtf8("История визитов"), "sectionTitle"));
    history = makeTable({ QString::fromUtf8("Дата"), QString::fromUtf8("Время"), QString::fromUtf8("Мастер"),
        QString::fromUtf8("Услуга"), QString::fromUtf8("Цена"), QString::fromUtf8("Статус") });
    history->setSelectionMode(QAbstractItemView::NoSelection);
    fixColumn(history, 1, 130);
    fixColumn(history, 5, 150);
    right->addWidget(history);

    connect(searchEdit, &QLineEdit::textChanged, this, &ClientsPage::onSearchChanged);
    connect(list, &QListWidget::currentRowChanged, this, &ClientsPage::onSelectionChanged);
    connect(newClient, &QPushButton::clicked, this, &ClientsPage::onNewClient);
    connect(addNote, &QPushButton::clicked, this, &ClientsPage::onAddNote);
    connect(noteEdit, &QLineEdit::returnPressed, this, &ClientsPage::onAddNote);
    connect(filterCombo, &QComboBox::currentIndexChanged, this, &ClientsPage::onFilterChanged);
    connect(banAction, &QAction::triggered, this, &ClientsPage::onBan);
    connect(unbanAction, &QAction::triggered, this, &ClientsPage::onUnban);
    connect(visitsAction, &QAction::triggered, this, &ClientsPage::onEditVisits);
    connect(book, &QPushButton::clicked, this, &ClientsPage::onBook);
    showDetails();
}

QString ClientsPage::title() const { return QString::fromUtf8("Клиенты"); }

std::shared_ptr<Client> ClientsPage::selected() const {
    QListWidgetItem* item = list->currentItem();
    if (item == nullptr) return nullptr;
    return findClientById(ctx()->shop(), item->data(Qt::UserRole).toInt());
}

void ClientsPage::refresh() {
    fillList();
    showDetails();
}

void ClientsPage::fillList() {
    loading = true;
    int keep = -1;
    std::shared_ptr<Client> previous = selected();
    if (previous) keep = previous->getId();
    list->clear();
    QString filter = searchEdit->text().trimmed();
    int restore = -1;
    int mode = filterCombo->currentData().toInt();
    int total = shop().getClients().getCount();
    for (const auto& client : shop().getClients()) {
        if (mode == 1 && client->isBanned()) continue;
        if (mode == 2 && !client->isBanned()) continue;
        QString name = qs(client->getName());
        QString phone = qs(client->getPhone());
        if (!filter.isEmpty() && !name.contains(filter, Qt::CaseInsensitive) && !phone.contains(filter)) continue;
        QString text = name + "\n" + phone;
        if (client->isBanned()) text += QString::fromUtf8("  ·  заблокирован");
        QListWidgetItem* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, client->getId());
        list->addItem(item);
        if (client->getId() == keep) restore = list->count() - 1;
    }
    if (restore >= 0) list->setCurrentRow(restore);
    countLabel->setText(QString::fromUtf8("Показано %1 из %2").arg(list->count()).arg(total));
    loading = false;
}

void ClientsPage::onSearchChanged() {
    fillList();
    showDetails();
}

void ClientsPage::onFilterChanged() {
    fillList();
    showDetails();
}

void ClientsPage::onSelectionChanged() {
    if (!loading) showDetails();
}

void ClientsPage::showDetails() {
    std::shared_ptr<Client> client = selected();
    detailsHost->setVisible(client != nullptr);
    emptyLabel->setVisible(client == nullptr);
    if (!client) return;

    avatar->setText(initials(client->getName()));
    nameLabel->setText(qs(client->getName()));
    phoneLabel->setText(qs(client->getPhone()));
    int visits = client->getVisitsCount();
    setBadge(levelBadge, levelText(client->classify()), levelKind(client->classify()));
    bannedBadge->setVisible(false);
    visitsValue->setText(QString::number(visits));
    spentValue->setText(money(client->getTotalSpent()));
    noShowValue->setText(QString::number(client->getNoShowCount()));
    cancelValue->setText(QString::number(client->getCancelCount()));
    discountValue->setText(QString("%1%").arg(client->getDiscountPercent()));

    banInfo->setVisible(client->isBanned());
    if (client->isBanned()) banInfo->setText(QString::fromUtf8("Заблокирован. Причина: ") + qs(client->getBanReason()));
    banAction->setVisible(!client->isBanned());
    unbanAction->setVisible(client->isBanned());
    unbanAction->setEnabled(ctx()->isAdmin());
    unbanAction->setToolTip(QString::fromUtf8("Разблокировать клиента может только администратор"));

    notesList->clear();
    const std::vector<std::string>& notes = client->getNotes();
    notesTitle->setText(QString::fromUtf8("Приватные заметки (%1)").arg(notes.size()));
    if (notes.empty()) {
        QListWidgetItem* empty = new QListWidgetItem(QString::fromUtf8("Заметок пока нет"));
        empty->setFlags(Qt::NoItemFlags);
        notesList->addItem(empty);
    }
    for (int i = static_cast<int>(notes.size()) - 1; i >= 0; --i) notesList->addItem(qs(notes[i]));
    fillLoyalty(*client);

    history->setRowCount(0);
    std::vector<std::shared_ptr<Appointment>> all = sortedAppointments(shop());
    for (int i = static_cast<int>(all.size()) - 1; i >= 0; --i) {
        const std::shared_ptr<Appointment>& appointment = all[i];
        if (appointment->getClient().get() != client.get()) continue;
        int row = history->rowCount();
        history->insertRow(row);
        history->setItem(row, 0, makeItem(formatDay(appointment->getStart())));
        history->setItem(row, 1, makeItem(timeRangeText(*appointment)));
        history->setItem(row, 2, makeItem(qs(appointment->getMaster()->getName())));
        history->setItem(row, 3, makeItem(qs(appointment->getService()->getName())));
        history->setItem(row, 4, makeItem(money(appointment->getPrice())));
        history->setCellWidget(row, 5, makeBadgeCell(statusText(appointment->getStatus()),
            statusKind(appointment->getStatus())));
    }
    fitTableHeight(history);
}

void ClientsPage::fillLoyalty(const Client& client) {
    clearLayout(loyaltyLayout);
    loyaltyLayout->addWidget(makeLabel(QString::fromUtf8("Программа лояльности"), "sectionTitle"));
    int visits = client.getVisitsCount();

    struct Step { const char* title; int from; int discount; };
    const Step steps[] = {
        { "Новый и постоянный клиент", 0, 0 },
        { "Бронза", BRONZE_VISITS, BRONZE_DISCOUNT },
        { "Серебро", SILVER_VISITS, SILVER_DISCOUNT },
        { "Золото", GOLD_VISITS, GOLD_DISCOUNT }
    };
    int current = 0;
    for (int i = 0; i < 4; ++i) {
        if (visits >= steps[i].from) current = i;
    }
    for (int i = 0; i < 4; ++i) {
        QString line = QString::fromUtf8("%1  ·  от %2 визитов  ·  скидка %3%")
            .arg(QString::fromUtf8(steps[i].title)).arg(steps[i].from).arg(steps[i].discount);
        if (i == 0) line = QString::fromUtf8("Начальный уровень  ·  скидки нет");
        QLabel* row = makeLabel((i == current ? QString::fromUtf8("●  ") : QString::fromUtf8("○  ")) + line,
            i == current ? "ladderActive" : "ladderRow");
        loyaltyLayout->addWidget(row);
    }
    if (current < 3) {
        int need = steps[current + 1].from;
        loyaltyLayout->addWidget(makeBarRow(QString::fromUtf8("До уровня «%1»").arg(QString::fromUtf8(steps[current + 1].title)),
            visits, need, QString::fromUtf8("%1 из %2").arg(visits).arg(need)));
    }
    else {
        loyaltyLayout->addWidget(makeLabel(QString::fromUtf8("Достигнут максимальный уровень."), "mutedSmall"));
    }
    double example = 30.0;
    QLabel* rule = makeLabel(QString::fromUtf8(
        "Скидка зависит только от числа оплаченных визитов. К оплате = цена × (100 − скидка) / 100. "
        "Пример: услуга %1 при скидке %2% стоит %3.")
        .arg(money(example)).arg(client.getDiscountPercent()).arg(money(example * client.getPriceFactor())), "mutedSmall");
    rule->setWordWrap(true);
    loyaltyLayout->addWidget(rule);
}

void ClientsPage::onNewClient() {
    ClientDialog dialog(shop(), this);
    if (dialog.exec() != QDialog::Accepted) return;
    searchEdit->clear();
    fillList();
    std::shared_ptr<Client> created = dialog.result();
    for (int i = 0; created && i < list->count(); ++i) {
        if (list->item(i)->data(Qt::UserRole).toInt() == created->getId()) list->setCurrentRow(i);
    }
    emit dataChanged();
    showDetails();
}

void ClientsPage::onAddNote() {
    std::shared_ptr<Client> client = selected();
    if (!client) return;
    try {
        shop().addClientNote(client, ss(noteEdit->text().trimmed()), ctx()->user());
        noteEdit->clear();
        emit dataChanged();
        showDetails();
    }
    catch (...) {
        showError(this, shop());
    }
}

void ClientsPage::onBan() {
    std::shared_ptr<Client> client = selected();
    if (!client) return;
    bool ok = false;
    QString reason = QInputDialog::getText(this, QString::fromUtf8("Блокировка клиента"),
        QString::fromUtf8("Причина блокировки (её увидит только персонал):"), QLineEdit::Normal, QString(), &ok);
    if (!ok) return;
    if (!askConfirm(this, QString::fromUtf8("Блокировка клиента"),
        QString::fromUtf8("Вы точно хотите заблокировать клиента «%1»? Он не сможет записываться.")
            .arg(qs(client->getName())), QString::fromUtf8("Да, заблокировать"))) {
        return;
    }
    try {
        shop().banClient(client, ss(reason.trimmed()), ctx()->user());
        emit dataChanged();
        fillList();
        showDetails();
    }
    catch (...) {
        showError(this, shop());
    }
}

void ClientsPage::onUnban() {
    std::shared_ptr<Client> client = selected();
    if (!client) return;
    if (!askConfirm(this, QString::fromUtf8("Разблокировка клиента"),
        QString::fromUtf8("Вы точно хотите разблокировать клиента «%1»?").arg(qs(client->getName())),
        QString::fromUtf8("Да, разблокировать"))) {
        return;
    }
    try {
        shop().unbanClient(client, ctx()->user());
        emit dataChanged();
        fillList();
        showDetails();
    }
    catch (...) {
        showError(this, shop());
    }
}

void ClientsPage::onEditVisits() {
    std::shared_ptr<Client> client = selected();
    if (!client || !ctx()->isAdmin()) return;
    bool ok = false;
    int visits = QInputDialog::getInt(this, QString::fromUtf8("Стаж клиента"),
        QString::fromUtf8("Число визитов (определяет уровень и скидку клиента):"), client->getVisitsCount(), 0, 1000, 1, &ok);
    if (!ok) return;
    if (!askConfirm(this, QString::fromUtf8("Изменение числа визитов"),
        QString::fromUtf8("Вы точно хотите изменить число визитов клиента «%1» с %2 на %3? Это изменит его скидку.")
            .arg(qs(client->getName())).arg(client->getVisitsCount()).arg(visits),
        QString::fromUtf8("Да, изменить"))) {
        return;
    }
    try {
        client->setVisitsCount(visits);
        shop().log("EDIT", "Client '" + client->getName() + "': visits set to " + std::to_string(visits));
        emit dataChanged();
        showDetails();
    }
    catch (...) {
        showError(this, shop());
    }
}

void ClientsPage::onBook() {
    std::shared_ptr<Client> client = selected();
    if (!client) return;
    NewBookingDialog dialog(ctx(), this);
    dialog.presetClient(client);
    if (dialog.exec() == QDialog::Accepted) {
        emit dataChanged();
        showDetails();
    }
}
