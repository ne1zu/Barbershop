#pragma once
#include <memory>
#include <string>
#include <vector>
#include <QDate>
#include <QIcon>
#include <QString>
#include <QStringList>
#include "appointment.h"
#include "barbershop.h"
#include "datetime.h"
#include "payment.h"

class QFrame;
class QLabel;
class QLayout;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;
class QWidget;

QString qs(const std::string& text);
std::string ss(const QString& text);

QDate toQDate(const DateTime& value);
DateTime toDateTime(const QDate& date, int minuteOfDay = 0);

QString formatDay(const DateTime& value);
QString formatDayTime(const DateTime& value);
QString formatClock(int minuteOfDay);
QString money(double value);

QString statusText(AppointmentStatus status);
QString statusKind(AppointmentStatus status);
QString levelText(const std::string& level);
QString levelKind(const std::string& level);
QString paymentMethodText(PaymentMethod method);
QString weekdayName(int dayOfWeek);
QString weekdayShort(int dayOfWeek);
QString dayOffText(int dayOff);
QString fullDayTitle(const DateTime& value);
QString initials(const std::string& name);

std::shared_ptr<Client> findClientById(const Barbershop& shop, int id);
std::shared_ptr<Master> findMasterById(const Barbershop& shop, int id);
std::shared_ptr<Service> findServiceById(const Barbershop& shop, int id);
std::shared_ptr<Appointment> findAppointmentById(const Barbershop& shop, int id);

std::vector<std::shared_ptr<Appointment>> sortedAppointments(const Barbershop& shop);

QLabel* makeLabel(const QString& text, const QString& objectName = QString());
QLabel* makeBadge(const QString& text, const QString& kind);
void setBadge(QLabel* badge, const QString& text, const QString& kind);
QWidget* makeBadgeCell(const QString& text, const QString& kind);
QPushButton* makeButton(const QString& text, const QString& kind = QString());
QFrame* makeCard();
QFrame* makeStatCard(const QString& title, const QString& value, const QString& hint = QString(),
    bool accent = false, QLabel** valueLabel = nullptr);
QWidget* makeBarRow(const QString& label, double value, double maxValue, const QString& valueText);
QWidget* makeInfoRow(const QString& key, QLabel** valueLabel);

QTableWidget* makeTable(const QStringList& headers);
QTableWidgetItem* makeItem(const QString& text, bool alignRight = false);
void fitTableHeight(QTableWidget* table);
void clearLayout(QLayout* layout);
void repolish(QWidget* widget);

QIcon makeIcon(const QString& name);
