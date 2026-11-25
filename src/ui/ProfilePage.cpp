#include "ProfilePage.h"
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>
#include "UiUtils.h"
#include "config.h"

ProfilePage::ProfilePage(AppContext* contextValue, QWidget* parent) : PageBase(contextValue, parent) {
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

    bannerLabel = makeLabel(QString::fromUtf8("Онлайн-запись для вашего аккаунта недоступна. Обратитесь к администратору барбершопа."),
        "dangerBanner");
    bannerLabel->setWordWrap(true);
    root->addWidget(bannerLabel);

    QFrame* card = makeCard();
    QHBoxLayout* head = new QHBoxLayout(card);
    head->setContentsMargins(24, 22, 24, 22);
    head->setSpacing(18);
    avatar = makeLabel(QString(), "avatarBig");
    avatar->setFixedSize(60, 60);
    avatar->setAlignment(Qt::AlignCenter);
    head->addWidget(avatar);
    QVBoxLayout* names = new QVBoxLayout;
    names->setSpacing(4);
    nameLabel = makeLabel(QString(), "bigName");
    phoneLabel = makeLabel(QString(), "muted");
    names->addWidget(nameLabel);
    names->addWidget(phoneLabel);
    head->addLayout(names, 1);
    levelBadge = makeBadge(QString(), "regular");
    head->addWidget(levelBadge, 0, Qt::AlignTop);
    root->addWidget(card);

    QHBoxLayout* stats = new QHBoxLayout;
    stats->setSpacing(14);
    stats->addWidget(makeStatCard(QString::fromUtf8("Визитов"), "0", QString(), false, &visitsValue));
    stats->addWidget(makeStatCard(QString::fromUtf8("Потрачено"), "0", QString(), false, &spentValue));
    stats->addWidget(makeStatCard(QString::fromUtf8("Ваша скидка"), "0%", QString(), true, &discountValue));
    root->addLayout(stats);

    QFrame* loyalty = makeCard();
    QVBoxLayout* loyaltyBox = new QVBoxLayout(loyalty);
    loyaltyBox->setContentsMargins(24, 20, 24, 20);
    loyaltyBox->setSpacing(10);
    loyaltyBox->addWidget(makeLabel(QString::fromUtf8("Программа лояльности"), "sectionTitle"));
    progressText = makeLabel(QString(), "muted");
    progressText->setWordWrap(true);
    loyaltyBox->addWidget(progressText);
    progress = new QProgressBar;
    progress->setRange(0, 100);
    progress->setTextVisible(false);
    loyaltyBox->addWidget(progress);
    loyaltyBox->addWidget(makeLabel(QString::fromUtf8("Бронза: %1 визитов, скидка %2%  ·  Серебро: %3 визитов, скидка %4%  ·  Золото: %5 визитов, скидка %6%")
        .arg(BRONZE_VISITS).arg(BRONZE_DISCOUNT).arg(SILVER_VISITS).arg(SILVER_DISCOUNT).arg(GOLD_VISITS)
        .arg(GOLD_DISCOUNT), "mutedSmall"));
    root->addWidget(loyalty);

    root->addWidget(makeLabel(QString::fromUtf8("История визитов"), "sectionTitle"));
    history = makeTable({ QString::fromUtf8("Дата"), QString::fromUtf8("Время"), QString::fromUtf8("Мастер"),
        QString::fromUtf8("Услуга"), QString::fromUtf8("Цена"), QString::fromUtf8("Статус") });
    history->setSelectionMode(QAbstractItemView::NoSelection);
    fixColumn(history, 5, 150);
    root->addWidget(history);
    root->addStretch(1);
}

QString ProfilePage::title() const { return QString::fromUtf8("Мой профиль"); }

void ProfilePage::refresh() {
    std::shared_ptr<Client> me = ctx()->client();
    if (!me) return;
    avatar->setText(initials(me->getName()));
    nameLabel->setText(qs(me->getName()));
    phoneLabel->setText(qs(me->getPhone()));

    bool banned = me->isBanned();
    bannerLabel->setVisible(banned);
    int visits = me->getVisitsCount();
    if (banned) {
        setBadge(levelBadge, QString::fromUtf8("Ограничен"), "banned");
    }
    else if (visits >= GOLD_VISITS) {
        setBadge(levelBadge, QString::fromUtf8("Золото"), "gold");
    }
    else if (visits >= SILVER_VISITS) {
        setBadge(levelBadge, QString::fromUtf8("Серебро"), "silver");
    }
    else if (visits >= BRONZE_VISITS) {
        setBadge(levelBadge, QString::fromUtf8("Бронза"), "bronze");
    }
    else if (visits == 0) {
        setBadge(levelBadge, QString::fromUtf8("Новый клиент"), "new");
    }
    else {
        setBadge(levelBadge, QString::fromUtf8("Постоянный клиент"), "regular");
    }

    visitsValue->setText(QString::number(visits));
    spentValue->setText(money(me->getTotalSpent()));
    discountValue->setText(QString("%1%").arg(me->getDiscountPercent()));

    int next = 0;
    if (visits < BRONZE_VISITS) next = BRONZE_VISITS;
    else if (visits < SILVER_VISITS) next = SILVER_VISITS;
    else if (visits < GOLD_VISITS) next = GOLD_VISITS;
    if (next == 0) {
        progressText->setText(QString::fromUtf8("Вы достигли максимального уровня. Спасибо, что с нами!"));
        progress->setValue(100);
    }
    else {
        progressText->setText(QString::fromUtf8("До следующего уровня осталось визитов: %1 (цель — %2).")
            .arg(next - visits).arg(next));
        progress->setValue(visits * 100 / next);
    }

    history->setRowCount(0);
    std::vector<std::shared_ptr<Appointment>> list = sortedAppointments(shop());
    for (int i = static_cast<int>(list.size()) - 1; i >= 0; --i) {
        const std::shared_ptr<Appointment>& appointment = list[i];
        if (appointment->getClient().get() != me.get()) continue;
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
