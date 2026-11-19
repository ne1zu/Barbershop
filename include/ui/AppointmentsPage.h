#pragma once
#include "PageBase.h"

class QComboBox;
class QDateEdit;
class QPushButton;
class QTableWidget;

class AppointmentsPage : public PageBase {
    Q_OBJECT

private:
    QComboBox* modeCombo;
    QDateEdit* dateEdit;
    QComboBox* masterCombo;
    QTableWidget* table;
    QPushButton* confirmButton;
    QPushButton* arrivedButton;
    QPushButton* noShowButton;
    QPushButton* rescheduleButton;
    QPushButton* cancelButton;
    QPushButton* payButton;
    QPushButton* removeButton;
    bool loading;

    std::shared_ptr<Appointment> selected() const;
    void fillTable();
    void afterAction(bool changed);

private slots:
    void onFilterChanged();
    void onSelectionChanged();
    void onConfirm();
    void onArrived();
    void onNoShow();
    void onReschedule();
    void onCancel();
    void onPay();
    void onRemove();

public:
    explicit AppointmentsPage(AppContext* contextValue, QWidget* parent = nullptr);
    void refresh() override;
    QString title() const override;
};
