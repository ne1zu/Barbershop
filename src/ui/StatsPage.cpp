#include "StatsPage.h"
#include <algorithm>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>
#include "UiUtils.h"
#include "collection.h"
#include "statistics.h"

namespace {
    QFrame* sectionCard(const QString& title, QVBoxLayout** boxOut) {
        QFrame* card = makeCard();
        QVBoxLayout* box = new QVBoxLayout(card);
        box->setContentsMargins(22, 18, 22, 18);
        box->setSpacing(8);
        box->addWidget(makeLabel(title, "sectionTitle"));
        *boxOut = box;
        return card;
    }
}

StatsPage::StatsPage(AppContext* contextValue, QWidget* parent) : PageBase(contextValue, parent) {
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(28, 20, 20, 20);
    QScrollArea* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    outer->addWidget(scroll);
    QWidget* inner = new QWidget;
    scroll->setWidget(inner);
    content = new QVBoxLayout(inner);
    content->setContentsMargins(0, 0, 8, 0);
    content->setSpacing(16);
}

QString StatsPage::title() const {
    return ctx()->isAdmin() ? QString::fromUtf8("Аналитика барбершопа") : QString::fromUtf8("Моя статистика");
}

void StatsPage::refresh() {
    clearLayout(content);
    if (ctx()->isAdmin()) buildAdmin();
    else buildMaster();
    content->addStretch(1);
}

void StatsPage::buildMaster() {
    MasterStatistics stats = calculateMasterStatistics(shop(), *ctx()->master());
    QHBoxLayout* first = new QHBoxLayout;
    first->setSpacing(14);
    first->addWidget(makeStatCard(QString::fromUtf8("Выручка"), money(stats.revenue), QString(), true));
    first->addWidget(makeStatCard(QString::fromUtf8("Оплаченных визитов"), QString::number(stats.paymentsCount)));
    first->addWidget(makeStatCard(QString::fromUtf8("Средний чек"), money(stats.averageCheck)));
    first->addWidget(makeStatCard(QString::fromUtf8("Предстоящих"), QString::number(stats.upcoming)));
    content->addLayout(first);
    QHBoxLayout* second = new QHBoxLayout;
    second->setSpacing(14);
    second->addWidget(makeStatCard(QString::fromUtf8("Пришли"), QString::number(stats.arrived)));
    second->addWidget(makeStatCard(QString::fromUtf8("Неявки"), QString::number(stats.noShow)));
    second->addWidget(makeStatCard(QString::fromUtf8("Отмены"), QString::number(stats.cancelled)));
    second->addWidget(makeStatCard(QString::fromUtf8("Уникальных клиентов"), QString::number(stats.uniqueClients)));
    content->addLayout(second);
}

void StatsPage::buildAdmin() {
    ShopStatistics stats = calculateStatistics(shop());

    QHBoxLayout* first = new QHBoxLayout;
    first->setSpacing(14);
    first->addWidget(makeStatCard(QString::fromUtf8("Выручка"), money(stats.revenue), QString(), true));
    first->addWidget(makeStatCard(QString::fromUtf8("Платежей"), QString::number(stats.paymentsCount),
        QString::fromUtf8("от %1 до %2").arg(money(stats.smallestPayment), money(stats.largestPayment))));
    first->addWidget(makeStatCard(QString::fromUtf8("Средний чек"), money(stats.averageCheck)));
    first->addWidget(makeStatCard(QString::fromUtf8("Клиентов"), QString::number(stats.clients),
        QString::fromUtf8("новых %1 · обслужено %2 · заблокировано %3")
        .arg(stats.newClients).arg(stats.servedClients).arg(stats.bannedClients)));
    content->addLayout(first);

    QVBoxLayout* statusBox = nullptr;
    QFrame* statusCard = sectionCard(QString::fromUtf8("Записи по статусам"), &statusBox);
    int top = std::max({ stats.pending, stats.confirmed, stats.arrived, stats.noShow, stats.cancelled, 1 });
    statusBox->addWidget(makeBarRow(statusText(AppointmentStatus::Pending), stats.pending, top, QString::number(stats.pending)));
    statusBox->addWidget(makeBarRow(statusText(AppointmentStatus::Confirmed), stats.confirmed, top, QString::number(stats.confirmed)));
    statusBox->addWidget(makeBarRow(statusText(AppointmentStatus::Arrived), stats.arrived, top, QString::number(stats.arrived)));
    statusBox->addWidget(makeBarRow(statusText(AppointmentStatus::NoShow), stats.noShow, top, QString::number(stats.noShow)));
    statusBox->addWidget(makeBarRow(statusText(AppointmentStatus::Cancelled), stats.cancelled, top, QString::number(stats.cancelled)));

    QVBoxLayout* revenueBox = nullptr;
    QFrame* revenueCard = sectionCard(QString::fromUtf8("Выручка по мастерам"), &revenueBox);
    double maxRevenue = 1.0;
    for (const auto& pair : stats.revenueByMaster) maxRevenue = std::max(maxRevenue, pair.second);
    for (const auto& pair : stats.revenueByMaster) {
        revenueBox->addWidget(makeBarRow(qs(pair.first), pair.second, maxRevenue, money(pair.second)));
    }
    if (stats.revenueByMaster.empty()) revenueBox->addWidget(makeLabel(QString::fromUtf8("Платежей пока нет"), "muted"));

    QHBoxLayout* row1 = new QHBoxLayout;
    row1->setSpacing(14);
    row1->addWidget(statusCard);
    row1->addWidget(revenueCard);
    content->addLayout(row1);

    QVBoxLayout* serviceBox = nullptr;
    QFrame* serviceCard = sectionCard(QString::fromUtf8("Визиты по услугам"), &serviceBox);
    int maxVisits = 1;
    for (const auto& pair : stats.visitsByService) maxVisits = std::max(maxVisits, pair.second);
    for (const auto& pair : stats.visitsByService) {
        serviceBox->addWidget(makeBarRow(qs(pair.first), pair.second, maxVisits, QString::number(pair.second)));
    }
    if (stats.visitsByService.empty()) serviceBox->addWidget(makeLabel(QString::fromUtf8("Визитов пока нет"), "muted"));

    QVBoxLayout* levelBox = nullptr;
    QFrame* levelCard = sectionCard(QString::fromUtf8("Клиенты по уровням"), &levelBox);
    int maxLevel = 1;
    for (const auto& pair : stats.clientsByLevel) maxLevel = std::max(maxLevel, pair.second);
    for (const auto& pair : stats.clientsByLevel) {
        levelBox->addWidget(makeBarRow(levelText(pair.first), pair.second, maxLevel, QString::number(pair.second)));
    }

    QHBoxLayout* row2 = new QHBoxLayout;
    row2->setSpacing(14);
    row2->addWidget(serviceCard);
    row2->addWidget(levelCard);
    content->addLayout(row2);

    QVBoxLayout* highlightBox = nullptr;
    QFrame* highlightCard = sectionCard(QString::fromUtf8("Лидеры"), &highlightBox);
    QString topService = stats.topService.empty() ? QString::fromUtf8("—")
        : qs(stats.topService) + QString::fromUtf8(" (%1 визитов)").arg(stats.topServiceVisits);
    QString topMaster = stats.topMaster.empty() ? QString::fromUtf8("—")
        : qs(stats.topMaster) + " (" + money(stats.topMasterRevenue) + ")";
    highlightBox->addWidget(makeLabel(QString::fromUtf8("Популярная услуга:  ") + topService));
    highlightBox->addWidget(makeLabel(QString::fromUtf8("Мастер с максимальной выручкой:  ") + topMaster));
    std::shared_ptr<Master> bestRated = findMax(shop().getMasters());
    if (bestRated) {
        highlightBox->addWidget(makeLabel(QString::fromUtf8("Лучший рейтинг:  %1 (★ %2)")
            .arg(qs(bestRated->getName())).arg(bestRated->getRating(), 0, 'f', 1)));
    }
    std::shared_ptr<Client> loyal = findMax(shop().getClients());
    if (loyal) {
        highlightBox->addWidget(makeLabel(QString::fromUtf8("Самый частый клиент:  %1 (%2 визитов)")
            .arg(qs(loyal->getName())).arg(loyal->getVisitsCount())));
    }
    content->addWidget(highlightCard);

    QVBoxLayout* mastersBox = nullptr;
    QFrame* mastersCard = sectionCard(QString::fromUtf8("Показатели мастеров"), &mastersBox);
    QTableWidget* table = makeTable({ QString::fromUtf8("Мастер"), QString::fromUtf8("Выручка"),
        QString::fromUtf8("Платежей"), QString::fromUtf8("Ср. чек"), QString::fromUtf8("Пришли"),
        QString::fromUtf8("Неявки"), QString::fromUtf8("Отмены"), QString::fromUtf8("Предстоит"),
        QString::fromUtf8("Клиентов") });
    table->setSelectionMode(QAbstractItemView::NoSelection);
    for (const auto& master : shop().getMasters()) {
        MasterStatistics item = calculateMasterStatistics(shop(), *master);
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, makeItem(qs(master->getName())));
        table->setItem(row, 1, makeItem(money(item.revenue)));
        table->setItem(row, 2, makeItem(QString::number(item.paymentsCount)));
        table->setItem(row, 3, makeItem(money(item.averageCheck)));
        table->setItem(row, 4, makeItem(QString::number(item.arrived)));
        table->setItem(row, 5, makeItem(QString::number(item.noShow)));
        table->setItem(row, 6, makeItem(QString::number(item.cancelled)));
        table->setItem(row, 7, makeItem(QString::number(item.upcoming)));
        table->setItem(row, 8, makeItem(QString::number(item.uniqueClients)));
    }
    fitTableHeight(table);
    mastersBox->addWidget(table);
    content->addWidget(mastersCard);

    QVBoxLayout* eventsBox = nullptr;
    QFrame* eventsCard = sectionCard(QString::fromUtf8("Последние события"), &eventsBox);
    const std::deque<std::string>& events = shop().getRecentEvents();
    int shown = 0;
    for (int i = static_cast<int>(events.size()) - 1; i >= 0 && shown < 12; --i, ++shown) {
        QLabel* line = makeLabel(qs(events[i]), "noteLine");
        line->setWordWrap(true);
        eventsBox->addWidget(line);
    }
    if (events.empty()) eventsBox->addWidget(makeLabel(QString::fromUtf8("Событий пока нет"), "muted"));
    content->addWidget(eventsCard);
}
