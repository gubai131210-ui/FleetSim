#include "MonitorBridge.h"

#include "core/types/Waypoint.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace fleetsim::app {

MonitorBridge::MonitorBridge(SimController* controller, QObject* parent)
    : QObject(parent)
    , controller_(controller)
{
}

void MonitorBridge::bind()
{
    if (controller_ == nullptr || pose_subscription_id_ != 0) {
        return;
    }

    auto& bus = controller_->engine().eventBus();
    pose_subscription_id_ = bus.subscribe("sim/pose_updated", [this](const std::string& payload) {
        const nlohmann::json json = nlohmann::json::parse(payload);
        const double x = json.at("x").get<double>();
        const double y = json.at("y").get<double>();
        const double linear_velocity = json.value("linear_velocity", 0.0);

        sim_time_s_ += controller_->engine().clock().fixedDt();
        const double cross_track_error = computeCrossTrackError(x, y);
        emit sampleReady(sim_time_s_, cross_track_error, linear_velocity);
    });
}

void MonitorBridge::unbind()
{
    if (controller_ == nullptr || pose_subscription_id_ == 0) {
        return;
    }
    controller_->engine().eventBus().unsubscribe("sim/pose_updated", pose_subscription_id_);
    pose_subscription_id_ = 0;
}

void MonitorBridge::reset()
{
    sim_time_s_ = 0.0;
}

double MonitorBridge::computeCrossTrackError(double x_m, double y_m) const
{
    if (controller_ == nullptr) {
        return 0.0;
    }

    const auto& path = controller_->engine().referencePath();
    const auto& waypoints = path.waypoints();
    if (waypoints.size() < 2) {
        return 0.0;
    }

    double min_distance = std::numeric_limits<double>::max();
    for (std::size_t i = 0; i + 1 < waypoints.size(); ++i) {
        const core::Waypoint& a = waypoints[i];
        const core::Waypoint& b = waypoints[i + 1];

        const double abx = b.x - a.x;
        const double aby = b.y - a.y;
        const double ab_len_sq = abx * abx + aby * aby;
        if (ab_len_sq < 1e-12) {
            continue;
        }

        const double t = std::clamp(((x_m - a.x) * abx + (y_m - a.y) * aby) / ab_len_sq, 0.0, 1.0);
        const double proj_x = a.x + t * abx;
        const double proj_y = a.y + t * aby;
        const double dx = x_m - proj_x;
        const double dy = y_m - proj_y;
        min_distance = std::min(min_distance, std::sqrt(dx * dx + dy * dy));
    }

    return min_distance == std::numeric_limits<double>::max() ? 0.0 : min_distance;
}

}  // namespace fleetsim::app
