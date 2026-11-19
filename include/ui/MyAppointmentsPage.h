#pragma once
#include "PageBase.h"

class QLabel;
class QPushButton;
class QTableWidget;

class MyAppointmentsPage : public PageBase {
    Q_OBJECT

private:
    QTableWidget* table;
    QLabel* emptyLabel;
    QPushButton* rescheduleButton;
    QPushButton* cancelButton;

    std::shared_ptr<Appointment> selected() const;

private slots:
    void onSelectionChanged();
    void onReschedule();
    void onCancel();

public:
    explicit MyAppointmentsPage(AppContext* contextValue, QWidget* parent = nullptr);
    void refresh() override;
    QString title() const override;
};
