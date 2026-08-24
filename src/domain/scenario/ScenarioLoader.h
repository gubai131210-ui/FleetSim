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
    double lane_snap_radius_m{1.0};
    /// empty → use Planning planner for hybrid first/last mile.
    std::string first_last_planner;
    /// "legacy" | "bt" — default legacy (ADR-020).
    std::string behavior_mode{"legacy"};
    /// Relative to scenario dir or assets/behavior_trees/.
    std::string behavior_tree_path;
    /// "json" | "xml" — default json (ADR-023).
    std::string bt_format{"json"};
    double replan_hz{1.0};
    int recovery_wait_ticks{20};
    int cbs_max_depth{10};
    int cbs_time_limit_ms{100};
};

struct VehicleConfig {
    core::VehicleId id;
    std::string model;
    std::string svg_path;
    double length_m{1.0};
    double wheelbase_m{0.8};       // used when model == "bicycle"
    double max_steering_rad{0.6};  // used when model == "bicycle"
    core::Pose initial_pose;
    /// Optional per-vehicle BT asset; falls back to simulation.behavior_tree_path (ADR-025).
    std::string behavior_tree_path;
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
