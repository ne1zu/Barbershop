#pragma once
#include <memory>
#include <QDialog>
#include "AppContext.h"
#include "appointment.h"

class QComboBox;
class QDateEdit;
class QPushButton;

class RescheduleDialog : public QDialog {
    Q_OBJECT

private:
    AppContext* context;
    std::shared_ptr<Appointment> appointment;
    QDateEdit* dateEdit;
    QComboBox* timeCombo;
    QPushButton* okButton;

    void fillTimes();

private slots:
    void onDateChanged();

public:
    RescheduleDialog(AppContext* contextValue, const std::shared_ptr<Appointment>& appointmentValue,
        QWidget* parent);
    void accept() override;
};
