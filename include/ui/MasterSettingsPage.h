#pragma once
#include "PageBase.h"

class QComboBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QTimeEdit;

class MasterSettingsPage : public PageBase {
    Q_OBJECT

private:
    QComboBox* masterCombo;
    QLabel* masterInfo;
    QTableWidget* offers;
    QPushButton* editButton;
    QPushButton* removeButton;
    QTimeEdit* startEdit;
    QTimeEdit* endEdit;
    QComboBox* dayOffCombo;
    bool loading;

    std::shared_ptr<Master> currentMaster() const;
    std::shared_ptr<Service> selectedService() const;
    void fillMasters();
    void fillOffers();
    void fillSchedule();

private slots:
    void onMasterChanged();
    void onSelectionChanged();
    void onAddOffer();
    void onEditOffer();
    void onRemoveOffer();
    void onSaveSchedule();

public:
    explicit MasterSettingsPage(AppContext* contextValue, QWidget* parent = nullptr);
    void refresh() override;
    QString title() const override;
};
