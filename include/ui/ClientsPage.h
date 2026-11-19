#pragma once
#include "PageBase.h"

class QLabel;
class QLineEdit;
class QListWidget;
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
    QVBoxLayout* notesLayout;
    QLineEdit* noteEdit;
    QPushButton* banButton;
    QPushButton* unbanButton;
    QTableWidget* history;
    bool loading;

    std::shared_ptr<Client> selected() const;
    void fillList();
    void showDetails();

private slots:
    void onSearchChanged();
    void onSelectionChanged();
    void onNewClient();
    void onAddNote();
    void onBan();
    void onUnban();
    void onBook();

public:
    explicit ClientsPage(AppContext* contextValue, QWidget* parent = nullptr);
    void refresh() override;
    QString title() const override;
};
