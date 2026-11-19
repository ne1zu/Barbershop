#pragma once
#include <memory>
#include <vector>
#include <QDate>
#include <QDialog>
#include <QWidget>
#include "AppContext.h"
#include "PageBase.h"
#include "slot_finder.h"

class QButtonGroup;
class QComboBox;
class QDateEdit;
class QGridLayout;
class QLabel;
class QPushButton;
class QSpinBox;
class QTabWidget;
class QVBoxLayout;

class BookingPanel : public QWidget {
    Q_OBJECT

private:
    AppContext* context;
    bool staffMode;
    bool updating;
    int desiredMinute;

    QLabel* bannerLabel;
    QWidget* clientRow;
    QComboBox* clientCombo;
    QTabWidget* tabs;

    QComboBox* masterCombo;
    QComboBox* serviceCombo;
    QDateEdit* dateEdit;
    QLabel* infoLabel;
    QLabel* slotsHint;
    QWidget* slotsHost;
    QGridLayout* slotsLayout;
    QButtonGroup* slotGroup;
    QPushButton* bookButton;
    std::vector<DateTime> times;
    int selectedSlot;

    QComboBox* smartService;
    QComboBox* smartMaster;
    QSpinBox* daysSpin;
    QSpinBox* hourSpin;
    QVBoxLayout* resultsLayout;
    QButtonGroup* optionGroup;
    std::vector<SlotOption> options;

    Barbershop& shop() const;
    std::shared_ptr<Client> currentClient() const;
    std::shared_ptr<Master> currentMaster() const;
    std::shared_ptr<Service> currentService() const;
    QWidget* buildManualTab();
    QWidget* buildSmartTab();
    void populateClients();
    void populateMasters(QComboBox* combo, bool includeAny);
    void populateManualServices();
    void populateSmartServices();
    void populateSmartMasters();
    void updateSlots();
    void clearSlots();
    void showOptions();
    void clearOptions();
    void finishBooking(const std::shared_ptr<Appointment>& appointment);
    void selectComboById(QComboBox* combo, int id);

private slots:
    void onClientChanged();
    void onNewClient();
    void onMasterChanged();
    void onServiceChanged();
    void onDateChanged();
    void onSlotChosen(int index);
    void onBook();
    void onSmartServiceChanged();
    void onFind();
    void onOptionChosen(int index);

public:
    BookingPanel(AppContext* contextValue, bool staffModeValue, QWidget* parent = nullptr);

    void refresh();
    void presetClient(const std::shared_ptr<Client>& client);
    void presetSlot(int masterId, const QDate& date, int minuteOfDay);
    void selectMaster(int masterId);

signals:
    void booked();
};

class NewBookingDialog : public QDialog {
private:
    BookingPanel* panel;

public:
    NewBookingDialog(AppContext* contextValue, QWidget* parent);
    void presetClient(const std::shared_ptr<Client>& client);
    void presetSlot(int masterId, const QDate& date, int minuteOfDay);
};

class BookingPage : public PageBase {
    Q_OBJECT

private:
    BookingPanel* panel;

private slots:
    void onBooked();

public:
    explicit BookingPage(AppContext* contextValue, QWidget* parent = nullptr);
    void refresh() override;
    QString title() const override;
    void selectMaster(int masterId);
};
