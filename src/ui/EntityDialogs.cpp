#include "EntityDialogs.h"
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include "UiUtils.h"
#include "config.h"
#include "exceptions/exceptions.h"

namespace {
    QVBoxLayout* buildDialogFrame(QDialog* dialog, const QString& title, const QString& hint,
        QFormLayout** form, QDialogButtonBox** buttons, const QString& okText) {
        dialog->setWindowTitle(title);
        dialog->setMinimumWidth(440);
        QVBoxLayout* root = new QVBoxLayout(dialog);
        root->setContentsMargins(26, 24, 26, 22);
        root->setSpacing(14);
        root->addWidget(makeLabel(title, "loginTitle"));
        if (!hint.isEmpty()) {
            QLabel* hintLabel = makeLabel(hint, "muted");
            hintLabel->setWordWrap(true);
            root->addWidget(hintLabel);
        }
        *form = new QFormLayout;
        (*form)->setSpacing(10);
        (*form)->setLabelAlignment(Qt::AlignLeft);
        root->addLayout(*form);
        root->addStretch(1);
        *buttons = new QDialogButtonBox;
        QPushButton* ok = makeButton(okText, "primary");
        QPushButton* cancel = makeButton(QString::fromUtf8("Отмена"), "ghost");
        (*buttons)->addButton(ok, QDialogButtonBox::AcceptRole);
        (*buttons)->addButton(cancel, QDialogButtonBox::RejectRole);
        root->addWidget(*buttons);
        QObject::connect(*buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
        QObject::connect(*buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
        return root;
    }

    QLabel* fieldLabel(const QString& text) {
        return makeLabel(text.toUpper(), "fieldLabel");
    }
}

ClientDialog::ClientDialog(Barbershop& shopValue, QWidget* parent)
    : QDialog(parent), shop(shopValue) {
    QFormLayout* form = nullptr;
    QDialogButtonBox* buttons = nullptr;
    buildDialogFrame(this, QString::fromUtf8("Новый клиент"),
        QString::fromUtf8("Клиент будет добавлен в базу, и на него можно сразу оформить запись."),
        &form, &buttons, QString::fromUtf8("Добавить"));
    nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText(QString::fromUtf8("Например, Игорь Петров"));
    phoneEdit = new QLineEdit;
    phoneEdit->setPlaceholderText("+375291234567");
    form->addRow(fieldLabel(QString::fromUtf8("Имя")), nameEdit);
    form->addRow(fieldLabel(QString::fromUtf8("Телефон")), phoneEdit);
}

std::shared_ptr<Client> ClientDialog::result() const { return created; }

void ClientDialog::accept() {
    try {
        std::shared_ptr<Client> client = std::make_shared<Client>(ss(nameEdit->text().trimmed()),
            ss(phoneEdit->text().trimmed()));
        shop += client;
        created = client;
        QDialog::accept();
    }
    catch (...) {
        showError(this, shop);
    }
}

MasterDialog::MasterDialog(Barbershop& shopValue, QWidget* parent)
    : QDialog(parent), shop(shopValue) {
    QFormLayout* form = nullptr;
    QDialogButtonBox* buttons = nullptr;
    buildDialogFrame(this, QString::fromUtf8("Новый мастер"),
        QString::fromUtf8("После добавления назначьте мастеру услуги и график на странице «Мастера и график»."),
        &form, &buttons, QString::fromUtf8("Добавить"));
    nameEdit = new QLineEdit;
    phoneEdit = new QLineEdit;
    phoneEdit->setPlaceholderText("+375291234567");
    loginEdit = new QLineEdit;
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText(QString::fromUtf8("Не короче %1 символов").arg(MIN_PASSWORD_LENGTH));
    experienceSpin = new QSpinBox;
    experienceSpin->setRange(0, 60);
    experienceSpin->setSuffix(QString::fromUtf8(" лет"));
    ratingSpin = new QDoubleSpinBox;
    ratingSpin->setRange(0.0, 5.0);
    ratingSpin->setDecimals(1);
    ratingSpin->setSingleStep(0.1);
    ratingSpin->setValue(4.5);
    form->addRow(fieldLabel(QString::fromUtf8("Имя")), nameEdit);
    form->addRow(fieldLabel(QString::fromUtf8("Телефон")), phoneEdit);
    form->addRow(fieldLabel(QString::fromUtf8("Логин")), loginEdit);
    form->addRow(fieldLabel(QString::fromUtf8("Пароль")), passwordEdit);
    form->addRow(fieldLabel(QString::fromUtf8("Опыт")), experienceSpin);
    form->addRow(fieldLabel(QString::fromUtf8("Рейтинг")), ratingSpin);
}

void MasterDialog::accept() {
    try {
        std::shared_ptr<Master> master = std::make_shared<Master>(ss(nameEdit->text().trimmed()),
            ss(phoneEdit->text().trimmed()), ss(loginEdit->text().trimmed()), ss(passwordEdit->text()),
            experienceSpin->value(), ratingSpin->value());
        shop += master;
        QDialog::accept();
    }
    catch (...) {
        showError(this, shop);
    }
}

AdminDialog::AdminDialog(Barbershop& shopValue, QWidget* parent)
    : QDialog(parent), shop(shopValue) {
    QFormLayout* form = nullptr;
    QDialogButtonBox* buttons = nullptr;
    buildDialogFrame(this, QString::fromUtf8("Новый администратор"), QString(), &form, &buttons,
        QString::fromUtf8("Добавить"));
    nameEdit = new QLineEdit;
    phoneEdit = new QLineEdit;
    phoneEdit->setPlaceholderText("+375291234567");
    loginEdit = new QLineEdit;
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText(QString::fromUtf8("Не короче %1 символов").arg(MIN_PASSWORD_LENGTH));
    positionEdit = new QLineEdit;
    positionEdit->setPlaceholderText(QString::fromUtf8("Например, Менеджер"));
    form->addRow(fieldLabel(QString::fromUtf8("Имя")), nameEdit);
    form->addRow(fieldLabel(QString::fromUtf8("Телефон")), phoneEdit);
    form->addRow(fieldLabel(QString::fromUtf8("Логин")), loginEdit);
    form->addRow(fieldLabel(QString::fromUtf8("Пароль")), passwordEdit);
    form->addRow(fieldLabel(QString::fromUtf8("Должность")), positionEdit);
}

void AdminDialog::accept() {
    try {
        std::shared_ptr<Administrator> admin = std::make_shared<Administrator>(ss(nameEdit->text().trimmed()),
            ss(phoneEdit->text().trimmed()), ss(loginEdit->text().trimmed()), ss(passwordEdit->text()),
            ss(positionEdit->text().trimmed()));
        shop += admin;
        QDialog::accept();
    }
    catch (...) {
        showError(this, shop);
    }
}

ServiceDialog::ServiceDialog(Barbershop& shopValue, QWidget* parent)
    : QDialog(parent), shop(shopValue) {
    QFormLayout* form = nullptr;
    QDialogButtonBox* buttons = nullptr;
    buildDialogFrame(this, QString::fromUtf8("Новая услуга"),
        QString::fromUtf8("Базовая цена умножается на коэффициент уровня мастера, если мастер не задал свою."),
        &form, &buttons, QString::fromUtf8("Добавить"));
    nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText(QString::fromUtf8("Например, Камуфляж бороды"));
    priceSpin = new QDoubleSpinBox;
    priceSpin->setRange(0.0, 10000.0);
    priceSpin->setDecimals(2);
    priceSpin->setSingleStep(1.0);
    priceSpin->setSuffix(" BYN");
    priceSpin->setValue(20.0);
    lengthSpin = new QSpinBox;
    lengthSpin->setRange(5, MAX_SERVICE_MINUTES);
    lengthSpin->setSingleStep(5);
    lengthSpin->setSuffix(QString::fromUtf8(" мин"));
    lengthSpin->setValue(30);
    form->addRow(fieldLabel(QString::fromUtf8("Название")), nameEdit);
    form->addRow(fieldLabel(QString::fromUtf8("Базовая цена")), priceSpin);
    form->addRow(fieldLabel(QString::fromUtf8("Длительность")), lengthSpin);
}

void ServiceDialog::accept() {
    try {
        std::shared_ptr<Service> service = std::make_shared<Service>(ss(nameEdit->text().trimmed()),
            priceSpin->value(), lengthSpin->value());
        shop += service;
        QDialog::accept();
    }
    catch (...) {
        showError(this, shop);
    }
}

OfferDialog::OfferDialog(AppContext* contextValue, const std::shared_ptr<Master>& masterValue,
    const std::shared_ptr<Service>& preset, QWidget* parent)
    : QDialog(parent), context(contextValue), master(masterValue) {
    QFormLayout* form = nullptr;
    QDialogButtonBox* buttons = nullptr;
    buildDialogFrame(this, QString::fromUtf8("Услуга мастера"),
        QString::fromUtf8("Цена и длительность «Авто» означают: базовая цена × коэффициент уровня и стандартная длительность."),
        &form, &buttons, QString::fromUtf8("Сохранить"));
    serviceCombo = new QComboBox;
    for (const auto& service : context->shop().getServices()) {
        serviceCombo->addItem(qs(service->getName()) + "  ·  " + money(service->getPrice()), service->getId());
        if (preset && service->getId() == preset->getId()) serviceCombo->setCurrentIndex(serviceCombo->count() - 1);
    }
    priceSpin = new QDoubleSpinBox;
    priceSpin->setRange(0.0, 10000.0);
    priceSpin->setDecimals(2);
    priceSpin->setSuffix(" BYN");
    priceSpin->setSpecialValueText(QString::fromUtf8("Авто"));
    durationSpin = new QSpinBox;
    durationSpin->setRange(0, MAX_SERVICE_MINUTES);
    durationSpin->setSingleStep(5);
    durationSpin->setSuffix(QString::fromUtf8(" мин"));
    durationSpin->setSpecialValueText(QString::fromUtf8("Стандартная"));
    if (preset) {
        std::shared_ptr<MasterService> offer = master->findOffer(preset->getId());
        if (offer) {
            priceSpin->setValue(offer->getCustomPrice());
            durationSpin->setValue(offer->getCustomDuration());
        }
    }
    form->addRow(fieldLabel(QString::fromUtf8("Услуга")), serviceCombo);
    form->addRow(fieldLabel(QString::fromUtf8("Цена мастера")), priceSpin);
    form->addRow(fieldLabel(QString::fromUtf8("Длительность")), durationSpin);
}

void OfferDialog::accept() {
    try {
        std::shared_ptr<Service> service = findServiceById(context->shop(), serviceCombo->currentData().toInt());
        context->shop().setOffer(master, service, priceSpin->value(), durationSpin->value(), context->user());
        QDialog::accept();
    }
    catch (const InvalidDataException&) {
        showError(this, context->shop());
    }
    catch (...) {
        showError(this, context->shop());
        QDialog::reject();
    }
}
