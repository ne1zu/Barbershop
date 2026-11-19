#pragma once
#include <memory>
#include <QString>
#include "administrator.h"
#include "barbershop.h"
#include "client.h"
#include "master.h"
#include "person.h"

class QWidget;

enum class Role { Client, Master, Admin };

class AppContext {
private:
    Barbershop& shopRef;
    std::shared_ptr<Person> currentUser;
    Role currentRole;

public:
    AppContext(Barbershop& shopValue, std::shared_ptr<Person> user);

    Barbershop& shop() const;
    std::shared_ptr<Person> user() const;
    Role role() const;

    bool isClient() const;
    bool isMaster() const;
    bool isAdmin() const;
    bool isStaff() const;

    std::shared_ptr<Client> client() const;
    std::shared_ptr<Master> master() const;
    std::shared_ptr<Administrator> admin() const;

    QString roleTitle() const;
};

void showError(QWidget* parent, const Barbershop& shop);
void showInfo(QWidget* parent, const QString& title, const QString& text);
bool askConfirm(QWidget* parent, const QString& title, const QString& text, const QString& yesText = QString());
