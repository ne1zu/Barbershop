#pragma once
#include <vector>
#include <QMainWindow>
#include "AppContext.h"

class BookingPage;
class PageBase;
class QButtonGroup;
class QLabel;
class QStackedWidget;
class QVBoxLayout;

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    AppContext* context;
    QStackedWidget* stack;
    QButtonGroup* navGroup;
    QVBoxLayout* navLayout;
    std::vector<PageBase*> pages;
    BookingPage* bookingPage;
    int bookingIndex;
    QLabel* titleLabel;
    QLabel* userName;
    QLabel* userRole;
    QLabel* avatar;
    bool logout;

    void addPage(PageBase* page, const QString& iconName);
    void buildPages();
    void showPage(int index);
    void refreshUser();

private slots:
    void onNavClicked(int index);
    void onNewBooking();
    void onDataChanged();
    void onLogout();
    void onBookWithMaster(int masterId);

public:
    explicit MainWindow(AppContext* contextValue, QWidget* parent = nullptr);
    bool logoutRequested() const;
};
