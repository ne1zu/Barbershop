#pragma once
#include "PageBase.h"

class QLabel;
class QPushButton;
class QTableWidget;

class StaffPage : public PageBase {
    Q_OBJECT

private:
    QTableWidget* table;
    QPushButton* removeButton;

private slots:
    void onSelectionChanged();
    void onAddMaster();
    void onAddAdmin();
    void onRemove();

public:
    explicit StaffPage(AppContext* contextValue, QWidget* parent = nullptr);
    void refresh() override;
    QString title() const override;
};

class ServicesCatalogPage : public PageBase {
    Q_OBJECT

private:
    QTableWidget* table;
    QPushButton* removeButton;

private slots:
    void onSelectionChanged();
    void onAdd();
    void onRemove();

public:
    explicit ServicesCatalogPage(AppContext* contextValue, QWidget* parent = nullptr);
    void refresh() override;
    QString title() const override;
};

class PaymentsPage : public PageBase {
    Q_OBJECT

private:
    QLabel* revenueValue;
    QLabel* countValue;
    QLabel* pendingValue;
    QTableWidget* unpaidTable;
    QTableWidget* paymentsTable;
    QPushButton* payButton;

    std::shared_ptr<Appointment> selectedUnpaid() const;

private slots:
    void onSelectionChanged();
    void onPay();

public:
    explicit PaymentsPage(AppContext* contextValue, QWidget* parent = nullptr);
    void refresh() override;
    QString title() const override;
};

class DataPage : public PageBase {
    Q_OBJECT

private slots:
    void onSave();
    void onLoad();
    void onReport();

public:
    explicit DataPage(AppContext* contextValue, QWidget* parent = nullptr);
    void refresh() override;
    QString title() const override;

signals:
    void reloginRequired();
};
