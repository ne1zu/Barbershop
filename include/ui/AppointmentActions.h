#pragma once
#include <memory>
#include "AppContext.h"
#include "appointment.h"

class QWidget;

struct ActionState {
    bool confirm;
    bool arrived;
    bool noShow;
    bool cancel;
    bool reschedule;
    bool pay;
    bool remove;
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
