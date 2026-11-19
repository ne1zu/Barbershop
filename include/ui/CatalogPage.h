#pragma once
#include "PageBase.h"

class QButtonGroup;
class QVBoxLayout;

class CatalogPage : public PageBase {
    Q_OBJECT

private:
    QVBoxLayout* listLayout;
    QButtonGroup* bookGroup;

private slots:
    void onBookClicked(int masterId);

public:
    explicit CatalogPage(AppContext* contextValue, QWidget* parent = nullptr);
    void refresh() override;
    QString title() const override;

signals:
    void bookWithMaster(int masterId);
};
