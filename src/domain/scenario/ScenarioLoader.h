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
};

struct VehicleConfig {
    core::VehicleId id;
    std::string model;
    std::string svg_path;
    double length_m{1.0};
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
