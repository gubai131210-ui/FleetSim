#pragma once

#include "collision/CollisionModule.h"
#include "control/PurePursuitTracker.h"
#include "planning/AStarPlanner.h"
#include "planning/DouglasPeuckerSmoother.h"
#include "scheduling/SchedulingModule.h"
#include "vehicle/Vehicle.h"

#include "core/EventBus.h"
#include "core/SimClock.h"
#include "core/types/Path.h"
#include "core/types/Pose.h"
#include "domain/map/OccupancyGrid.h"

#include <memory>
#include <optional>

namespace fleetsim::domain {

class SimEngine {
public:
    SimEngine();

    core::SimClock& clock();
    const core::SimClock& clock() const;

    core::EventBus& eventBus();
    const core::EventBus& eventBus() const;

    scheduling::SchedulingModule& scheduling();
    const scheduling::SchedulingModule& scheduling() const;

    collision::CollisionModule& collision();
    const collision::CollisionModule& collision() const;

    void setMap(map::OccupancyGrid map);
    const map::OccupancyGrid& map() const;

    void setVehicle(std::unique_ptr<vehicle::Vehicle> vehicle);
    const vehicle::Vehicle* vehicle() const;

    void setGoal(const core::Pose& goal);
    const core::Pose& goal() const;

    bool planPath();
    const core::Path& referencePath() const;

    void tick(double dt);

    int tickCount() const;
    bool goalReached() const;

private:
    void publishPoseUpdate();
    void publishPathUpdate();

    core::SimClock clock_;
    core::EventBus event_bus_;
    scheduling::SchedulingModule scheduling_;
    collision::CollisionModule collision_;

    map::OccupancyGrid map_;
    std::unique_ptr<vehicle::Vehicle> vehicle_;

    planning::AStarPlanner planner_;
    planning::DouglasPeuckerSmoother smoother_;
    control::PurePursuitTracker tracker_;

    core::Path reference_path_;
    core::Pose goal_;
    bool goal_reached_{false};
    bool goal_reached_published_{false};
    int tick_count_{0};
};

}  // namespace fleetsim::domain
