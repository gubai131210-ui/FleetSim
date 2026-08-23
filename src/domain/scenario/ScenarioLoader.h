#pragma once

#include "core/types/Pose.h"
#include "core/types/Task.h"
#include "core/types/VehicleId.h"
#include "domain/map/LaneTypes.h"
#include "domain/map/OccupancyGrid.h"

#include <string>
#include <vector>

namespace fleetsim::domain::scenario {

struct SimulationConfig {
    double dt_s{0.05};
    bool realtime{false};
    /// "astar" | "hybrid_astar" | empty/"auto" (bicycle → hybrid_astar, else astar).
    std::string planner;
    /// "pure_pursuit" | "stanley" | "mpc" | empty/"auto" (mpc never from auto).
    std::string tracker;
    /// "priority" | "none" | empty → priority (ADR-013).
    std::string coordination;
    /// "st_graph" | "none" | empty → none (ADR-015).
    std::string speed_planner;
    /// "none" | "constant_velocity" | empty → none (ADR-016).
    std::string prediction;
    /// "freespace" | "lane_graph" | "hybrid" | empty → freespace (ADR-018).
    std::string routing_mode;
};

struct VehicleConfig {
    core::VehicleId id;
    std::string model;
    std::string svg_path;
    double length_m{1.0};
    double wheelbase_m{0.8};       // used when model == "bicycle"
    double max_steering_rad{0.6};  // used when model == "bicycle"
    core::Pose initial_pose;
};

struct ScenarioData {
    SimulationConfig simulation;
    std::vector<VehicleConfig> vehicles;
    std::vector<core::Task> tasks;
    map::OccupancyGrid map;
    map::LaneMapData lanes;
    std::string scenario_directory;
};

class ScenarioLoader {
public:
    static ScenarioData loadFromDirectory(const std::string& scenario_directory,
                                          double inflation_radius_m = 0.55);
};

}  // namespace fleetsim::domain::scenario
