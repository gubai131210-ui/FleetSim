#include "DouglasPeuckerSmoother.h"

#include <cmath>

namespace fleetsim::domain::planning {

DouglasPeuckerSmoother::DouglasPeuckerSmoother(double epsilon_m, double interpolate_step_m)
    : epsilon_m_(epsilon_m)
    , interpolate_step_m_(interpolate_step_m)
{
}

double DouglasPeuckerSmoother::perpendicularDistance(const core::Waypoint& point,
                                                     const core::Waypoint& line_start,
                                                     const core::Waypoint& line_end)
{
    const double dx = line_end.x - line_start.x;
    const double dy = line_end.y - line_start.y;
    const double norm = std::sqrt(dx * dx + dy * dy);
    if (norm < 1e-9) {
        const double px = point.x - line_start.x;
        const double py = point.y - line_start.y;
        return std::sqrt(px * px + py * py);
    }

    return std::abs(dy * point.x - dx * point.y + line_end.x * line_start.y - line_end.y * line_start.x)
        / norm;
}

void DouglasPeuckerSmoother::douglasPeucker(const std::vector<core::Waypoint>& points,
                                            std::size_t start_index,
                                            std::size_t end_index,
                                            double epsilon,
                                            std::vector<bool>& keep)
{
    if (end_index <= start_index + 1) {
        return;
    }

    double max_distance = 0.0;
    std::size_t index = start_index;
    for (std::size_t i = start_index + 1; i < end_index; ++i) {
        const double distance = perpendicularDistance(points[i], points[start_index], points[end_index]);
        if (distance > max_distance) {
            max_distance = distance;
            index = i;
        }
    }

    if (max_distance > epsilon) {
        keep[index] = true;
        douglasPeucker(points, start_index, index, epsilon, keep);
        douglasPeucker(points, index, end_index, epsilon, keep);
    }
}

core::Path DouglasPeuckerSmoother::interpolate(const std::vector<core::Waypoint>& simplified) const
{
    if (simplified.size() < 2) {
        return core::Path(simplified);
    }

    std::vector<core::Waypoint> dense;
    dense.push_back(simplified.front());

    for (std::size_t i = 1; i < simplified.size(); ++i) {
        const core::Waypoint& a = simplified[i - 1];
        const core::Waypoint& b = simplified[i];
        const double dx = b.x - a.x;
        const double dy = b.y - a.y;
        const double segment_length = std::sqrt(dx * dx + dy * dy);
        if (segment_length < 1e-9) {
            continue;
        }

        const int steps = static_cast<int>(std::floor(segment_length / interpolate_step_m_));
        for (int step = 1; step <= steps; ++step) {
            const double t = (static_cast<double>(step) * interpolate_step_m_) / segment_length;
            if (t >= 1.0) {
                break;
            }
            dense.push_back({a.x + t * dx, a.y + t * dy});
        }
        dense.push_back(b);
    }

    return core::Path(std::move(dense));
}

core::Path DouglasPeuckerSmoother::smooth(const core::Path& raw_path) const
{
    const auto& points = raw_path.waypoints();
    if (points.size() < 2) {
        return raw_path;
    }

    std::vector<bool> keep(points.size(), false);
    keep.front() = true;
    keep.back() = true;
    douglasPeucker(points, 0, points.size() - 1, epsilon_m_, keep);

    std::vector<core::Waypoint> simplified;
    simplified.reserve(points.size());
    for (std::size_t i = 0; i < points.size(); ++i) {
        if (keep[i]) {
            simplified.push_back(points[i]);
        }
    }

    return interpolate(simplified);
}

}  // namespace fleetsim::domain::planning
