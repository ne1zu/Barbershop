#pragma once
#include "PageBase.h"

class QAction;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QMenu;
class QPushButton;
class QTableWidget;
class QVBoxLayout;
class QWidget;

class ClientsPage : public PageBase {
    Q_OBJECT

private:
    QLineEdit* searchEdit;
    QListWidget* list;
    QWidget* detailsHost;
    QLabel* emptyLabel;
    QLabel* avatar;
    QLabel* nameLabel;
    QLabel* phoneLabel;
    QLabel* levelBadge;
    QLabel* bannedBadge;
    QLabel* visitsValue;
    QLabel* spentValue;
    QLabel* noShowValue;
    QLabel* cancelValue;
    QLabel* discountValue;
    QLabel* banInfo;
    QLabel* countLabel;
    QLabel* notesTitle;
    QComboBox* filterCombo;
    QListWidget* notesList;
    QVBoxLayout* loyaltyLayout;
    QLineEdit* noteEdit;
    QPushButton* actionsButton;
    QAction* banAction;
    QAction* unbanAction;
    QAction* visitsAction;
    QTableWidget* history;
    bool loading;

    std::shared_ptr<Client> selected() const;
    void fillList();
    void showDetails();
    void fillLoyalty(const Client& client);

private slots:
    void onSearchChanged();
    void onFilterChanged();
    void onSelectionChanged();
    void onNewClient();
    void onAddNote();
    void onBan();
    void onUnban();
    void onEditVisits();
    void onBook();

public:
    explicit ClientsPage(AppContext* contextValue, QWidget* parent = nullptr);
    void refresh() override;
    QString title() const override;
};
