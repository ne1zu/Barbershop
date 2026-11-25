#include "AppContext.h"
#include <QMessageBox>
#include <QPushButton>
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
    void warn(QWidget* parent, const QString& title, const QString& text) {
        QMessageBox::warning(parent, title, text);
    }

    void critical(QWidget* parent, const QString& title, const QString& text) {
        QMessageBox::critical(parent, title, text);
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
    QMessageBox::information(parent, title, text);
}

bool askConfirm(QWidget* parent, const QString& title, const QString& text, const QString& yesText) {
    QMessageBox box(QMessageBox::Question, title, text, QMessageBox::NoButton, parent);
    QPushButton* yes = box.addButton(yesText.isEmpty() ? QString::fromUtf8("Да") : yesText,
        QMessageBox::AcceptRole);
    QPushButton* no = box.addButton(QString::fromUtf8("Нет, отмена"), QMessageBox::RejectRole);
    box.setDefaultButton(no);
    box.setEscapeButton(no);
    box.exec();
    return box.clickedButton() == yes;
}
