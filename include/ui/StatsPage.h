#pragma once
#include "PageBase.h"

class QVBoxLayout;

class StatsPage : public PageBase {
    Q_OBJECT

private:
    QVBoxLayout* content;

    void buildAdmin();
    void buildMaster();

public:
    explicit StatsPage(AppContext* contextValue, QWidget* parent = nullptr);
    void refresh() override;
    QString title() const override;
};
