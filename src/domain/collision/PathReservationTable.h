#pragma once

#include "core/types/TimeWindow.h"
#include "core/types/VehicleId.h"

#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace fleetsim::domain::collision {

struct ReservationEntry {
    core::VehicleId vehicle_id;
    core::TimeWindow window;
    int priority{0};
};

class PathReservationTable {
public:
    void clear();

    void addReservation(int row, int col, const core::VehicleId& vehicle_id,
                        const core::TimeWindow& window, int priority);

    bool hasConflict(int row, int col, const core::TimeWindow& window,
                     const core::VehicleId& self_id, int self_priority) const;

    double speedScaleFor(int row, int col, const core::TimeWindow& window,
                         const core::VehicleId& self_id, int self_priority,
                         double conflict_slowdown) const;

    std::size_t reservationCount() const { return reservations_.size(); }

private:
    using CellTimeKey = std::tuple<int, int, int>;

    static CellTimeKey makeKey(int row, int col, int slot);

    std::map<CellTimeKey, ReservationEntry> reservations_;
};

}  // namespace fleetsim::domain::collision
