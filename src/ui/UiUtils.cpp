#include "UiUtils.h"
#include <algorithm>
#include <QBoxLayout>
#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QStyle>
#include <QTableWidget>

QString qs(const std::string& text) { return QString::fromStdString(text); }
std::string ss(const QString& text) { return text.toStdString(); }

QDate toQDate(const DateTime& value) {
    return QDate(value.getYear(), value.getMonth(), value.getDay());
}

DateTime toDateTime(const QDate& date, int minuteOfDay) {
    return DateTime(date.year(), date.month(), date.day(), minuteOfDay / 60, minuteOfDay % 60);
}

QString formatClock(int minuteOfDay) {
    return QString("%1:%2").arg(minuteOfDay / 60, 2, 10, QChar('0')).arg(minuteOfDay % 60, 2, 10, QChar('0'));
}

QString formatDay(const DateTime& value) {
    return QString("%1.%2.%3")
        .arg(value.getDay(), 2, 10, QChar('0'))
        .arg(value.getMonth(), 2, 10, QChar('0'))
        .arg(value.getYear());
}

QString formatDayTime(const DateTime& value) {
    return formatDay(value) + " " + formatClock(value.getHour() * 60 + value.getMinute());
}

QString money(double value) {
    return QString::number(value, 'f', 2) + " BYN";
}

QString statusText(AppointmentStatus status) {
    switch (status) {
    case AppointmentStatus::Pending: return QString::fromUtf8("Ожидает");
    case AppointmentStatus::Confirmed: return QString::fromUtf8("Подтверждена");
    case AppointmentStatus::Arrived: return QString::fromUtf8("Пришёл");
    case AppointmentStatus::NoShow: return QString::fromUtf8("Неявка");
    case AppointmentStatus::Cancelled: return QString::fromUtf8("Отменена");
    }
    return QString();
}

QString statusKind(AppointmentStatus status) {
    switch (status) {
    case AppointmentStatus::Pending: return "pending";
    case AppointmentStatus::Confirmed: return "confirmed";
    case AppointmentStatus::Arrived: return "arrived";
    case AppointmentStatus::NoShow: return "noshow";
    case AppointmentStatus::Cancelled: return "cancelled";
    }
    return QString();
}

QString levelText(const std::string& level) {
    const QString key = qs(level);
    if (key == "New") return QString::fromUtf8("Новый");
    if (key == "Regular") return QString::fromUtf8("Постоянный");
    if (key == "Bronze") return QString::fromUtf8("Бронза");
    if (key == "Silver") return QString::fromUtf8("Серебро");
    if (key == "Gold") return QString::fromUtf8("Золото");
    if (key == "Banned") return QString::fromUtf8("Заблокирован");
    if (key == "Top") return QString::fromUtf8("Топ-мастер");
    if (key == "Standard") return QString::fromUtf8("Мастер");
    if (key == "Junior") return QString::fromUtf8("Младший мастер");
    return key;
}

QString levelKind(const std::string& level) {
    return qs(level).toLower();
}

QString paymentMethodText(PaymentMethod method) {
    return method == PaymentMethod::Cash ? QString::fromUtf8("Наличные") : QString::fromUtf8("Карта");
}

QString weekdayName(int dayOfWeek) {
    const char* const names[] = { "Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье" };
    if (dayOfWeek < 0 || dayOfWeek > 6) return QString();
    return QString::fromUtf8(names[dayOfWeek]);
}

QString weekdayShort(int dayOfWeek) {
    const char* const names[] = { "Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс" };
    if (dayOfWeek < 0 || dayOfWeek > 6) return QString();
    return QString::fromUtf8(names[dayOfWeek]);
}

QString dayOffText(int dayOff) {
    if (dayOff < 0 || dayOff > 6) return QString::fromUtf8("нет");
    return weekdayName(dayOff);
}

QString fullDayTitle(const DateTime& value) {
    return weekdayName(value.dayOfWeek()) + ", " + formatDay(value);
}

QString initials(const std::string& name) {
    const QStringList parts = qs(name).split(' ', Qt::SkipEmptyParts);
    QString result;
    for (int i = 0; i < parts.size() && i < 2; ++i) result += parts[i].left(1).toUpper();
    return result.isEmpty() ? QString("?") : result;
}

std::shared_ptr<Client> findClientById(const Barbershop& shop, int id) {
    for (const auto& item : shop.getClients()) {
        if (item->getId() == id) return item;
    }
    return nullptr;
}

std::shared_ptr<Master> findMasterById(const Barbershop& shop, int id) {
    for (const auto& item : shop.getMasters()) {
        if (item->getId() == id) return item;
    }
    return nullptr;
}

std::shared_ptr<Service> findServiceById(const Barbershop& shop, int id) {
    for (const auto& item : shop.getServices()) {
        if (item->getId() == id) return item;
    }
    return nullptr;
}

std::shared_ptr<Appointment> findAppointmentById(const Barbershop& shop, int id) {
    for (const auto& item : shop.getAppointments()) {
        if (item->getId() == id) return item;
    }
    return nullptr;
}

namespace {
    bool startsEarlier(const std::shared_ptr<Appointment>& first, const std::shared_ptr<Appointment>& second) {
        return first->getStart() < second->getStart();
    }
}

std::vector<std::shared_ptr<Appointment>> sortedAppointments(const Barbershop& shop) {
    std::vector<std::shared_ptr<Appointment>> result;
    for (const auto& item : shop.getAppointments()) result.push_back(item);
    std::sort(result.begin(), result.end(), startsEarlier);
    return result;
}

QLabel* makeLabel(const QString& text, const QString& objectName) {
    QLabel* label = new QLabel(text);
    if (!objectName.isEmpty()) label->setObjectName(objectName);
    return label;
}

void repolish(QWidget* widget) {
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

void setBadge(QLabel* badge, const QString& text, const QString& kind) {
    badge->setText(text);
    badge->setProperty("kind", kind);
    repolish(badge);
}

QLabel* makeBadge(const QString& text, const QString& kind) {
    QLabel* badge = new QLabel(text);
    badge->setObjectName("badge");
    badge->setProperty("kind", kind);
    badge->setAlignment(Qt::AlignCenter);
    badge->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    return badge;
}

QWidget* makeBadgeCell(const QString& text, const QString& kind) {
    QWidget* cell = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout(cell);
    layout->setContentsMargins(8, 0, 8, 0);
    layout->addWidget(makeBadge(text, kind), 0, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addStretch(1);
    return cell;
}

QPushButton* makeButton(const QString& text, const QString& kind) {
    QPushButton* button = new QPushButton(text);
    if (!kind.isEmpty()) button->setProperty("kind", kind);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

QFrame* makeCard() {
    QFrame* card = new QFrame;
    card->setObjectName("card");
    return card;
}

QFrame* makeStatCard(const QString& title, const QString& value, const QString& hint, bool accent,
    QLabel** valueLabel) {
    QFrame* card = makeCard();
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 16);
    layout->setSpacing(4);
    layout->addWidget(makeLabel(title.toUpper(), "statTitle"));
    QLabel* valueText = makeLabel(value, accent ? "statValueAccent" : "statValue");
    layout->addWidget(valueText);
    QLabel* hintText = makeLabel(hint, "mutedSmall");
    hintText->setWordWrap(true);
    hintText->setVisible(!hint.isEmpty());
    layout->addWidget(hintText);
    if (valueLabel != nullptr) *valueLabel = valueText;
    return card;
}

QWidget* makeBarRow(const QString& label, double value, double maxValue, const QString& valueText) {
    QWidget* row = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 3, 0, 3);
    layout->setSpacing(12);
    QLabel* name = makeLabel(label);
    name->setMinimumWidth(150);
    name->setMaximumWidth(190);
    layout->addWidget(name);
    QProgressBar* bar = new QProgressBar;
    bar->setRange(0, 1000);
    bar->setValue(maxValue > 0 ? static_cast<int>(value / maxValue * 1000.0) : 0);
    bar->setTextVisible(false);
    layout->addWidget(bar, 1);
    QLabel* amount = makeLabel(valueText);
    amount->setMinimumWidth(90);
    amount->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(amount);
    return row;
}

QWidget* makeInfoRow(const QString& key, QLabel** valueLabel) {
    QWidget* row = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 2, 0, 2);
    QLabel* keyLabel = makeLabel(key, "infoKey");
    keyLabel->setMinimumWidth(96);
    keyLabel->setMaximumWidth(96);
    layout->addWidget(keyLabel);
    QLabel* value = makeLabel("-", "infoValue");
    value->setWordWrap(true);
    value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(value, 1);
    *valueLabel = value;
    return row;
}

QTableWidget* makeTable(const QStringList& headers) {
    QTableWidget* table = new QTableWidget;
    table->setColumnCount(headers.size());
    QStringList upper;
    for (const QString& header : headers) upper << header.toUpper();
    table->setHorizontalHeaderLabels(upper);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    table->setWordWrap(false);
    table->setFocusPolicy(Qt::NoFocus);
    table->verticalHeader()->hide();
    table->verticalHeader()->setDefaultSectionSize(44);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->horizontalHeader()->setHighlightSections(false);
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    return table;
}

QTableWidgetItem* makeItem(const QString& text, bool alignRight) {
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setTextAlignment((alignRight ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter);
    return item;
}

void fitTableHeight(QTableWidget* table) {
    int height = table->horizontalHeader()->sizeHint().height() + 4;
    for (int row = 0; row < table->rowCount(); ++row) height += table->rowHeight(row);
    table->setFixedHeight(height);
}

void clearLayout(QLayout* layout) {
    while (layout->count() > 0) {
        QLayoutItem* item = layout->takeAt(0);
        if (item->widget() != nullptr) {
            item->widget()->hide();
            item->widget()->deleteLater();
        }
        else if (item->layout() != nullptr) {
            clearLayout(item->layout());
        }
        delete item;
    }
}

namespace {
    QPixmap drawIcon(const QString& name, const QColor& color) {
        const int size = 20;
        QPixmap pixmap(size * 2, size * 2);
        pixmap.setDevicePixelRatio(2.0);
        pixmap.fill(Qt::transparent);
        QPainter p(&pixmap);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(color, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(Qt::NoBrush);

        if (name == "calendar") {
            p.drawRoundedRect(QRectF(3, 4.5, 14, 12.5), 2.5, 2.5);
            p.drawLine(QPointF(3, 8.5), QPointF(17, 8.5));
            p.drawLine(QPointF(7, 2.5), QPointF(7, 5.5));
            p.drawLine(QPointF(13, 2.5), QPointF(13, 5.5));
        }
        else if (name == "list") {
            p.drawLine(QPointF(8, 5.5), QPointF(17, 5.5));
            p.drawLine(QPointF(8, 10), QPointF(17, 10));
            p.drawLine(QPointF(8, 14.5), QPointF(17, 14.5));
            p.drawPoint(QPointF(4, 5.5));
            p.drawPoint(QPointF(4, 10));
            p.drawPoint(QPointF(4, 14.5));
        }
        else if (name == "users") {
            p.drawEllipse(QPointF(7.5, 7), 2.8, 2.8);
            p.drawArc(QRectF(2.5, 11, 10, 8), 0, 180 * 16);
            p.drawEllipse(QPointF(14, 7.5), 2.2, 2.2);
            p.drawArc(QRectF(11.5, 11.5, 7, 6), 0, 180 * 16);
        }
        else if (name == "user") {
            p.drawEllipse(QPointF(10, 7), 3.2, 3.2);
            p.drawArc(QRectF(4, 12, 12, 9), 0, 180 * 16);
        }
        else if (name == "scissors") {
            p.drawEllipse(QPointF(5.5, 15), 2.5, 2.5);
            p.drawEllipse(QPointF(14.5, 15), 2.5, 2.5);
            p.drawLine(QPointF(7, 13), QPointF(14, 3));
            p.drawLine(QPointF(13, 13), QPointF(6, 3));
        }
        else if (name == "cash") {
            p.drawRoundedRect(QRectF(2, 5, 16, 10), 2.5, 2.5);
            p.drawEllipse(QPointF(10, 10), 2.4, 2.4);
        }
        else if (name == "chart") {
            p.drawLine(QPointF(5, 16.5), QPointF(5, 10));
            p.drawLine(QPointF(10, 16.5), QPointF(10, 4.5));
            p.drawLine(QPointF(15, 16.5), QPointF(15, 8));
        }
        else if (name == "clock") {
            p.drawEllipse(QPointF(10, 10), 7.2, 7.2);
            p.drawLine(QPointF(10, 10), QPointF(10, 5.5));
            p.drawLine(QPointF(10, 10), QPointF(13.2, 11.5));
        }
        else if (name == "database") {
            p.drawEllipse(QRectF(4, 3, 12, 4.5));
            p.drawLine(QPointF(4, 5.2), QPointF(4, 14.8));
            p.drawLine(QPointF(16, 5.2), QPointF(16, 14.8));
            p.drawArc(QRectF(4, 12.5, 12, 4.5), 180 * 16, 180 * 16);
        }
        else if (name == "book") {
            p.drawRoundedRect(QRectF(4, 2.5, 12, 15), 2, 2);
            p.drawLine(QPointF(7.5, 7), QPointF(12.5, 7));
            p.drawLine(QPointF(7.5, 10.5), QPointF(12.5, 10.5));
        }
        else if (name == "plus") {
            p.drawLine(QPointF(10, 4), QPointF(10, 16));
            p.drawLine(QPointF(4, 10), QPointF(16, 10));
        }
        return pixmap;
    }
}

QIcon makeIcon(const QString& name) {
    QIcon icon;
    icon.addPixmap(drawIcon(name, QColor("#8e8e97")), QIcon::Normal, QIcon::Off);
    icon.addPixmap(drawIcon(name, QColor("#f1f1f1")), QIcon::Active, QIcon::Off);
    icon.addPixmap(drawIcon(name, QColor("#e6c9a2")), QIcon::Normal, QIcon::On);
    icon.addPixmap(drawIcon(name, QColor("#e6c9a2")), QIcon::Active, QIcon::On);
    return icon;
}
