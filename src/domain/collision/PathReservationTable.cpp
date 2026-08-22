#include "PathReservationTable.h"

#include <cmath>

namespace fleetsim::domain::collision {

void PathReservationTable::clear()
{
    reservations_.clear();
}

PathReservationTable::CellTimeKey PathReservationTable::makeKey(int row, int col, int slot)
{
    return CellTimeKey{row, col, slot};
}

void PathReservationTable::addReservation(int row, int col, const core::VehicleId& vehicle_id,
                                          const core::TimeWindow& window, int priority)
{
    const int start_slot = static_cast<int>(window.start_s);
    const int end_slot = static_cast<int>(std::ceil(window.end_s));
    for (int slot = start_slot; slot <= end_slot; ++slot) {
        core::TimeWindow slot_window;
        slot_window.start_s = static_cast<double>(slot);
        slot_window.end_s = static_cast<double>(slot + 1);
        reservations_[makeKey(row, col, slot)] = ReservationEntry{vehicle_id, slot_window, priority};
    }
}

bool PathReservationTable::hasConflict(int row, int col, const core::TimeWindow& window,
                                       const core::VehicleId& self_id, int self_priority) const
{
    const int start_slot = static_cast<int>(window.start_s);
    const int end_slot = static_cast<int>(std::ceil(window.end_s));
    for (int slot = start_slot; slot <= end_slot; ++slot) {
        const auto it = reservations_.find(makeKey(row, col, slot));
        if (it == reservations_.end()) {
            continue;
        }
        if (it->second.vehicle_id == self_id) {
            continue;
        }
        if (it->second.window.overlaps(window) && it->second.priority >= self_priority) {
            return true;
        }
    }
    return false;
}

double PathReservationTable::speedScaleFor(int row, int col, const core::TimeWindow& window,
                                           const core::VehicleId& self_id, int self_priority,
                                           double conflict_slowdown) const
{
    if (hasConflict(row, col, window, self_id, self_priority)) {
        return conflict_slowdown;
    }
    return 1.0;
}

}  // namespace fleetsim::domain::collision
