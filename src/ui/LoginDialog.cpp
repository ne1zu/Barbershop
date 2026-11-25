#include "LoginDialog.h"
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include "AppContext.h"
#include "UiUtils.h"
#include "exceptions/exceptions.h"

namespace {
    QWidget* labeled(const QString& title, QLineEdit* edit) {
        QWidget* box = new QWidget;
        QVBoxLayout* layout = new QVBoxLayout(box);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(6);
        layout->addWidget(makeLabel(title.toUpper(), "fieldLabel"));
        layout->addWidget(edit);
        return box;
    }

    QPushButton* demoButton(const QString& text, int tab, const QString& login, const QString& password) {
        QPushButton* button = makeButton(text, "ghost");
        button->setProperty("demoTab", tab);
        button->setProperty("demoLogin", login);
        button->setProperty("demoPassword", password);
        return button;
    }
}

LoginDialog::LoginDialog(Barbershop& shopValue, QWidget* parent) : QDialog(parent), shop(shopValue) {
    setWindowTitle(QString::fromUtf8("Вход — ") + qs(shop.getName()));
    setFixedSize(1080, 700);

    QHBoxLayout* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    QFrame* hero = new QFrame;
    hero->setObjectName("heroPanel");
    hero->setFixedWidth(340);
    QVBoxLayout* heroLayout = new QVBoxLayout(hero);
    heroLayout->setContentsMargins(36, 44, 36, 36);
    heroLayout->setSpacing(10);
    QLabel* logo = makeLabel(QString::fromUtf8("ОЛ"), "heroLogo");
    logo->setFixedSize(64, 64);
    logo->setAlignment(Qt::AlignCenter);
    heroLayout->addWidget(logo);
    heroLayout->addSpacing(14);
    QLabel* title = makeLabel(qs(shop.getName()), "heroTitle");
    title->setWordWrap(true);
    heroLayout->addWidget(title);
    QLabel* address = makeLabel(qs(shop.getAddress()), "heroText");
    heroLayout->addWidget(address);
    heroLayout->addSpacing(22);
    QLabel* about = makeLabel(QString::fromUtf8(
        "Онлайн-запись, журнал мастеров, умный подбор времени и учёт клиентов — всё в одном месте."), "heroText");
    about->setWordWrap(true);
    heroLayout->addWidget(about);
    heroLayout->addStretch(1);
    heroLayout->addWidget(makeLabel(QString::fromUtf8("Премиальный груминг · CRM"), "mutedSmall"));
    root->addWidget(hero);

    QWidget* right = new QWidget;
    QVBoxLayout* rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(40, 36, 40, 28);
    rightLayout->setSpacing(12);
    rightLayout->addWidget(makeLabel(QString::fromUtf8("Добро пожаловать"), "loginTitle"));
    rightLayout->addWidget(makeLabel(QString::fromUtf8("Войдите как клиент или сотрудник барбершопа."), "muted"));

    tabs = new QTabWidget;
    tabs->addTab(buildClientTab(), QString::fromUtf8("Клиент"));
    tabs->addTab(buildRegisterTab(), QString::fromUtf8("Регистрация"));
    tabs->addTab(buildStaffTab(), QString::fromUtf8("Персонал"));
    rightLayout->addWidget(tabs);
    rightLayout->addStretch(1);
    rightLayout->addWidget(buildDemoCard());
    root->addWidget(right, 1);
}

std::shared_ptr<Person> LoginDialog::result() const { return person; }

QWidget* LoginDialog::buildClientTab() {
    QWidget* page = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 20, 0, 0);
    layout->setSpacing(14);
    clientPhone = new QLineEdit;
    clientPhone->setPlaceholderText("+375291234567");
    layout->addWidget(labeled(QString::fromUtf8("Телефон"), clientPhone));
    QPushButton* button = makeButton(QString::fromUtf8("Войти"), "primary");
    button->setProperty("kind", "primary");
    button->setMinimumHeight(44);
    layout->addWidget(button);
    layout->addStretch(1);
    connect(button, &QPushButton::clicked, this, &LoginDialog::onClientLogin);
    connect(clientPhone, &QLineEdit::returnPressed, this, &LoginDialog::onClientLogin);
    return page;
}

QWidget* LoginDialog::buildRegisterTab() {
    QWidget* page = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 20, 0, 0);
    layout->setSpacing(14);
    registerName = new QLineEdit;
    registerName->setPlaceholderText(QString::fromUtf8("Имя и фамилия"));
    registerPhone = new QLineEdit;
    registerPhone->setPlaceholderText("+375291234567");
    layout->addWidget(labeled(QString::fromUtf8("Имя"), registerName));
    layout->addWidget(labeled(QString::fromUtf8("Телефон"), registerPhone));
    QPushButton* button = makeButton(QString::fromUtf8("Зарегистрироваться"), "primary");
    button->setMinimumHeight(44);
    layout->addWidget(button);
    layout->addStretch(1);
    connect(button, &QPushButton::clicked, this, &LoginDialog::onRegister);
    connect(registerPhone, &QLineEdit::returnPressed, this, &LoginDialog::onRegister);
    return page;
}

QWidget* LoginDialog::buildStaffTab() {
    QWidget* page = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 20, 0, 0);
    layout->setSpacing(14);
    staffLogin = new QLineEdit;
    staffLogin->setPlaceholderText(QString::fromUtf8("Логин"));
    staffPassword = new QLineEdit;
    staffPassword->setEchoMode(QLineEdit::Password);
    staffPassword->setPlaceholderText(QString::fromUtf8("Пароль"));
    layout->addWidget(labeled(QString::fromUtf8("Логин"), staffLogin));
    layout->addWidget(labeled(QString::fromUtf8("Пароль"), staffPassword));
    QPushButton* button = makeButton(QString::fromUtf8("Войти"), "primary");
    button->setMinimumHeight(44);
    layout->addWidget(button);
    layout->addStretch(1);
    connect(button, &QPushButton::clicked, this, &LoginDialog::onStaffLogin);
    connect(staffPassword, &QLineEdit::returnPressed, this, &LoginDialog::onStaffLogin);
    return page;
}

QWidget* LoginDialog::buildDemoCard() {
    QFrame* card = makeCard();
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 14, 18, 14);
    layout->setSpacing(8);
    layout->addWidget(makeLabel(QString::fromUtf8("ДЕМО-ДАННЫЕ ДЛЯ БЫСТРОГО ВХОДА"), "statTitle"));
    QLabel* hint = makeLabel(QString::fromUtf8(
        "Нажмите на кнопку — вход выполнится сразу. Пароль мастера: логин + 123 "
        "(alex123, ivan123, max12345 …). Клиенты: +375291110001 (с историей), …0003 (заблокирован), …0004 (новый)."),
        "mutedSmall");
    hint->setWordWrap(true);
    layout->addWidget(hint);

    struct DemoEntry { const char* title; int tab; const char* login; const char* password; };
    const DemoEntry entries[] = {
        { "Админ Ольга", 2, "admin", "admin123" },
        { "Мастер Алексей", 2, "alex", "alex123" },
        { "Мастер Иван", 2, "ivan", "ivan123" },
        { "Мастер Максим", 2, "max", "max12345" },
        { "Мастер Артём", 2, "artem", "artem123" },
        { "Мастер Денис", 2, "denis", "denis123" },
        { "Мастер Никита", 2, "nikita", "nikita123" },
        { "Мастер Роман", 2, "roman", "roman123" },
        { "Мастер Кирилл", 2, "kirill", "kirill123" },
        { "Клиент Дмитрий", 0, "+375291110001", "" },
        { "Клиент Олег", 0, "+375291110003", "" },
        { "Клиент Павел", 0, "+375291110004", "" }
    };
    const int entryCount = 12;
    QGridLayout* buttons = new QGridLayout;
    buttons->setSpacing(8);
    for (int i = 0; i < entryCount; ++i) {
        QPushButton* button = demoButton(QString::fromUtf8(entries[i].title), entries[i].tab,
            entries[i].login, entries[i].password);
        buttons->addWidget(button, i / 4, i % 4);
        connect(button, &QPushButton::clicked, this, &LoginDialog::onDemo);
    }
    layout->addLayout(buttons);
    return card;
}

void LoginDialog::onDemo() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button == nullptr) return;
    int tab = button->property("demoTab").toInt();
    tabs->setCurrentIndex(tab);
    if (tab == 0) {
        clientPhone->setText(button->property("demoLogin").toString());
        onClientLogin();
    }
    else {
        staffLogin->setText(button->property("demoLogin").toString());
        staffPassword->setText(button->property("demoPassword").toString());
        onStaffLogin();
    }
}

void LoginDialog::onClientLogin() {
    try {
        person = shop.loginClient(ss(clientPhone->text().trimmed()));
        accept();
    }
    catch (...) {
        showError(this, shop);
    }
}

void LoginDialog::onRegister() {
    try {
        std::shared_ptr<Client> client = std::make_shared<Client>(ss(registerName->text().trimmed()),
            ss(registerPhone->text().trimmed()));
        shop += client;
        person = client;
        accept();
    }
    catch (...) {
        showError(this, shop);
    }
}

void LoginDialog::onStaffLogin() {
    try {
        std::shared_ptr<Person> staff = shop.loginStaff(ss(staffLogin->text().trimmed()), ss(staffPassword->text()));
        person = staff;
        accept();
    }
    catch (...) {
        showError(this, shop);
    }
}
