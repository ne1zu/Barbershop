#include "ClientsPage.h"
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>
#include "BookingPanel.h"
#include "EntityDialogs.h"
#include "UiUtils.h"

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

    banInfo = makeLabel(QString(), "dangerBanner");
    banInfo->setWordWrap(true);
    right->addWidget(banInfo);

    QFrame* notesCard = makeCard();
    QVBoxLayout* notesBox = new QVBoxLayout(notesCard);
    notesBox->setContentsMargins(22, 18, 22, 18);
    notesBox->setSpacing(10);
    notesBox->addWidget(makeLabel(QString::fromUtf8("Приватные заметки"), "sectionTitle"));
    notesBox->addWidget(makeLabel(QString::fromUtf8("Видны только мастерам и администратору. Клиент их не видит."), "mutedSmall"));
    QWidget* notesHost = new QWidget;
    notesLayout = new QVBoxLayout(notesHost);
    notesLayout->setContentsMargins(0, 0, 0, 0);
    notesLayout->setSpacing(6);
    notesBox->addWidget(notesHost);
    QHBoxLayout* noteRow = new QHBoxLayout;
    noteEdit = new QLineEdit;
    noteEdit->setPlaceholderText(QString::fromUtf8("Например: предпочитает короткую стрижку, не любит разговоры"));
    QPushButton* addNote = makeButton(QString::fromUtf8("Добавить"), "primary");
    noteRow->addWidget(noteEdit, 1);
    noteRow->addWidget(addNote);
    notesBox->addLayout(noteRow);
    QHBoxLayout* banRow = new QHBoxLayout;
    banButton = makeButton(QString::fromUtf8("Заблокировать клиента"), "danger");
    unbanButton = makeButton(QString::fromUtf8("Разблокировать"), "success");
    banRow->addWidget(banButton);
    banRow->addWidget(unbanButton);
    banRow->addStretch(1);
    notesBox->addLayout(banRow);
    right->addWidget(notesCard);

    right->addWidget(makeLabel(QString::fromUtf8("История визитов"), "sectionTitle"));
    history = makeTable({ QString::fromUtf8("Дата"), QString::fromUtf8("Время"), QString::fromUtf8("Мастер"),
        QString::fromUtf8("Услуга"), QString::fromUtf8("Цена"), QString::fromUtf8("Статус") });
    history->setSelectionMode(QAbstractItemView::NoSelection);
    right->addWidget(history);

    connect(searchEdit, &QLineEdit::textChanged, this, &ClientsPage::onSearchChanged);
    connect(list, &QListWidget::currentRowChanged, this, &ClientsPage::onSelectionChanged);
    connect(newClient, &QPushButton::clicked, this, &ClientsPage::onNewClient);
    connect(addNote, &QPushButton::clicked, this, &ClientsPage::onAddNote);
    connect(noteEdit, &QLineEdit::returnPressed, this, &ClientsPage::onAddNote);
    connect(banButton, &QPushButton::clicked, this, &ClientsPage::onBan);
    connect(unbanButton, &QPushButton::clicked, this, &ClientsPage::onUnban);
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
    for (const auto& client : shop().getClients()) {
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
    loading = false;
}

void ClientsPage::onSearchChanged() {
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
    QString level = visits == 0 ? "New" : visits >= 20 ? "Gold" : visits >= 10 ? "Silver" : visits >= 5 ? "Bronze" : "Regular";
    setBadge(levelBadge, levelText(ss(level)), levelKind(ss(level)));
    bannedBadge->setVisible(client->isBanned());
    visitsValue->setText(QString::number(visits));
    spentValue->setText(money(client->getTotalSpent()));
    noShowValue->setText(QString::number(client->getNoShowCount()));
    cancelValue->setText(QString::number(client->getCancelCount()));
    discountValue->setText(QString("%1%").arg(client->getDiscountPercent()));

    banInfo->setVisible(client->isBanned());
    if (client->isBanned()) banInfo->setText(QString::fromUtf8("Клиент заблокирован. Причина: ") + qs(client->getBanReason()));
    banButton->setVisible(!client->isBanned());
    unbanButton->setVisible(client->isBanned() && ctx()->isAdmin());

    clearLayout(notesLayout);
    const std::vector<std::string>& notes = client->getNotes();
    if (notes.empty()) notesLayout->addWidget(makeLabel(QString::fromUtf8("Заметок пока нет"), "mutedSmall"));
    for (int i = static_cast<int>(notes.size()) - 1; i >= 0; --i) {
        QLabel* note = makeLabel(qs(notes[i]), "noteLine");
        note->setWordWrap(true);
        notesLayout->addWidget(note);
    }

    history->setRowCount(0);
    std::vector<std::shared_ptr<Appointment>> all = sortedAppointments(shop());
    for (int i = static_cast<int>(all.size()) - 1; i >= 0; --i) {
        const std::shared_ptr<Appointment>& appointment = all[i];
        if (appointment->getClient().get() != client.get()) continue;
        int row = history->rowCount();
        history->insertRow(row);
        history->setItem(row, 0, makeItem(formatDay(appointment->getStart())));
        history->setItem(row, 1, makeItem(qs(appointment->getTimeRange())));
        history->setItem(row, 2, makeItem(qs(appointment->getMaster()->getName())));
        history->setItem(row, 3, makeItem(qs(appointment->getService()->getName())));
        history->setItem(row, 4, makeItem(money(appointment->getPrice())));
        history->setCellWidget(row, 5, makeBadgeCell(statusText(appointment->getStatus()),
            statusKind(appointment->getStatus())));
    }
    fitTableHeight(history);
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
