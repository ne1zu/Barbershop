#pragma once
#include <memory>
#include <QDialog>
#include "barbershop.h"
#include "person.h"

class QLineEdit;
class QTabWidget;

class LoginDialog : public QDialog {
    Q_OBJECT

private:
    Barbershop& shop;
    QTabWidget* tabs;
    QLineEdit* clientPhone;
    QLineEdit* registerName;
    QLineEdit* registerPhone;
    QLineEdit* staffLogin;
    QLineEdit* staffPassword;
    std::shared_ptr<Person> person;

    QWidget* buildClientTab();
    QWidget* buildRegisterTab();
    QWidget* buildStaffTab();
    QWidget* buildDemoCard();

private slots:
    void onClientLogin();
    void onRegister();
    void onStaffLogin();
    void onDemo();

public:
    explicit LoginDialog(Barbershop& shopValue, QWidget* parent = nullptr);
    std::shared_ptr<Person> result() const;
};
