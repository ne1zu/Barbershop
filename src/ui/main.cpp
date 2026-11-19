#include <filesystem>
#include <memory>
#include <string>
#include <system_error>
#include <QApplication>
#include <QLocale>
#include <QMessageBox>
#include "AppContext.h"
#include "LoginDialog.h"
#include "MainWindow.h"
#include "Styles.h"
#include "barbershop.h"
#include "config.h"
#include "data_storage.h"
#include "demo_data.h"
#include "exceptions/exceptions.h"

class GuardedApplication : public QApplication {
private:
    Barbershop* shopPointer;

public:
    GuardedApplication(int& argc, char** argv) : QApplication(argc, argv), shopPointer(nullptr) {}

    void setShop(Barbershop* shop) { shopPointer = shop; }

    bool notify(QObject* receiver, QEvent* event) override {
        try {
            return QApplication::notify(receiver, event);
        }
        catch (...) {
            if (shopPointer != nullptr) showError(activeWindow(), *shopPointer);
            return false;
        }
    }
};

namespace {
    bool initializeState(Barbershop& shop, std::string& startNote) {
        std::error_code error;
        if (!std::filesystem::exists(STATE_FILE_NAME, error)) {
            fillDemoData(shop);
            startNote = "No saved state, demo data created";
            return true;
        }
        try {
            shop.replaceContent(DataStorage(STATE_FILE_NAME).load());
            startNote = std::string("State loaded from ") + STATE_FILE_NAME;
            return true;
        }
        catch (const BarbershopException&) {
            showError(nullptr, shop);
            fillDemoData(shop);
            QMessageBox::information(nullptr, QString::fromUtf8("Запуск с демо-данными"),
                QString::fromUtf8("Файл %1 не удалось прочитать, поэтому программа запущена с демо-данными.\n\n"
                    "Автосохранение при выходе отключено, чтобы не затереть файл. "
                    "Исправьте или удалите файл, либо сохраните данные вручную (администратор → «Данные и отчёты»).")
                .arg(STATE_FILE_NAME));
            startNote = "The state file could not be loaded, demo data created, auto-save is off";
            return false;
        }
    }

    void autoSaveState(const Barbershop& shop) {
        try {
            DataStorage(STATE_FILE_NAME).save(shop);
            shop.log("AUTOSAVE", std::string("State saved to ") + STATE_FILE_NAME + " at exit");
        }
        catch (const BarbershopException&) {
            showError(nullptr, shop);
        }
    }
}

int main(int argc, char* argv[]) {
    GuardedApplication app(argc, argv);
    QLocale::setDefault(QLocale(QLocale::Russian, QLocale::Belarus));
    app.setStyle("Fusion");
    app.setPalette(Style::darkPalette());
    app.setStyleSheet(Style::sheet());

    Barbershop shop("Dolgih Barbershop", "Minsk, Gintovta 5");
    app.setShop(&shop);

    std::string startNote;
    bool autoSaveEnabled = false;
    try {
        autoSaveEnabled = initializeState(shop, startNote);
    }
    catch (...) {
        showError(nullptr, shop);
        return 1;
    }
    shop.setJournal(LOG_FILE_NAME);
    shop.log("START", startNote);

    while (true) {
        LoginDialog login(shop);
        if (login.exec() != QDialog::Accepted) break;

        AppContext context(shop, login.result());
        MainWindow window(&context);
        window.show();
        app.exec();
        if (!window.logoutRequested()) break;
    }

    if (autoSaveEnabled) autoSaveState(shop);
    shop.log("EXIT", "Program finished");
    return 0;
}
