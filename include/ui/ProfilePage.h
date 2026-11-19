#pragma once
#include "PageBase.h"

class QLabel;
class QProgressBar;
class QTableWidget;

class ProfilePage : public PageBase {
    Q_OBJECT

private:
    QLabel* avatar;
    QLabel* nameLabel;
    QLabel* phoneLabel;
    QLabel* levelBadge;
    QLabel* bannerLabel;
    QLabel* visitsValue;
    QLabel* spentValue;
    QLabel* discountValue;
    QLabel* progressText;
    QProgressBar* progress;
    QTableWidget* history;

public:
    explicit ProfilePage(AppContext* contextValue, QWidget* parent = nullptr);
    void refresh() override;
    QString title() const override;
};
