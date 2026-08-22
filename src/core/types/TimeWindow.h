#pragma once

namespace fleetsim::core {

struct TimeWindow {
    double start_s{0.0};
    double end_s{0.0};

    bool overlaps(const TimeWindow& other) const
    {
        return start_s < other.end_s && other.start_s < end_s;
    }

    bool contains(double time_s) const
    {
        return time_s >= start_s && time_s <= end_s;
    }
};

}  // namespace fleetsim::core
