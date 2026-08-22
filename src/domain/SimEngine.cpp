#include "SimEngine.h"

#include <nlohmann/json.hpp>

#include <cmath>

namespace fleetsim::domain {

SimEngine::SimEngine() = default;

core::SimClock& SimEngine::clock()
{
    return clock_;
}

const core::SimClock& SimEngine::clock() const
{
    return clock_;
}

core::EventBus& SimEngine::eventBus()
{
    return event_bus_;
}

const core::EventBus& SimEngine::eventBus() const
{
    return event_bus_;
}

scheduling::SchedulingModule& SimEngine::scheduling()
{
    return scheduling_;
}

const scheduling::SchedulingModule& SimEngine::scheduling() const
{
    return scheduling_;
}

collision::CollisionModule& SimEngine::collision()
{
    return collision_;
}

const collision::CollisionModule& SimEngine::collision() const
{
    return collision_;
}

void SimEngine::setMap(map::OccupancyGrid map)
{
    map_ = std::move(map);
}

const map::OccupancyGrid& SimEngine::map() const
{
    return map_;
}

void SimEngine::setVehicle(std::unique_ptr<vehicle::Vehicle> vehicle)
{
    vehicle_ = std::move(vehicle);
    goal_reached_ = false;
    goal_reached_published_ = false;
}

const vehicle::Vehicle* SimEngine::vehicle() const
{
    return vehicle_.get();
}

void SimEngine::setGoal(const core::Pose& goal)
{
    goal_ = goal;
    goal_reached_ = false;
    goal_reached_published_ = false;
}

const core::Pose& SimEngine::goal() const
{
    return goal_;
}

bool SimEngine::planPath()
{
    if (vehicle_ == nullptr || map_.rows() == 0) {
        return false;
    }

    const core::Path raw_path = planner_.plan(map_, vehicle_->pose(), goal_);
    if (raw_path.empty()) {
        reference_path_.clear();
        publishPathUpdate();
        return false;
    }

    reference_path_ = smoother_.smooth(raw_path);
    goal_reached_ = false;
    goal_reached_published_ = false;
    publishPathUpdate();
    return !reference_path_.empty();
}

const core::Path& SimEngine::referencePath() const
{
    return reference_path_;
}

void SimEngine::publishPoseUpdate()
{
    if (vehicle_ == nullptr) {
        return;
    }

    nlohmann::json payload;
    payload["id"] = vehicle_->id();
    payload["x"] = vehicle_->pose().x;
    payload["y"] = vehicle_->pose().y;
    payload["theta"] = vehicle_->pose().theta;
    event_bus_.publish("sim/pose_updated", payload.dump());
}

void SimEngine::publishPathUpdate()
{
    nlohmann::json payload = nlohmann::json::array();
    for (const core::Waypoint& waypoint : reference_path_.waypoints()) {
        payload.push_back({{"x", waypoint.x}, {"y", waypoint.y}});
    }
    event_bus_.publish("sim/path_updated", payload.dump());
}

void SimEngine::tick(double dt)
{
    scheduling_.tick(dt);
    collision_.tick(dt);

    if (vehicle_ != nullptr && !reference_path_.empty() && !goal_reached_) {
        const core::ControlCommand command = tracker_.compute(vehicle_->pose(), reference_path_, dt);
        vehicle_->integrate(command, dt);
        publishPoseUpdate();

        const core::Waypoint goal_point = reference_path_.waypoints().back();
        const double dx = goal_point.x - vehicle_->pose().x;
        const double dy = goal_point.y - vehicle_->pose().y;
        const double distance = std::sqrt(dx * dx + dy * dy);
        if (distance <= 0.2) {
            goal_reached_ = true;
            if (!goal_reached_published_) {
                event_bus_.publish("sim/goal_reached", vehicle_->id());
                goal_reached_published_ = true;
            }
        }
    }

    ++tick_count_;
    event_bus_.publish("sim/tick", std::to_string(tick_count_));
}

int SimEngine::tickCount() const
{
    return tick_count_;
}

bool SimEngine::goalReached() const
{
    return goal_reached_;
}

}  // namespace fleetsim::domain
