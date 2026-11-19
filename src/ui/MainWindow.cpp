#include "MainWindow.h"
#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include "AdminPages.h"
#include "AppointmentsPage.h"
#include "BookingPanel.h"
#include "CatalogPage.h"
#include "ClientsPage.h"
#include "MasterSettingsPage.h"
#include "MyAppointmentsPage.h"
#include "PageBase.h"
#include "ProfilePage.h"
#include "SchedulePage.h"
#include "StatsPage.h"
#include "UiUtils.h"
#include "config.h"

MainWindow::MainWindow(AppContext* contextValue, QWidget* parent)
    : QMainWindow(parent), context(contextValue), bookingPage(nullptr), bookingIndex(-1), logout(false) {
    setWindowTitle(qs(context->shop().getName()) + QString::fromUtf8(" — CRM"));
    resize(1420, 860);
    setMinimumSize(1180, 720);

    QWidget* central = new QWidget;
    setCentralWidget(central);
    QHBoxLayout* root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    QFrame* sidebar = new QFrame;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(250);
    QVBoxLayout* side = new QVBoxLayout(sidebar);
    side->setContentsMargins(16, 20, 16, 16);
    side->setSpacing(6);

    QHBoxLayout* brand = new QHBoxLayout;
    brand->setSpacing(12);
    QLabel* logo = makeLabel(QString::fromUtf8("✂"), "brandLogo");
    logo->setFixedSize(40, 40);
    logo->setAlignment(Qt::AlignCenter);
    brand->addWidget(logo);
    QVBoxLayout* brandText = new QVBoxLayout;
    brandText->setSpacing(0);
    brandText->addWidget(makeLabel("CIryulnya", "brandTitle"));
    brandText->addWidget(makeLabel("Premium grooming", "brandSub"));
    brand->addLayout(brandText, 1);
    side->addLayout(brand);
    side->addSpacing(16);

    QPushButton* newBooking = makeButton(QString::fromUtf8("+  НОВАЯ ЗАПИСЬ"), "primary");
    newBooking->setMinimumHeight(42);
    side->addWidget(newBooking);
    side->addSpacing(10);

    navLayout = new QVBoxLayout;
    navLayout->setSpacing(4);
    side->addLayout(navLayout);
    side->addStretch(1);

    QFrame* userCard = new QFrame;
    userCard->setObjectName("userCard");
    QHBoxLayout* userLayout = new QHBoxLayout(userCard);
    userLayout->setContentsMargins(12, 10, 12, 10);
    userLayout->setSpacing(10);
    avatar = makeLabel(QString(), "avatar");
    avatar->setFixedSize(40, 40);
    avatar->setAlignment(Qt::AlignCenter);
    userLayout->addWidget(avatar);
    QVBoxLayout* userText = new QVBoxLayout;
    userText->setSpacing(0);
    userName = makeLabel(QString(), "userName");
    userRole = makeLabel(QString(), "userRole");
    userText->addWidget(userName);
    userText->addWidget(userRole);
    userLayout->addLayout(userText, 1);
    side->addWidget(userCard);
    QPushButton* logoutButton = makeButton(QString::fromUtf8("Выйти"), "ghost");
    side->addWidget(logoutButton);
    root->addWidget(sidebar);

    QVBoxLayout* mainColumn = new QVBoxLayout;
    mainColumn->setContentsMargins(0, 0, 0, 0);
    mainColumn->setSpacing(0);
    QFrame* topbar = new QFrame;
    topbar->setObjectName("topbar");
    topbar->setFixedHeight(68);
    QHBoxLayout* topLayout = new QHBoxLayout(topbar);
    topLayout->setContentsMargins(28, 0, 28, 0);
    titleLabel = makeLabel(QString(), "pageTitle");
    topLayout->addWidget(titleLabel);
    topLayout->addStretch(1);
    topLayout->addWidget(makeLabel(fullDayTitle(DateTime::now()), "pageDate"));
    mainColumn->addWidget(topbar);
    stack = new QStackedWidget;
    mainColumn->addWidget(stack, 1);
    root->addLayout(mainColumn, 1);

    navGroup = new QButtonGroup(this);
    navGroup->setExclusive(true);
    buildPages();

    connect(navGroup, &QButtonGroup::idClicked, this, &MainWindow::onNavClicked);
    connect(newBooking, &QPushButton::clicked, this, &MainWindow::onNewBooking);
    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::onLogout);

    refreshUser();
    if (navGroup->button(0) != nullptr) navGroup->button(0)->setChecked(true);
    showPage(0);
}

bool MainWindow::logoutRequested() const { return logout; }

void MainWindow::addPage(PageBase* page, const QString& iconName) {
    int index = static_cast<int>(pages.size());
    pages.push_back(page);
    stack->addWidget(page);
    QPushButton* button = new QPushButton("  " + page->title());
    button->setObjectName("navButton");
    button->setCheckable(true);
    button->setIcon(makeIcon(iconName));
    button->setIconSize(QSize(20, 20));
    button->setCursor(Qt::PointingHandCursor);
    navLayout->addWidget(button);
    navGroup->addButton(button, index);
    connect(page, &PageBase::dataChanged, this, &MainWindow::onDataChanged);
}

void MainWindow::buildPages() {
    if (context->isClient()) {
        CatalogPage* catalog = new CatalogPage(context);
        addPage(catalog, "book");
        connect(catalog, &CatalogPage::bookWithMaster, this, &MainWindow::onBookWithMaster);
        bookingPage = new BookingPage(context);
        bookingIndex = static_cast<int>(pages.size());
        addPage(bookingPage, "calendar");
        addPage(new MyAppointmentsPage(context), "list");
        addPage(new ProfilePage(context), "user");
        return;
    }
    addPage(new SchedulePage(context), "calendar");
    addPage(new AppointmentsPage(context), "list");
    addPage(new ClientsPage(context), "users");
    if (context->isAdmin()) {
        addPage(new StaffPage(context), "user");
        addPage(new ServicesCatalogPage(context), "scissors");
        addPage(new MasterSettingsPage(context), "clock");
        addPage(new PaymentsPage(context), "cash");
        addPage(new StatsPage(context), "chart");
        DataPage* data = new DataPage(context);
        addPage(data, "database");
        connect(data, &DataPage::reloginRequired, this, &MainWindow::onLogout);
    }
    else {
        addPage(new MasterSettingsPage(context), "clock");
        addPage(new StatsPage(context), "chart");
    }
}

void MainWindow::showPage(int index) {
    if (index < 0 || index >= static_cast<int>(pages.size())) return;
    pages[index]->refresh();
    stack->setCurrentIndex(index);
    titleLabel->setText(pages[index]->title());
}

void MainWindow::refreshUser() {
    std::shared_ptr<Person> user = context->user();
    avatar->setText(initials(user->getName()));
    userName->setText(qs(user->getName()));
    QString role = context->roleTitle();
    if (context->isClient()) {
        role += QString::fromUtf8(" · скидка %1%").arg(context->client()->getDiscountPercent());
    }
    else if (context->isMaster()) {
        role += " · " + levelText(context->master()->classify());
    }
    else {
        role += " · " + qs(context->admin()->getPosition());
    }
    userRole->setText(role);
}

void MainWindow::onNavClicked(int index) { showPage(index); }

void MainWindow::onNewBooking() {
    if (context->isClient()) {
        if (navGroup->button(bookingIndex) != nullptr) navGroup->button(bookingIndex)->setChecked(true);
        showPage(bookingIndex);
        return;
    }
    NewBookingDialog dialog(context, this);
    if (dialog.exec() == QDialog::Accepted) {
        showPage(stack->currentIndex());
        refreshUser();
    }
}

void MainWindow::onDataChanged() { refreshUser(); }

void MainWindow::onLogout() {
    logout = true;
    close();
}

void MainWindow::onBookWithMaster(int masterId) {
    if (bookingPage == nullptr) return;
    if (navGroup->button(bookingIndex) != nullptr) navGroup->button(bookingIndex)->setChecked(true);
    showPage(bookingIndex);
    bookingPage->selectMaster(masterId);
}
