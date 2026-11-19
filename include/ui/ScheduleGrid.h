#pragma once
#include <vector>
#include <QRect>
#include <QWidget>
#include "barbershop.h"
#include "datetime.h"

class QScrollBar;
class QTimer;

class ScheduleGrid : public QWidget {
    Q_OBJECT

private:
    struct Block {
        QRect rect;
        int appointmentId;
        bool faded;
    };

    Barbershop& shop;
    DateTime day;
    bool showCancelled;
    int selectedId;
    int startMinute;
    int endMinute;
    int columnWidthValue;
    std::vector<int> masterIds;
    std::vector<Block> blocks;
    QScrollBar* bar;
    QTimer* timer;

    int contentHeight() const;
    int yOf(int minute) const;
    int columnX(int column) const;
    int columnAt(int x) const;
    void rebuild();
    void layoutBlocks();
    void updateScrollRange();
    void scrollToDefault();
    void paintHeader(QPainter& painter);
    void paintBody(QPainter& painter);
    void paintBlock(QPainter& painter, const Block& block);

private slots:
    void onScroll();
    void onTick();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

public:
    explicit ScheduleGrid(Barbershop& shopValue, QWidget* parent = nullptr);

    void setDay(const DateTime& value);
    void reload();
    void setShowCancelled(bool value);
    int selectedAppointmentId() const;
    void setSelectedAppointmentId(int id);

signals:
    void appointmentSelected(int appointmentId);
    void emptySlotActivated(int masterId, int minuteOfDay);
};
