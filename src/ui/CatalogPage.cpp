#include "CatalogPage.h"
#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>
#include "UiUtils.h"

CatalogPage::CatalogPage(AppContext* contextValue, QWidget* parent) : PageBase(contextValue, parent) {
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(28, 20, 20, 20);
    QScrollArea* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    outer->addWidget(scroll);
    QWidget* inner = new QWidget;
    scroll->setWidget(inner);
    listLayout = new QVBoxLayout(inner);
    listLayout->setContentsMargins(0, 0, 8, 0);
    listLayout->setSpacing(16);
    bookGroup = new QButtonGroup(this);
    connect(bookGroup, &QButtonGroup::idClicked, this, &CatalogPage::onBookClicked);
}

QString CatalogPage::title() const { return QString::fromUtf8("Мастера и услуги"); }

void CatalogPage::onBookClicked(int masterId) { emit bookWithMaster(masterId); }

void CatalogPage::refresh() {
    const QList<QAbstractButton*> old = bookGroup->buttons();
    for (QAbstractButton* button : old) bookGroup->removeButton(button);
    clearLayout(listLayout);

    for (const auto& master : shop().getMasters()) {
        QFrame* card = makeCard();
        QVBoxLayout* box = new QVBoxLayout(card);
        box->setContentsMargins(22, 20, 22, 20);
        box->setSpacing(14);

        QHBoxLayout* head = new QHBoxLayout;
        head->setSpacing(14);
        QLabel* avatar = makeLabel(initials(master->getName()), "avatar");
        avatar->setFixedSize(40, 40);
        avatar->setAlignment(Qt::AlignCenter);
        head->addWidget(avatar);
        QVBoxLayout* names = new QVBoxLayout;
        names->setSpacing(2);
        QHBoxLayout* nameLine = new QHBoxLayout;
        nameLine->setSpacing(10);
        nameLine->addWidget(makeLabel(qs(master->getName()), "sectionTitle"));
        nameLine->addWidget(makeBadge(levelText(master->classify()), levelKind(master->classify())));
        nameLine->addStretch(1);
        names->addLayout(nameLine);
        QString dayOff = master->getDayOff() < 0 ? QString::fromUtf8("без выходных")
            : QString::fromUtf8("выходной: ") + dayOffText(master->getDayOff());
        names->addWidget(makeLabel(QString::fromUtf8("★ %1  ·  опыт %2 лет  ·  %3–%4  ·  %5")
            .arg(master->getRating(), 0, 'f', 1).arg(master->getExperience())
            .arg(formatClock(master->getWorkStart()), formatClock(master->getWorkEnd()), dayOff), "muted"));
        head->addLayout(names, 1);
        QPushButton* book = makeButton(QString::fromUtf8("Записаться к мастеру"), "primary");
        bookGroup->addButton(book, master->getId());
        head->addWidget(book);
        box->addLayout(head);

        if (master->getOffers().getCount() == 0) {
            box->addWidget(makeLabel(QString::fromUtf8("Мастер пока не добавил услуги."), "muted"));
        }
        else {
            QTableWidget* table = makeTable({ QString::fromUtf8("Услуга"), QString::fromUtf8("Длительность"),
                QString::fromUtf8("Цена") });
            table->setSelectionMode(QAbstractItemView::NoSelection);
            for (const auto& offer : master->getOffers()) {
                std::shared_ptr<Service> service = offer->getService();
                int row = table->rowCount();
                table->insertRow(row);
                table->setItem(row, 0, makeItem(qs(service->getName())));
                table->setItem(row, 1, makeItem(QString::fromUtf8("%1 мин").arg(master->calculateDuration(*service))));
                table->setItem(row, 2, makeItem(money(master->calculatePrice(*service))));
            }
            fitTableHeight(table);
            box->addWidget(table);
        }
        listLayout->addWidget(card);
    }

    QFrame* ranges = makeCard();
    QVBoxLayout* rangeBox = new QVBoxLayout(ranges);
    rangeBox->setContentsMargins(22, 20, 22, 20);
    rangeBox->setSpacing(12);
    rangeBox->addWidget(makeLabel(QString::fromUtf8("Диапазон цен по услугам"), "sectionTitle"));
    QTableWidget* table = makeTable({ QString::fromUtf8("Услуга"), QString::fromUtf8("Цена от"),
        QString::fromUtf8("Цена до"), QString::fromUtf8("Мастеров") });
    table->setSelectionMode(QAbstractItemView::NoSelection);
    for (const auto& service : shop().getServices()) {
        double low = 0.0;
        double high = 0.0;
        int count = 0;
        for (const auto& master : shop().getMasters()) {
            if (!master->canPerform(*service)) continue;
            double price = master->calculatePrice(*service);
            if (count == 0 || price < low) low = price;
            if (count == 0 || price > high) high = price;
            ++count;
        }
        if (count == 0) continue;
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, makeItem(qs(service->getName())));
        table->setItem(row, 1, makeItem(money(low)));
        table->setItem(row, 2, makeItem(money(high)));
        table->setItem(row, 3, makeItem(QString::number(count)));
    }
    fitTableHeight(table);
    rangeBox->addWidget(table);
    listLayout->addWidget(ranges);
    listLayout->addStretch(1);
}
