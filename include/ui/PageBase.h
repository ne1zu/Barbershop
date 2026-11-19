#pragma once
#include <QString>
#include <QWidget>
#include "AppContext.h"

class PageBase : public QWidget {
    Q_OBJECT

private:
    AppContext* context;

public:
    explicit PageBase(AppContext* contextValue, QWidget* parent = nullptr)
        : QWidget(parent), context(contextValue) {
    }

    virtual void refresh() = 0;
    virtual QString title() const = 0;

signals:
    void dataChanged();

protected:
    AppContext* ctx() const { return context; }
    Barbershop& shop() const { return context->shop(); }
};
