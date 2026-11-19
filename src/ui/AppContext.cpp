#include "AppContext.h"
#include <QDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "UiUtils.h"
#include "exceptions/exceptions.h"

AppContext::AppContext(Barbershop& shopValue, std::shared_ptr<Person> user)
    : shopRef(shopValue), currentUser(std::move(user)), currentRole(Role::Client) {
    if (std::dynamic_pointer_cast<Master>(currentUser)) {
        currentRole = Role::Master;
    }
    else if (std::dynamic_pointer_cast<Administrator>(currentUser)) {
        currentRole = Role::Admin;
    }
    else {
        currentRole = Role::Client;
    }
}

Barbershop& AppContext::shop() const { return shopRef; }
std::shared_ptr<Person> AppContext::user() const { return currentUser; }
Role AppContext::role() const { return currentRole; }

bool AppContext::isClient() const { return currentRole == Role::Client; }
bool AppContext::isMaster() const { return currentRole == Role::Master; }
bool AppContext::isAdmin() const { return currentRole == Role::Admin; }
bool AppContext::isStaff() const { return currentRole != Role::Client; }

std::shared_ptr<Client> AppContext::client() const { return std::dynamic_pointer_cast<Client>(currentUser); }
std::shared_ptr<Master> AppContext::master() const { return std::dynamic_pointer_cast<Master>(currentUser); }
std::shared_ptr<Administrator> AppContext::admin() const {
    return std::dynamic_pointer_cast<Administrator>(currentUser);
}

QString AppContext::roleTitle() const {
    if (currentRole == Role::Master) return QString::fromUtf8("Мастер");
    if (currentRole == Role::Admin) return QString::fromUtf8("Администратор");
    return QString::fromUtf8("Клиент");
}

namespace {
    enum class PopupKind { Info, Warning, Error, Question };

    QLabel* makePopupIcon(PopupKind kind) {
        QString symbol = QString::fromUtf8("✓");
        QString style = "background: #142a21; color: #46c28e; border: 1px solid #1f4634; "
            "border-radius: 18px; font-size: 16px; font-weight: 700;";

        if (kind == PopupKind::Warning) {
            symbol = "!";
            style = "background: #2e2716; color: #e0b44c; border: 1px solid #4a3b1c; "
                "border-radius: 18px; font-size: 17px; font-weight: 700;";
        }
        else if (kind == PopupKind::Error) {
            symbol = QString::fromUtf8("✕");
            style = "background: #3a1a1d; color: #ef6b70; border: 1px solid #5c282d; "
                "border-radius: 18px; font-size: 15px; font-weight: 700;";
        }
        else if (kind == PopupKind::Question) {
            symbol = "?";
            style = "background: #33281a; color: #e6c9a2; border: 1px solid #4d3c25; "
                "border-radius: 18px; font-size: 16px; font-weight: 700;";
        }

        QLabel* badge = new QLabel(symbol);
        badge->setFixedSize(36, 36);
        badge->setAlignment(Qt::AlignCenter);
        badge->setStyleSheet(style);
        return badge;
    }

    bool showStyledPopup(QWidget* parent, PopupKind kind, const QString& title, const QString& text,
        const QString& okText, bool withCancel) {
        QDialog dialog(parent);
        dialog.setWindowTitle(title);
        dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
        dialog.setMinimumWidth(400);
        dialog.setMaximumWidth(520);

        QVBoxLayout* root = new QVBoxLayout(&dialog);
        root->setContentsMargins(24, 22, 24, 20);
        root->setSpacing(16);

        QHBoxLayout* header = new QHBoxLayout;
        header->setSpacing(12);
        header->addWidget(makePopupIcon(kind));
        QLabel* heading = makeLabel(title);
        heading->setStyleSheet("font-size: 16px; font-weight: 700; color: #f1f1f1;");
        heading->setWordWrap(true);
        header->addWidget(heading, 1);
        root->addLayout(header);

        QFrame* card = makeCard();
        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(16, 14, 16, 14);
        QLabel* body = makeLabel(text);
        body->setWordWrap(true);
        body->setStyleSheet("color: #e4e4e7; font-size: 13px;");
        body->setTextInteractionFlags(Qt::TextSelectableByMouse);
        cardLayout->addWidget(body);
        root->addWidget(card);

        QHBoxLayout* buttons = new QHBoxLayout;
        buttons->setSpacing(10);
        buttons->addStretch(1);

        if (withCancel) {
            QPushButton* cancelButton = makeButton(QString::fromUtf8("Отмена"), "ghost");
            cancelButton->setMinimumWidth(100);
            buttons->addWidget(cancelButton);
            QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
        }

        QString acceptKind = "primary";
        if (withCancel && (okText.contains(QString::fromUtf8("Удал"))
            || okText.contains(QString::fromUtf8("Отмен")))) {
            acceptKind = "danger";
        }
        QPushButton* okButton = makeButton(okText, acceptKind);
        okButton->setMinimumWidth(110);
        okButton->setDefault(true);
        buttons->addWidget(okButton);
        QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

        root->addLayout(buttons);
        return dialog.exec() == QDialog::Accepted;
    }

    void warn(QWidget* parent, const QString& title, const QString& text) {
        showStyledPopup(parent, PopupKind::Warning, title, text, QString::fromUtf8("Понятно"), false);
    }

    void critical(QWidget* parent, const QString& title, const QString& text) {
        showStyledPopup(parent, PopupKind::Error, title, text, QString::fromUtf8("Понятно"), false);
    }
}

void showError(QWidget* parent, const Barbershop& shop) {
    try {
        throw;
    }
    catch (const CancelledException&) {
        return;
    }
    catch (const ClientBannedException& error) {
        warn(parent, QString::fromUtf8("Запись недоступна"), qs(error.what()));
    }
    catch (const TimeAlreadyBookedException& error) {
        warn(parent, QString::fromUtf8("Время занято"),
            qs(error.what()) + QString::fromUtf8("\n\nВыберите другое время или воспользуйтесь умным подбором."));
    }
    catch (const OutsideWorkingHoursException& error) {
        warn(parent, QString::fromUtf8("Вне графика мастера"), qs(error.what()));
    }
    catch (const AccessDeniedException& error) {
        warn(parent, QString::fromUtf8("Нет доступа"), qs(error.what()));
    }
    catch (const LinkViolationException& error) {
        warn(parent, QString::fromUtf8("Есть связанные данные"), qs(error.what()));
    }
    catch (const AuthenticationException& error) {
        warn(parent, QString::fromUtf8("Ошибка входа"), qs(error.what()));
    }
    catch (const ObjectNotFoundException& error) {
        warn(parent, QString::fromUtf8("Не найдено"), qs(error.what()));
    }
    catch (const DuplicateObjectException& error) {
        warn(parent, QString::fromUtf8("Такая запись уже есть"), qs(error.what()));
    }
    catch (const InvalidDataException& error) {
        warn(parent, QString::fromUtf8("Некорректные данные"), qs(error.what()));
    }
    catch (const InvalidStatusTransitionException& error) {
        warn(parent, QString::fromUtf8("Смена статуса невозможна"), qs(error.what()));
    }
    catch (const InvalidOperationException& error) {
        warn(parent, QString::fromUtf8("Операция невозможна"), qs(error.what()));
    }
    catch (const FileFormatException& error) {
        critical(parent, QString::fromUtf8("Повреждённый файл"),
            qs(error.what()) + QString::fromUtf8("\n\nСостояние программы не изменено."));
    }
    catch (const FileOpenException& error) {
        critical(parent, QString::fromUtf8("Ошибка файла"), qs(error.what()));
    }
    catch (const FileIOException& error) {
        critical(parent, QString::fromUtf8("Ошибка чтения или записи"), qs(error.what()));
    }
    catch (const BarbershopException& error) {
        warn(parent, QString::fromUtf8("Ошибка"), qs(error.what()));
    }
    catch (const std::exception& error) {
        critical(parent, QString::fromUtf8("Непредвиденная ошибка"), qs(error.what()));
    }
    catch (...) {
        critical(parent, QString::fromUtf8("Непредвиденная ошибка"),
            QString::fromUtf8("Исключение неизвестного типа."));
    }

    try {
        throw;
    }
    catch (const std::exception& error) {
        shop.log("ERROR", error.what());
    }
    catch (...) {
        shop.log("ERROR", "An exception of an unknown type");
    }
}

void showInfo(QWidget* parent, const QString& title, const QString& text) {
    showStyledPopup(parent, PopupKind::Info, title, text, QString::fromUtf8("Готово"), false);
}

bool askConfirm(QWidget* parent, const QString& title, const QString& text, const QString& yesText) {
    QString okLabel = yesText.isEmpty() ? QString::fromUtf8("Да") : yesText;
    return showStyledPopup(parent, PopupKind::Question, title, text, okLabel, true);
}