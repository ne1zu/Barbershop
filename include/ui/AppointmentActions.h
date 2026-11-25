#pragma once
#include <memory>
#include <QString>
#include "AppContext.h"
#include "appointment.h"

class QWidget;

struct ActionState {
    bool confirm = false;
    bool arrived = false;
    bool noShow = false;
    bool cancel = false;
    bool reschedule = false;
    bool pay = false;
    bool remove = false;
    QString confirmHint;
    QString arrivedHint;
    QString noShowHint;
    QString cancelHint;
    QString rescheduleHint;
    QString payHint;
};

namespace actions {
    ActionState stateFor(const AppContext* ctx, const std::shared_ptr<Appointment>& appointment);

    bool confirm(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment);
    bool markArrived(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment);
    bool markNoShow(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment);
    bool cancel(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment);
    bool reschedule(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment);
    bool pay(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment);
    bool remove(AppContext* ctx, QWidget* parent, const std::shared_ptr<Appointment>& appointment);
}
