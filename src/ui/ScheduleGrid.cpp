#include "ScheduleGrid.h"
#include <algorithm>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QTimer>
#include <QWheelEvent>
#include "UiUtils.h"
#include "config.h"

namespace {
    const int HEADER_HEIGHT = 60;
    const int TIME_WIDTH = 64;
    const int ROW_HEIGHT = 24;
    const int MIN_COLUMN_WIDTH = 190;
    const int BAR_SIZE = 12;

    QColor statusColor(AppointmentStatus status) {
        switch (status) {
        case AppointmentStatus::Pending: return QColor("#e0b44c");
        case AppointmentStatus::Confirmed: return QColor("#46c28e");
        case AppointmentStatus::Arrived: return QColor("#55b2e6");
        case AppointmentStatus::NoShow: return QColor("#ef6b70");
        case AppointmentStatus::Cancelled: return QColor("#85858d");
        }
        return QColor("#85858d");
    }
}

ScheduleGrid::ScheduleGrid(Barbershop& shopValue, QWidget* parent)
    : QWidget(parent), shop(shopValue), day(DateTime::now()), showCancelled(false), selectedId(-1),
    startMinute(10 * 60), endMinute(20 * 60), columnWidthValue(MIN_COLUMN_WIDTH) {
    setMinimumHeight(360);
    setMouseTracking(false);
    bar = new QScrollBar(Qt::Vertical, this);
    connect(bar, &QScrollBar::valueChanged, this, &ScheduleGrid::onScroll);
    hbar = new QScrollBar(Qt::Horizontal, this);
    connect(hbar, &QScrollBar::valueChanged, this, &ScheduleGrid::onScroll);
    timer = new QTimer(this);
    timer->setInterval(60000);
    connect(timer, &QTimer::timeout, this, &ScheduleGrid::onTick);
    timer->start();
    rebuild();
}

void ScheduleGrid::onScroll() { update(); }
void ScheduleGrid::onTick() { update(); }

int ScheduleGrid::contentHeight() const {
    return (endMinute - startMinute) / GRID_STEP_MINUTES * ROW_HEIGHT + 12;
}

int ScheduleGrid::contentWidth() const {
    return TIME_WIDTH + static_cast<int>(masterIds.size()) * columnWidthValue;
}

int ScheduleGrid::viewportWidth() const { return std::max(0, width() - BAR_SIZE); }

int ScheduleGrid::viewportHeight() const {
    return std::max(0, height() - HEADER_HEIGHT - (hbar->maximum() > 0 ? BAR_SIZE : 0));
}

void ScheduleGrid::placeBars() {
    bool horizontal = hbar->maximum() > 0;
    hbar->setVisible(horizontal);
    bar->setGeometry(width() - BAR_SIZE, HEADER_HEIGHT, BAR_SIZE, viewportHeight());
    hbar->setGeometry(TIME_WIDTH, height() - BAR_SIZE, std::max(0, width() - TIME_WIDTH - BAR_SIZE), BAR_SIZE);
}

int ScheduleGrid::yOf(int minute) const {
    return (minute - startMinute) * ROW_HEIGHT / GRID_STEP_MINUTES;
}

int ScheduleGrid::columnX(int column) const {
    return TIME_WIDTH + column * columnWidthValue;
}

int ScheduleGrid::columnAt(int x) const {
    if (x < TIME_WIDTH) return -1;
    int column = (x - TIME_WIDTH) / columnWidthValue;
    if (column >= static_cast<int>(masterIds.size())) return -1;
    return column;
}

void ScheduleGrid::setDay(const DateTime& value) {
    day = value;
    rebuild();
    scrollToDefault();
}

void ScheduleGrid::reload() {
    int keep = bar->value();
    int keepX = hbar->value();
    rebuild();
    bar->setValue(std::min(keep, bar->maximum()));
    hbar->setValue(std::min(keepX, hbar->maximum()));
}

void ScheduleGrid::setShowCancelled(bool value) {
    showCancelled = value;
    layoutBlocks();
    update();
}

int ScheduleGrid::selectedAppointmentId() const { return selectedId; }

void ScheduleGrid::setSelectedAppointmentId(int id) {
    selectedId = id;
    update();
}

void ScheduleGrid::rebuild() {
    masterIds.clear();
    int first = 24 * 60;
    int last = 0;
    for (const auto& master : shop.getMasters()) {
        masterIds.push_back(master->getId());
        first = std::min(first, master->getWorkStart());
        last = std::max(last, master->getWorkEnd());
    }
    if (masterIds.empty()) {
        first = 10 * 60;
        last = 20 * 60;
    }
    for (const auto& appointment : shop.getAppointments()) {
        if (appointment->getStart().dayNumber() != day.dayNumber()) continue;
        first = std::min(first, appointment->getStart().minuteOfDay());
        last = std::max(last, std::min(24 * 60, appointment->getStart().minuteOfDay() + appointment->getDuration()));
    }
    startMinute = first / 60 * 60;
    endMinute = (last + 59) / 60 * 60;
    if (endMinute <= startMinute) endMinute = startMinute + 60;
    updateScrollRange();
    layoutBlocks();
    update();
}

void ScheduleGrid::updateScrollRange() {
    int visibleWidth = std::max(0, viewportWidth() - TIME_WIDTH);
    hbar->setRange(0, std::max(0, contentWidth() - TIME_WIDTH - visibleWidth));
    hbar->setPageStep(visibleWidth);
    hbar->setSingleStep(40);
    placeBars();
    int visible = viewportHeight();
    bar->setRange(0, std::max(0, contentHeight() - visible));
    bar->setPageStep(visible);
    bar->setSingleStep(ROW_HEIGHT);
}

void ScheduleGrid::scrollToDefault() {
    int target = 0;
    if (day.dayNumber() == DateTime::now().dayNumber()) {
        int now = DateTime::now().minuteOfDay();
        target = std::max(0, yOf(std::max(startMinute, now - 90)));
    }
    bar->setValue(std::min(target, bar->maximum()));
}

void ScheduleGrid::layoutBlocks() {
    blocks.clear();
    int count = std::max(1, static_cast<int>(masterIds.size()));
    int available = width() - TIME_WIDTH - BAR_SIZE;
    columnWidthValue = std::max(MIN_COLUMN_WIDTH, available / count);

    for (int pass = 0; pass < 2; ++pass) {
        bool fadedPass = pass == 0;
        for (const auto& appointment : shop.getAppointments()) {
            if (appointment->getStart().dayNumber() != day.dayNumber()) continue;
            bool faded = !appointment->occupiesTime();
            if (faded != fadedPass) continue;
            if (faded && !showCancelled) continue;
            int column = -1;
            for (std::size_t i = 0; i < masterIds.size(); ++i) {
                if (masterIds[i] == appointment->getMaster()->getId()) column = static_cast<int>(i);
            }
            if (column < 0) continue;
            int top = yOf(appointment->getStart().minuteOfDay());
            int height = std::max(26, appointment->getDuration() * ROW_HEIGHT / GRID_STEP_MINUTES - 3);
            int x = columnX(column) + 5;
            int width = columnWidthValue - 10;
            if (faded) {
                x += width / 3;
                width -= width / 3;
            }
            Block block = { QRect(x, top + 1, width, height), appointment->getId(), faded };
            blocks.push_back(block);
        }
    }
}

void ScheduleGrid::resizeEvent(QResizeEvent*) {
    layoutBlocks();
    updateScrollRange();
}

void ScheduleGrid::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().x() != 0 || (event->modifiers() & Qt::ShiftModifier)) {
        int delta = event->angleDelta().x() != 0 ? event->angleDelta().x() : event->angleDelta().y();
        hbar->setValue(hbar->value() - delta / 2);
    }
    else {
        bar->setValue(bar->value() - event->angleDelta().y() / 2);
    }
    event->accept();
}

void ScheduleGrid::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    int screenX = static_cast<int>(event->position().x());
    int x = screenX + hbar->value();
    int y = static_cast<int>(event->position().y());
    if (y < HEADER_HEIGHT || screenX < TIME_WIDTH) return;
    int contentY = y - HEADER_HEIGHT + bar->value();
    int found = -1;
    for (int i = static_cast<int>(blocks.size()) - 1; i >= 0; --i) {
        if (blocks[i].rect.contains(x, contentY)) {
            found = blocks[i].appointmentId;
            break;
        }
    }
    selectedId = found;
    update();
    emit appointmentSelected(found);
}

void ScheduleGrid::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    int screenX = static_cast<int>(event->position().x());
    int x = screenX + hbar->value();
    int y = static_cast<int>(event->position().y());
    if (y < HEADER_HEIGHT || screenX < TIME_WIDTH) return;
    int contentY = y - HEADER_HEIGHT + bar->value();
    for (const Block& block : blocks) {
        if (block.rect.contains(x, contentY)) return;
    }
    int column = columnAt(x);
    if (column < 0) return;
    int minute = startMinute + contentY / ROW_HEIGHT * GRID_STEP_MINUTES;
    if (minute >= endMinute) return;
    emit emptySlotActivated(masterIds[column], minute);
}

void ScheduleGrid::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#0d0d0f"));
    paintBody(painter);
    paintHeader(painter);
}

void ScheduleGrid::paintHeader(QPainter& painter) {
    int right = viewportWidth();
    painter.fillRect(QRect(0, 0, width(), HEADER_HEIGHT), QColor("#111113"));
    painter.setPen(QColor("#2c2c32"));
    painter.drawLine(0, HEADER_HEIGHT - 1, width(), HEADER_HEIGHT - 1);

    painter.save();
    painter.setClipRect(QRect(TIME_WIDTH, 0, std::max(0, right - TIME_WIDTH), HEADER_HEIGHT));
    painter.translate(-hbar->value(), 0);
    for (std::size_t i = 0; i < masterIds.size(); ++i) {
        std::shared_ptr<Master> master = findMasterById(shop, masterIds[i]);
        if (!master) continue;
        int x = columnX(static_cast<int>(i));
        painter.setPen(QColor("#2c2c32"));
        painter.drawLine(x, 0, x, HEADER_HEIGHT);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#e6c9a2"));
        painter.drawEllipse(QRect(x + 12, 14, 32, 32));
        QFont initialsFont = font();
        initialsFont.setPixelSize(12);
        initialsFont.setBold(true);
        painter.setFont(initialsFont);
        painter.setPen(QColor("#1b1509"));
        painter.drawText(QRect(x + 12, 14, 32, 32), Qt::AlignCenter, initials(master->getName()));

        int textX = x + 52;
        int textWidth = std::max(10, columnWidthValue - 60);
        QFont nameFont = font();
        nameFont.setPixelSize(13);
        nameFont.setBold(true);
        painter.setFont(nameFont);
        painter.setPen(QColor("#f1f1f1"));
        QFontMetrics nameMetrics(nameFont);
        painter.drawText(QRect(textX, 12, textWidth, 18), Qt::AlignLeft | Qt::AlignVCenter,
            nameMetrics.elidedText(qs(master->getName()), Qt::ElideRight, textWidth));

        QFont infoFont = font();
        infoFont.setPixelSize(11);
        painter.setFont(infoFont);
        bool dayOff = master->getDayOff() == day.dayOfWeek();
        painter.setPen(dayOff ? QColor("#ef6b70") : QColor("#8e8e97"));
        QString info = dayOff ? QString::fromUtf8("Выходной")
            : levelText(master->classify()) + QString::fromUtf8("  ★ ") + QString::number(master->getRating(), 'f', 1);
        QFontMetrics infoMetrics(infoFont);
        painter.drawText(QRect(textX, 31, textWidth, 16), Qt::AlignLeft | Qt::AlignVCenter,
            infoMetrics.elidedText(info, Qt::ElideRight, textWidth));
    }
    painter.restore();

    painter.fillRect(QRect(0, 0, TIME_WIDTH, HEADER_HEIGHT), QColor("#111113"));
    painter.setPen(QColor("#2c2c32"));
    painter.drawLine(0, HEADER_HEIGHT - 1, TIME_WIDTH, HEADER_HEIGHT - 1);
    QFont small = font();
    small.setPixelSize(10);
    small.setBold(true);
    painter.setFont(small);
    painter.setPen(QColor("#8e8e97"));
    painter.drawText(QRect(0, 0, TIME_WIDTH, HEADER_HEIGHT), Qt::AlignCenter, QString::fromUtf8("ВРЕМЯ"));
}

void ScheduleGrid::paintBody(QPainter& painter) {
    int right = contentWidth();
    painter.save();
    painter.setClipRect(QRect(TIME_WIDTH, HEADER_HEIGHT, std::max(0, viewportWidth() - TIME_WIDTH), viewportHeight()));
    painter.translate(-hbar->value(), HEADER_HEIGHT - bar->value());

    int columns = static_cast<int>(masterIds.size());
    int bodyHeight = contentHeight();

    for (int i = 0; i < columns; ++i) {
        std::shared_ptr<Master> master = findMasterById(shop, masterIds[i]);
        if (!master) continue;
        int x = columnX(i);
        QRect column(x, 0, columnWidthValue, bodyHeight);
        QBrush hatch(QColor("#1b1b1f"), Qt::BDiagPattern);
        if (master->getDayOff() == day.dayOfWeek()) {
            painter.fillRect(column, QColor("#101012"));
            painter.fillRect(column, hatch);
            continue;
        }
        int workTop = yOf(master->getWorkStart());
        int workBottom = yOf(master->getWorkEnd());
        painter.fillRect(QRect(x, workTop, columnWidthValue, workBottom - workTop), QColor("#121214"));
        painter.fillRect(QRect(x, 0, columnWidthValue, workTop), QColor("#0d0d0f"));
        painter.fillRect(QRect(x, 0, columnWidthValue, workTop), hatch);
        painter.fillRect(QRect(x, workBottom, columnWidthValue, bodyHeight - workBottom), QColor("#0d0d0f"));
        painter.fillRect(QRect(x, workBottom, columnWidthValue, bodyHeight - workBottom), hatch);
    }

    QFont timeFont = font();
    timeFont.setPixelSize(11);
    painter.setFont(timeFont);
    for (int minute = startMinute; minute <= endMinute; minute += GRID_STEP_MINUTES) {
        int y = yOf(minute);
        if (minute % 60 == 0) {
            painter.setPen(QColor("#2c2c32"));
            painter.drawLine(TIME_WIDTH, y, right, y);
        }
        else if (minute % 30 == 0) {
            painter.setPen(QPen(QColor("#222226"), 1, Qt::DashLine));
            painter.drawLine(TIME_WIDTH, y, right, y);
        }
    }

    painter.setPen(QColor("#2c2c32"));
    for (int i = 0; i <= columns; ++i) painter.drawLine(columnX(i), 0, columnX(i), bodyHeight);

    for (const Block& block : blocks) paintBlock(painter, block);

    DateTime now = DateTime::now();
    if (day.dayNumber() == now.dayNumber() && now.minuteOfDay() >= startMinute && now.minuteOfDay() <= endMinute) {
        int y = yOf(now.minuteOfDay());
        painter.setPen(QPen(QColor("#ef6b70"), 2));
        painter.drawLine(TIME_WIDTH, y, right, y);
    }

    if (columns == 0) {
        painter.setPen(QColor("#8e8e97"));
        painter.drawText(QRect(TIME_WIDTH, 40, viewportWidth(), 40), Qt::AlignCenter, QString::fromUtf8("Мастера ещё не добавлены"));
    }
    painter.restore();

    painter.save();
    painter.setClipRect(QRect(0, HEADER_HEIGHT, TIME_WIDTH, viewportHeight()));
    painter.translate(0, HEADER_HEIGHT - bar->value());
    painter.fillRect(QRect(0, 0, TIME_WIDTH, bodyHeight), QColor("#0d0d0f"));
    painter.setFont(timeFont);
    painter.setPen(QColor("#8e8e97"));
    for (int minute = startMinute; minute <= endMinute; minute += 60) {
        painter.drawText(QRect(0, yOf(minute) - 8, TIME_WIDTH - 12, 16), Qt::AlignRight | Qt::AlignVCenter,
            formatClock(minute));
    }
    if (day.dayNumber() == now.dayNumber() && now.minuteOfDay() >= startMinute && now.minuteOfDay() <= endMinute) {
        painter.setBrush(QColor("#ef6b70"));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPoint(TIME_WIDTH - 4, yOf(now.minuteOfDay())), 4, 4);
    }
    painter.setPen(QColor("#2c2c32"));
    painter.drawLine(TIME_WIDTH, 0, TIME_WIDTH, bodyHeight);
    painter.restore();
}

void ScheduleGrid::paintBlock(QPainter& painter, const Block& block) {
    std::shared_ptr<Appointment> appointment = findAppointmentById(shop, block.appointmentId);
    if (!appointment) return;

    QColor base = statusColor(appointment->getStatus());
    QColor fill = base;
    fill.setAlpha(block.faded ? 28 : 52);
    QRect r = block.rect;

    painter.setPen(Qt::NoPen);
    painter.setBrush(block.faded ? QColor("#17171a") : QColor("#16161a"));
    painter.drawRoundedRect(r, 9, 9);
    painter.setBrush(fill);
    painter.drawRoundedRect(r, 9, 9);

    QPainterPath path;
    path.addRoundedRect(r, 9, 9);
    painter.save();
    painter.setClipPath(path, Qt::IntersectClip);
    painter.fillRect(QRect(r.left(), r.top(), 4, r.height()), base);
    painter.restore();

    if (appointment->getId() == selectedId) {
        painter.setPen(QPen(QColor("#e6c9a2"), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(r.adjusted(1, 1, -1, -1), 9, 9);
    }

    int left = r.left() + 14;
    int textWidth = std::max(10, r.width() - 22);
    QString range = formatClock(appointment->getStart().minuteOfDay()) + "–"
        + formatClock(appointment->getEnd().minuteOfDay());
    QString clientName = qs(appointment->getClient()->getName());
    if (appointment->isFirstVisit()) clientName += QString::fromUtf8("  · новый");

    QFont nameFont = font();
    nameFont.setPixelSize(12);
    nameFont.setBold(true);
    QFont smallFont = font();
    smallFont.setPixelSize(11);
    QFontMetrics nameMetrics(nameFont);
    QFontMetrics smallMetrics(smallFont);
    QColor textColor = block.faded ? QColor("#8e8e97") : QColor("#f1f1f1");

    if (r.height() < 50) {
        painter.setFont(nameFont);
        painter.setPen(textColor);
        QString line = range.left(5) + "  " + clientName + "  ·  " + qs(appointment->getService()->getName());
        painter.drawText(QRect(left, r.top(), textWidth, r.height()), Qt::AlignLeft | Qt::AlignVCenter,
            nameMetrics.elidedText(line, Qt::ElideRight, textWidth));
        return;
    }

    painter.setFont(smallFont);
    painter.setPen(QColor("#8e8e97"));
    painter.drawText(QRect(left, r.top() + 6, textWidth, 15), Qt::AlignLeft | Qt::AlignVCenter, range);

    QString statusLabel = statusText(appointment->getStatus());
    if (appointment->isPaid()) statusLabel += QString::fromUtf8(" · оплачено");
    painter.setPen(base);
    painter.drawText(QRect(left, r.top() + 6, textWidth, 15), Qt::AlignRight | Qt::AlignVCenter,
        smallMetrics.elidedText(statusLabel, Qt::ElideRight, std::max(10, textWidth - 72)));

    painter.setFont(nameFont);
    painter.setPen(textColor);
    painter.drawText(QRect(left, r.top() + 22, textWidth, 17), Qt::AlignLeft | Qt::AlignVCenter,
        nameMetrics.elidedText(clientName, Qt::ElideRight, textWidth));

    painter.setFont(smallFont);
    painter.setPen(QColor("#b9b9c0"));
    painter.drawText(QRect(left, r.top() + 39, textWidth, 15), Qt::AlignLeft | Qt::AlignVCenter,
        smallMetrics.elidedText(qs(appointment->getService()->getName()), Qt::ElideRight, textWidth));
}
