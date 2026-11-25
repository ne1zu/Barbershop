#pragma once
#include <memory>
#include <QDialog>
#include "AppContext.h"

class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QSpinBox;

class ClientDialog : public QDialog {
private:
    Barbershop& shop;
    QLineEdit* nameEdit;
    QLineEdit* phoneEdit;
    std::shared_ptr<Client> created;

public:
    ClientDialog(Barbershop& shopValue, QWidget* parent);
    std::shared_ptr<Client> result() const;
    void accept() override;
};

class MasterDialog : public QDialog {
private:
    Barbershop& shop;
    QLineEdit* nameEdit;
    QLineEdit* phoneEdit;
    QLineEdit* loginEdit;
    QLineEdit* passwordEdit;
    QSpinBox* experienceSpin;
    QDoubleSpinBox* ratingSpin;

public:
    MasterDialog(Barbershop& shopValue, QWidget* parent);
    void accept() override;
};

class AdminDialog : public QDialog {
private:
    Barbershop& shop;
    QLineEdit* nameEdit;
    QLineEdit* phoneEdit;
    QLineEdit* loginEdit;
    QLineEdit* passwordEdit;
    QLineEdit* positionEdit;

public:
    AdminDialog(Barbershop& shopValue, QWidget* parent);
    void accept() override;
};

class ServiceDialog : public QDialog {
private:
    Barbershop& shop;
    QLineEdit* nameEdit;
    QDoubleSpinBox* priceSpin;
    QSpinBox* lengthSpin;
    std::shared_ptr<Service> editing;

    void buildFields(const QString& title, const QString& hint, const QString& okText);

public:
    ServiceDialog(Barbershop& shopValue, QWidget* parent);
    ServiceDialog(Barbershop& shopValue, const std::shared_ptr<Service>& existing, QWidget* parent);
    void accept() override;
};

class MasterProfileDialog : public QDialog {
private:
    Barbershop& shop;
    std::shared_ptr<Master> master;
    QSpinBox* experienceSpin;
    QDoubleSpinBox* ratingSpin;

public:
    MasterProfileDialog(Barbershop& shopValue, const std::shared_ptr<Master>& masterValue, QWidget* parent);
    void accept() override;
};

class OfferDialog : public QDialog {
private:
    AppContext* context;
    std::shared_ptr<Master> master;
    QComboBox* serviceCombo;
    QDoubleSpinBox* priceSpin;
    QSpinBox* durationSpin;

public:
    OfferDialog(AppContext* contextValue, const std::shared_ptr<Master>& masterValue,
        const std::shared_ptr<Service>& preset, QWidget* parent);
    void accept() override;
};
