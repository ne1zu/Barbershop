#pragma once
#include "AppointmentActions.h"
#include "PageBase.h"

class QCheckBox;
class QDateEdit;
class QFrame;
class QLabel;
class QPushButton;
class QVBoxLayout;
class ScheduleGrid;

class SchedulePage : public PageBase {
    Q_OBJECT

private:
    QDateEdit* dateEdit;
    QLabel* weekdayLabel;
    QCheckBox* cancelledCheck;
    ScheduleGrid* grid;

    QFrame* detailsCard;
    QLabel* emptyLabel;
    QWidget* detailsBox;
    QLabel* statusBadge;
    QLabel* paidBadge;
    QLabel* clientLabel;
    QLabel* phoneLabel;
    QLabel* serviceValue;
    QLabel* masterValue;
    QLabel* timeValue;
    QLabel* durationValue;
    QLabel* priceValue;
    QLabel* discountValue;
    QVBoxLayout* notesLayout;
    QPushButton* confirmButton;
    QPushButton* arrivedButton;
    QPushButton* noShowButton;
    QPushButton* cancelButton;
    QPushButton* rescheduleButton;
    QPushButton* payButton;

    int selectedId;

    std::shared_ptr<Appointment> current() const;
    void updateDetails();
    void afterAction(bool changed);
    void buildDetails();

private slots:
    void onPrevDay();
    void onNextDay();
    void onToday();
    void onDateChanged();
    void onShowCancelled();
    void onSelected(int appointmentId);
    void onEmptySlot(int masterId, int minuteOfDay);
    void onConfirm();
    void onArrived();
    void onNoShow();
    void onCancel();
    void onReschedule();
    void onPay();

public:
    explicit SchedulePage(AppContext* contextValue, QWidget* parent = nullptr);
    void refresh() override;
    QString title() const override;
};
