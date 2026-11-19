#include "BookingPanel.h"
#include <QButtonGroup>
#include <QComboBox>
#include <QCompleter>
#include <QDateEdit>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include "EntityDialogs.h"
#include "UiUtils.h"
#include "config.h"
#include "exceptions/exceptions.h"
#include "utils.h"

BookingPanel::BookingPanel(AppContext* contextValue, bool staffModeValue, QWidget* parent)
    : QWidget(parent), context(contextValue), staffMode(staffModeValue), updating(false), desiredMinute(-1),
    selectedSlot(-1) {
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    QScrollArea* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    outer->addWidget(scroll);

    QWidget* inner = new QWidget;
    scroll->setWidget(inner);
    QVBoxLayout* root = new QVBoxLayout(inner);
    root->setContentsMargins(0, 0, 8, 0);
    root->setSpacing(14);

    bannerLabel = makeLabel(QString::fromUtf8("Онлайн-запись для вашего аккаунта недоступна. Обратитесь к администратору барбершопа."),
        "dangerBanner");
    bannerLabel->setWordWrap(true);
    bannerLabel->setVisible(false);
    root->addWidget(bannerLabel);

    clientRow = new QWidget;
    QHBoxLayout* clientLayout = new QHBoxLayout(clientRow);
    clientLayout->setContentsMargins(0, 0, 0, 0);
    clientLayout->setSpacing(10);
    clientLayout->addWidget(makeLabel(QString::fromUtf8("КЛИЕНТ"), "fieldLabel"));
    clientCombo = new QComboBox;
    clientCombo->setEditable(true);
    clientCombo->setInsertPolicy(QComboBox::NoInsert);
    clientCombo->setMinimumWidth(320);
    clientCombo->completer()->setFilterMode(Qt::MatchContains);
    clientCombo->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    clientLayout->addWidget(clientCombo, 1);
    QPushButton* newClientButton = makeButton(QString::fromUtf8("+ Новый клиент"), "ghost");
    clientLayout->addWidget(newClientButton);
    clientRow->setVisible(staffMode);
    root->addWidget(clientRow);

    tabs = new QTabWidget;
    tabs->addTab(buildManualTab(), QString::fromUtf8("Выбрать время"));
    tabs->addTab(buildSmartTab(), QString::fromUtf8("Умный подбор"));
    root->addWidget(tabs, 1);

    connect(newClientButton, &QPushButton::clicked, this, &BookingPanel::onNewClient);
    connect(clientCombo, &QComboBox::currentIndexChanged, this, &BookingPanel::onClientChanged);
    connect(masterCombo, &QComboBox::currentIndexChanged, this, &BookingPanel::onMasterChanged);
    connect(serviceCombo, &QComboBox::currentIndexChanged, this, &BookingPanel::onServiceChanged);
    connect(dateEdit, &QDateEdit::dateChanged, this, &BookingPanel::onDateChanged);
    connect(slotGroup, &QButtonGroup::idClicked, this, &BookingPanel::onSlotChosen);
    connect(smartService, &QComboBox::currentIndexChanged, this, &BookingPanel::onSmartServiceChanged);
    connect(optionGroup, &QButtonGroup::idClicked, this, &BookingPanel::onOptionChosen);

    refresh();
}

Barbershop& BookingPanel::shop() const { return context->shop(); }

QWidget* BookingPanel::buildManualTab() {
    QWidget* page = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 14, 0, 0);
    layout->setSpacing(14);

    QFrame* card = makeCard();
    QGridLayout* form = new QGridLayout(card);
    form->setContentsMargins(20, 18, 20, 18);
    form->setHorizontalSpacing(16);
    form->setVerticalSpacing(8);
    masterCombo = new QComboBox;
    serviceCombo = new QComboBox;
    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("dd.MM.yyyy");
    dateEdit->setMinimumDate(QDate::currentDate());
    form->addWidget(makeLabel(QString::fromUtf8("МАСТЕР"), "fieldLabel"), 0, 0);
    form->addWidget(makeLabel(QString::fromUtf8("УСЛУГА"), "fieldLabel"), 0, 1);
    form->addWidget(makeLabel(QString::fromUtf8("ДАТА"), "fieldLabel"), 0, 2);
    form->addWidget(masterCombo, 1, 0);
    form->addWidget(serviceCombo, 1, 1);
    form->addWidget(dateEdit, 1, 2);
    form->setColumnStretch(0, 2);
    form->setColumnStretch(1, 3);
    form->setColumnStretch(2, 1);
    infoLabel = makeLabel(QString(), "muted");
    infoLabel->setWordWrap(true);
    form->addWidget(infoLabel, 2, 0, 1, 3);
    layout->addWidget(card);

    QFrame* slotsCard = makeCard();
    QVBoxLayout* slotsOuter = new QVBoxLayout(slotsCard);
    slotsOuter->setContentsMargins(20, 18, 20, 18);
    slotsOuter->setSpacing(10);
    slotsOuter->addWidget(makeLabel(QString::fromUtf8("Свободное время"), "sectionTitle"));
    slotsHint = makeLabel(QString(), "muted");
    slotsHint->setWordWrap(true);
    slotsOuter->addWidget(slotsHint);
    slotsHost = new QWidget;
    slotsLayout = new QGridLayout(slotsHost);
    slotsLayout->setContentsMargins(0, 0, 0, 0);
    slotsLayout->setSpacing(8);
    slotsOuter->addWidget(slotsHost);
    slotGroup = new QButtonGroup(this);
    slotGroup->setExclusive(true);
    layout->addWidget(slotsCard);

    bookButton = makeButton(QString::fromUtf8("Выберите время"), "primary");
    bookButton->setProperty("kind", "primary");
    bookButton->setMinimumHeight(44);
    bookButton->setEnabled(false);
    connect(bookButton, &QPushButton::clicked, this, &BookingPanel::onBook);
    layout->addWidget(bookButton);
    layout->addStretch(1);
    return page;
}

QWidget* BookingPanel::buildSmartTab() {
    QWidget* page = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 14, 0, 0);
    layout->setSpacing(14);

    QFrame* card = makeCard();
    QGridLayout* form = new QGridLayout(card);
    form->setContentsMargins(20, 18, 20, 18);
    form->setHorizontalSpacing(16);
    form->setVerticalSpacing(8);
    smartService = new QComboBox;
    smartMaster = new QComboBox;
    daysSpin = new QSpinBox;
    daysSpin->setRange(1, MAX_SEARCH_DAYS);
    daysSpin->setValue(7);
    daysSpin->setSuffix(QString::fromUtf8(" дн."));
    hourSpin = new QSpinBox;
    hourSpin->setRange(-1, 23);
    hourSpin->setValue(-1);
    hourSpin->setSpecialValueText(QString::fromUtf8("Любое"));
    hourSpin->setSuffix(":00");
    form->addWidget(makeLabel(QString::fromUtf8("УСЛУГА"), "fieldLabel"), 0, 0);
    form->addWidget(makeLabel(QString::fromUtf8("МАСТЕР"), "fieldLabel"), 0, 1);
    form->addWidget(makeLabel(QString::fromUtf8("ИСКАТЬ ВПЕРЁД"), "fieldLabel"), 0, 2);
    form->addWidget(makeLabel(QString::fromUtf8("ЖЕЛАЕМОЕ ВРЕМЯ"), "fieldLabel"), 0, 3);
    form->addWidget(smartService, 1, 0);
    form->addWidget(smartMaster, 1, 1);
    form->addWidget(daysSpin, 1, 2);
    form->addWidget(hourSpin, 1, 3);
    form->setColumnStretch(0, 3);
    form->setColumnStretch(1, 3);
    form->setColumnStretch(2, 1);
    form->setColumnStretch(3, 1);
    QLabel* explain = makeLabel(QString::fromUtf8(
        "Система сравнит свободное время всех подходящих мастеров и покажет 5 лучших вариантов. "
        "Score (0-100) учитывает близость к желаемому времени, скорость записи, рейтинг мастера, "
        "плотность графика и цену."), "mutedSmall");
    explain->setWordWrap(true);
    form->addWidget(explain, 2, 0, 1, 4);
    QPushButton* findButton = makeButton(QString::fromUtf8("Подобрать время"), "primary");
    findButton->setMinimumHeight(40);
    form->addWidget(findButton, 3, 0, 1, 4);
    connect(findButton, &QPushButton::clicked, this, &BookingPanel::onFind);
    layout->addWidget(card);

    QWidget* results = new QWidget;
    resultsLayout = new QVBoxLayout(results);
    resultsLayout->setContentsMargins(0, 0, 0, 0);
    resultsLayout->setSpacing(10);
    layout->addWidget(results);
    optionGroup = new QButtonGroup(this);
    layout->addStretch(1);
    return page;
}

void BookingPanel::selectComboById(QComboBox* combo, int id) {
    for (int i = 0; i < combo->count(); ++i) {
        if (combo->itemData(i).toInt() == id) {
            combo->setCurrentIndex(i);
            return;
        }
    }
}

std::shared_ptr<Client> BookingPanel::currentClient() const {
    if (!staffMode) return context->client();
    int index = clientCombo->findText(clientCombo->currentText());
    if (index < 0) return nullptr;
    return findClientById(shop(), clientCombo->itemData(index).toInt());
}

std::shared_ptr<Master> BookingPanel::currentMaster() const {
    if (masterCombo->currentIndex() < 0) return nullptr;
    return findMasterById(shop(), masterCombo->currentData().toInt());
}

std::shared_ptr<Service> BookingPanel::currentService() const {
    if (serviceCombo->currentIndex() < 0) return nullptr;
    return findServiceById(shop(), serviceCombo->currentData().toInt());
}

void BookingPanel::populateClients() {
    int keep = -1;
    std::shared_ptr<Client> selected = currentClient();
    if (selected) keep = selected->getId();
    clientCombo->clear();
    for (const auto& client : shop().getClients()) {
        QString text = qs(client->getName()) + "  —  " + qs(client->getPhone());
        if (client->isBanned()) text += QString::fromUtf8("  (заблокирован)");
        clientCombo->addItem(text, client->getId());
    }
    clientCombo->setCurrentIndex(-1);
    clientCombo->lineEdit()->setPlaceholderText(QString::fromUtf8("Начните вводить имя или телефон клиента"));
    if (keep >= 0) selectComboById(clientCombo, keep);
}

void BookingPanel::populateMasters(QComboBox* combo, bool includeAny) {
    int keep = combo->currentIndex() >= 0 ? combo->currentData().toInt() : -1;
    combo->clear();
    if (includeAny) combo->addItem(QString::fromUtf8("Любой мастер"), -1);
    for (const auto& master : shop().getMasters()) {
        if (context->isMaster() && master.get() != context->master().get()) continue;
        combo->addItem(qs(master->getName()) + "  ·  " + levelText(master->classify()), master->getId());
    }
    if (keep >= 0) selectComboById(combo, keep);
    if (combo->currentIndex() < 0 && combo->count() > 0) combo->setCurrentIndex(0);
    combo->setEnabled(!context->isMaster());
}

void BookingPanel::populateManualServices() {
    int keep = serviceCombo->currentIndex() >= 0 ? serviceCombo->currentData().toInt() : -1;
    serviceCombo->clear();
    std::shared_ptr<Master> master = currentMaster();
    if (master) {
        for (const auto& offer : master->getOffers()) {
            std::shared_ptr<Service> service = offer->getService();
            serviceCombo->addItem(qs(service->getName()) + "  ·  " + money(master->calculatePrice(*service))
                + "  ·  " + QString::fromUtf8("%1 мин").arg(master->calculateDuration(*service)), service->getId());
        }
    }
    if (keep >= 0) selectComboById(serviceCombo, keep);
    if (serviceCombo->currentIndex() < 0 && serviceCombo->count() > 0) serviceCombo->setCurrentIndex(0);
}

void BookingPanel::populateSmartServices() {
    int keep = smartService->currentIndex() >= 0 ? smartService->currentData().toInt() : -1;
    smartService->clear();
    for (const auto& service : shop().getServices()) {
        bool offered = false;
        for (const auto& master : shop().getMasters()) {
            if (context->isMaster() && master.get() != context->master().get()) continue;
            if (master->canPerform(*service)) offered = true;
        }
        if (offered) smartService->addItem(qs(service->getName()), service->getId());
    }
    if (keep >= 0) selectComboById(smartService, keep);
    if (smartService->currentIndex() < 0 && smartService->count() > 0) smartService->setCurrentIndex(0);
}

void BookingPanel::populateSmartMasters() {
    smartMaster->clear();
    smartMaster->addItem(QString::fromUtf8("Любой мастер"), -1);
    if (smartService->currentIndex() < 0) return;
    std::shared_ptr<Service> service = findServiceById(shop(), smartService->currentData().toInt());
    if (!service) return;
    for (const auto& master : shop().getMasters()) {
        if (context->isMaster() && master.get() != context->master().get()) continue;
        if (!master->canPerform(*service)) continue;
        smartMaster->addItem(qs(master->getName()) + "  ·  " + money(master->calculatePrice(*service)), master->getId());
    }
    if (context->isMaster() && smartMaster->count() > 1) smartMaster->setCurrentIndex(1);
}

void BookingPanel::refresh() {
    updating = true;
    populateClients();
    populateMasters(masterCombo, false);
    populateManualServices();
    populateSmartServices();
    populateSmartMasters();
    updating = false;

    bool banned = !staffMode && context->client() && context->client()->isBanned();
    bannerLabel->setVisible(banned);
    tabs->setEnabled(!banned);

    updateSlots();
}

void BookingPanel::presetClient(const std::shared_ptr<Client>& client) {
    if (!client) return;
    updating = true;
    selectComboById(clientCombo, client->getId());
    updating = false;
    updateSlots();
}

void BookingPanel::selectMaster(int masterId) {
    updating = true;
    selectComboById(masterCombo, masterId);
    populateManualServices();
    updating = false;
    tabs->setCurrentIndex(0);
    updateSlots();
}

void BookingPanel::presetSlot(int masterId, const QDate& date, int minuteOfDay) {
    updating = true;
    selectComboById(masterCombo, masterId);
    populateManualServices();
    if (date >= dateEdit->minimumDate()) dateEdit->setDate(date);
    updating = false;
    desiredMinute = minuteOfDay;
    tabs->setCurrentIndex(0);
    updateSlots();
}

void BookingPanel::onClientChanged() {
    if (!updating) updateSlots();
}

void BookingPanel::onNewClient() {
    ClientDialog dialog(shop(), this);
    if (dialog.exec() != QDialog::Accepted) return;
    std::shared_ptr<Client> client = dialog.result();
    populateClients();
    if (client) selectComboById(clientCombo, client->getId());
}

void BookingPanel::onMasterChanged() {
    if (updating) return;
    updating = true;
    populateManualServices();
    updating = false;
    updateSlots();
}

void BookingPanel::onServiceChanged() {
    if (!updating) updateSlots();
}

void BookingPanel::onDateChanged() {
    if (!updating) updateSlots();
}

void BookingPanel::clearSlots() {
    const QList<QAbstractButton*> old = slotGroup->buttons();
    for (QAbstractButton* button : old) slotGroup->removeButton(button);
    clearLayout(slotsLayout);
    times.clear();
    selectedSlot = -1;
    bookButton->setEnabled(false);
    bookButton->setText(QString::fromUtf8("Выберите время"));
}

void BookingPanel::updateSlots() {
    clearSlots();
    std::shared_ptr<Master> master = currentMaster();
    std::shared_ptr<Service> service = currentService();
    if (!master || !service) {
        infoLabel->setText(QString());
        slotsHint->setText(QString::fromUtf8("Выберите мастера и услугу."));
        slotsHint->setVisible(true);
        return;
    }
    std::shared_ptr<Client> client = currentClient();
    double price = master->calculatePrice(*service);
    int duration = master->calculateDuration(*service);
    QString info = QString::fromUtf8("Цена: %1  ·  длительность: %2 мин").arg(money(price)).arg(duration);
    if (client && client->getDiscountPercent() > 0) {
        info += QString::fromUtf8("  ·  скидка клиента %1%, к оплате %2")
            .arg(client->getDiscountPercent()).arg(money(roundMoney(price * client->getPriceFactor())));
    }
    infoLabel->setText(info);

    DateTime day = toDateTime(dateEdit->date());
    if (master->getDayOff() == day.dayOfWeek()) {
        slotsHint->setText(QString::fromUtf8("У мастера в этот день выходной (%1). Выберите другую дату.")
            .arg(weekdayName(day.dayOfWeek())));
        slotsHint->setVisible(true);
        return;
    }
    times = findFreeStartTimes(shop(), *master, client.get(), day, duration);
    if (times.empty()) {
        slotsHint->setText(QString::fromUtf8("На эту дату свободного времени нет. Попробуйте другой день или умный подбор."));
        slotsHint->setVisible(true);
        return;
    }
    slotsHint->setVisible(false);
    const int columns = 7;
    int preferred = -1;
    for (int i = 0; i < static_cast<int>(times.size()); ++i) {
        QPushButton* chip = new QPushButton(formatClock(times[i].minuteOfDay()));
        chip->setObjectName("chip");
        chip->setCheckable(true);
        chip->setCursor(Qt::PointingHandCursor);
        slotGroup->addButton(chip, i);
        slotsLayout->addWidget(chip, i / columns, i % columns);
        if (times[i].minuteOfDay() == desiredMinute) preferred = i;
    }
    if (preferred >= 0) {
        slotGroup->button(preferred)->setChecked(true);
        onSlotChosen(preferred);
    }
}

void BookingPanel::onSlotChosen(int index) {
    if (index < 0 || index >= static_cast<int>(times.size())) return;
    selectedSlot = index;
    desiredMinute = times[index].minuteOfDay();
    bookButton->setEnabled(true);
    bookButton->setText(QString::fromUtf8("Записаться на %1, %2")
        .arg(formatDay(times[index]), formatClock(times[index].minuteOfDay())));
}

void BookingPanel::finishBooking(const std::shared_ptr<Appointment>& appointment) {
    QString text = QString::fromUtf8("%1\n%2 · %3\nВремя: %4\nЦена: %5")
        .arg(qs(appointment->getClient()->getName()), qs(appointment->getService()->getName()),
            qs(appointment->getMaster()->getName()), formatDayTime(appointment->getStart()),
            money(appointment->getPrice()));
    int discount = appointment->getClient()->getDiscountPercent();
    if (discount > 0) text += QString::fromUtf8("\nСкидка %1% будет применена при оплате.").arg(discount);
    showInfo(this, QString::fromUtf8("Запись создана"), text);
    emit booked();
}

void BookingPanel::onBook() {
    try {
        std::shared_ptr<Client> client = currentClient();
        if (!client) throw InvalidDataException("client", "choose a client from the list");
        if (client->isBanned()) throw ClientBannedException(client->getName());
        std::shared_ptr<Master> master = currentMaster();
        std::shared_ptr<Service> service = currentService();
        if (!master || !service || selectedSlot < 0) throw InvalidDataException("time", "choose a time slot");
        std::shared_ptr<Appointment> appointment =
            shop().book(client, master, service, times[selectedSlot], context->user());
        updateSlots();
        finishBooking(appointment);
    }
    catch (...) {
        showError(this, shop());
        updateSlots();
    }
}

void BookingPanel::onSmartServiceChanged() {
    if (updating) return;
    populateSmartMasters();
    clearOptions();
}

void BookingPanel::clearOptions() {
    const QList<QAbstractButton*> old = optionGroup->buttons();
    for (QAbstractButton* button : old) optionGroup->removeButton(button);
    clearLayout(resultsLayout);
    options.clear();
}

void BookingPanel::onFind() {
    try {
        std::shared_ptr<Client> client = currentClient();
        if (!client) throw InvalidDataException("client", "choose a client from the list");
        if (client->isBanned()) throw ClientBannedException(client->getName());
        if (smartService->currentIndex() < 0) throw InvalidDataException("service", "no service is available");
        SlotRequest request;
        request.client = client;
        request.service = findServiceById(shop(), smartService->currentData().toInt());
        int masterId = smartMaster->currentData().toInt();
        request.master = masterId < 0 ? nullptr : findMasterById(shop(), masterId);
        request.firstDay = DateTime::now();
        request.days = daysSpin->value();
        request.preferredMinute = hourSpin->value() < 0 ? -1 : hourSpin->value() * 60;
        options = suggestSlots(shop(), request);
        showOptions();
    }
    catch (...) {
        clearOptions();
        showError(this, shop());
    }
}

void BookingPanel::showOptions() {
    const QList<QAbstractButton*> old = optionGroup->buttons();
    for (QAbstractButton* button : old) optionGroup->removeButton(button);
    clearLayout(resultsLayout);

    if (options.empty()) {
        QLabel* none = makeLabel(QString::fromUtf8(
            "Свободного времени не найдено. Увеличьте период поиска или выберите другого мастера."), "dangerBanner");
        none->setWordWrap(true);
        resultsLayout->addWidget(none);
        return;
    }
    resultsLayout->addWidget(makeLabel(QString::fromUtf8("Лучшие варианты"), "sectionTitle"));

    for (int i = 0; i < static_cast<int>(options.size()) && i < SUGGESTION_LIMIT; ++i) {
        const SlotOption& option = options[i];
        QFrame* card = new QFrame;
        card->setObjectName("optionCard");
        QHBoxLayout* row = new QHBoxLayout(card);
        row->setContentsMargins(18, 14, 18, 14);
        row->setSpacing(16);

        QLabel* rank = makeLabel("#" + QString::number(i + 1), "rankBubble");
        rank->setFixedSize(34, 34);
        rank->setAlignment(Qt::AlignCenter);
        row->addWidget(rank);

        QVBoxLayout* info = new QVBoxLayout;
        info->setSpacing(2);
        info->addWidget(makeLabel(fullDayTitle(option.start) + "  ·  " + formatClock(option.start.minuteOfDay()),
            "sectionTitle"));
        info->addWidget(makeLabel(qs(option.master->getName()) + "  ·  " + levelText(option.master->classify())
            + QString::fromUtf8("  ·  ★ ") + QString::number(option.master->getRating(), 'f', 1), "muted"));
        row->addLayout(info, 1);

        QVBoxLayout* priceBox = new QVBoxLayout;
        priceBox->setSpacing(2);
        priceBox->addWidget(makeLabel(QString::fromUtf8("ЦЕНА"), "statTitle"));
        priceBox->addWidget(makeLabel(money(option.price), "infoValue"));
        row->addLayout(priceBox);

        QVBoxLayout* scoreBox = new QVBoxLayout;
        scoreBox->setSpacing(4);
        QHBoxLayout* scoreLine = new QHBoxLayout;
        scoreLine->addWidget(makeLabel("SCORE", "statTitle"));
        scoreLine->addStretch(1);
        scoreLine->addWidget(makeLabel(QString::number(option.score), "scoreValue"));
        scoreBox->addLayout(scoreLine);
        QProgressBar* bar = new QProgressBar;
        bar->setRange(0, 100);
        bar->setValue(option.score);
        bar->setTextVisible(false);
        bar->setMinimumWidth(120);
        scoreBox->addWidget(bar);
        row->addLayout(scoreBox);

        QPushButton* choose = makeButton(QString::fromUtf8("Записаться"), "primary");
        optionGroup->addButton(choose, i);
        row->addWidget(choose);
        resultsLayout->addWidget(card);
    }
}

void BookingPanel::onOptionChosen(int index) {
    if (index < 0 || index >= static_cast<int>(options.size())) return;
    try {
        std::shared_ptr<Client> client = currentClient();
        if (!client) throw InvalidDataException("client", "choose a client from the list");
        if (client->isBanned()) throw ClientBannedException(client->getName());
        std::shared_ptr<Service> service = findServiceById(shop(), smartService->currentData().toInt());
        SlotOption option = options[index];
        std::shared_ptr<Appointment> appointment =
            shop().book(client, option.master, service, option.start, context->user());
        clearOptions();
        updateSlots();
        finishBooking(appointment);
    }
    catch (...) {
        showError(this, shop());
    }
}

NewBookingDialog::NewBookingDialog(AppContext* contextValue, QWidget* parent) : QDialog(parent) {
    setWindowTitle(QString::fromUtf8("Новая запись"));
    resize(900, 720);
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(26, 22, 18, 20);
    root->setSpacing(12);
    root->addWidget(makeLabel(QString::fromUtf8("Новая запись"), "loginTitle"));
    panel = new BookingPanel(contextValue, true);
    root->addWidget(panel, 1);
    connect(panel, &BookingPanel::booked, this, &QDialog::accept);
}

void NewBookingDialog::presetClient(const std::shared_ptr<Client>& client) { panel->presetClient(client); }

void NewBookingDialog::presetSlot(int masterId, const QDate& date, int minuteOfDay) {
    panel->presetSlot(masterId, date, minuteOfDay);
}

BookingPage::BookingPage(AppContext* contextValue, QWidget* parent) : PageBase(contextValue, parent) {
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 20, 20, 20);
    panel = new BookingPanel(contextValue, false);
    root->addWidget(panel);
    connect(panel, &BookingPanel::booked, this, &BookingPage::onBooked);
}

QString BookingPage::title() const { return QString::fromUtf8("Запись на стрижку"); }
void BookingPage::refresh() { panel->refresh(); }
void BookingPage::selectMaster(int masterId) { panel->selectMaster(masterId); }
void BookingPage::onBooked() { emit dataChanged(); }
