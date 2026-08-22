#pragma once

#include "core/types/Pose.h"
#include "core/types/Task.h"
#include "core/types/VehicleId.h"
#include "domain/map/OccupancyGrid.h"

#include <string>
#include <vector>

namespace fleetsim::domain::scenario {

struct SimulationConfig {
    double dt_s{0.05};
    bool realtime{false};
    /// "astar" | "hybrid_astar" | empty/"auto" (bicycle → hybrid_astar, else astar).
    std::string planner;
    /// "pure_pursuit" | "stanley" | empty/"auto" (Session 3 wires stanley).
    std::string tracker;
    /// "priority" | "none" | empty → priority (ADR-013).
    std::string coordination;
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
    std::string scenario_directory;
};

class ScenarioLoader {
public:
    static ScenarioData loadFromDirectory(const std::string& scenario_directory,
                                          double inflation_radius_m = 0.55);
};

}  // namespace fleetsim::domain::scenario
