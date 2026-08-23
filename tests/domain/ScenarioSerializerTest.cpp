#include "domain/scenario/ScenarioSerializer.h"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

using fleetsim::core::Task;
using fleetsim::domain::scenario::ScenarioData;
using fleetsim::domain::scenario::ScenarioSerializer;
using fleetsim::domain::scenario::VehicleConfig;

TEST(ScenarioSerializerTest, RoundTripPlannerTrackerFields)
{
    ScenarioData scenario;
    scenario.simulation.dt_s = 0.05;
    scenario.simulation.realtime = false;
    scenario.simulation.planner = "hybrid_astar";
    scenario.simulation.tracker = "stanley";

    VehicleConfig vehicle;
    vehicle.id = "car_0";
    vehicle.model = "bicycle";
    vehicle.svg_path = "assets/vehicles/agv_diff.svg";
    vehicle.wheelbase_m = 0.9;
    vehicle.max_steering_rad = 0.6;
    vehicle.initial_pose = {1.0, 2.0, 0.0};
    scenario.vehicles.push_back(vehicle);

    const nlohmann::json json = ScenarioSerializer::toJson(scenario);
    EXPECT_EQ(json["simulation"]["planner"], "hybrid_astar");
    EXPECT_EQ(json["simulation"]["tracker"], "stanley");

    const ScenarioData loaded = ScenarioSerializer::fromJson(json, "/tmp/scenario");
    EXPECT_EQ(loaded.simulation.planner, "hybrid_astar");
    EXPECT_EQ(loaded.simulation.tracker, "stanley");
}

TEST(ScenarioSerializerTest, RoundTripTrackerMpc)
{
    ScenarioData scenario;
    scenario.simulation.dt_s = 0.05;
    scenario.simulation.realtime = false;
    scenario.simulation.planner = "hybrid_astar";
    scenario.simulation.tracker = "mpc";

    const nlohmann::json json = ScenarioSerializer::toJson(scenario);
    EXPECT_EQ(json["simulation"]["tracker"], "mpc");

    const ScenarioData loaded = ScenarioSerializer::fromJson(json, "/tmp/scenario");
    EXPECT_EQ(loaded.simulation.tracker, "mpc");
}

TEST(ScenarioSerializerTest, RoundTripSpeedPlannerStGraph)
{
    ScenarioData scenario;
    scenario.simulation.dt_s = 0.05;
    scenario.simulation.realtime = false;
    scenario.simulation.speed_planner = "st_graph";

    const nlohmann::json json = ScenarioSerializer::toJson(scenario);
    EXPECT_EQ(json["simulation"]["speed_planner"], "st_graph");

    const ScenarioData loaded = ScenarioSerializer::fromJson(json, "/tmp/scenario");
    EXPECT_EQ(loaded.simulation.speed_planner, "st_graph");
}

TEST(ScenarioSerializerTest, RoundTripPredictionConstantVelocity)
{
    ScenarioData scenario;
    scenario.simulation.dt_s = 0.05;
    scenario.simulation.realtime = false;
    scenario.simulation.speed_planner = "st_graph";
    scenario.simulation.prediction = "constant_velocity";

    const nlohmann::json json = ScenarioSerializer::toJson(scenario);
    EXPECT_EQ(json["simulation"]["prediction"], "constant_velocity");

    const ScenarioData loaded = ScenarioSerializer::fromJson(json, "/tmp/scenario");
    EXPECT_EQ(loaded.simulation.prediction, "constant_velocity");
}

TEST(ScenarioSerializerTest, RoundTripRoutingModeHybrid)
{
    ScenarioData scenario;
    scenario.simulation.dt_s = 0.05;
    scenario.simulation.realtime = false;
    scenario.simulation.routing_mode = "hybrid";

    const nlohmann::json json = ScenarioSerializer::toJson(scenario);
    EXPECT_EQ(json["simulation"]["routing_mode"], "hybrid");

    const ScenarioData loaded = ScenarioSerializer::fromJson(json, "/tmp/scenario");
    EXPECT_EQ(loaded.simulation.routing_mode, "hybrid");
}

TEST(ScenarioSerializerTest, RoundTripVehicleConfig)
{
    ScenarioData scenario;
    scenario.simulation.dt_s = 0.05;
    scenario.simulation.realtime = false;

    VehicleConfig vehicle;
    vehicle.id = "agv_0";
    vehicle.model = "diff_drive";
    vehicle.svg_path = "assets/vehicles/agv_diff.svg";
    vehicle.length_m = 1.0;
    vehicle.initial_pose = {2.0, 3.0, 0.5};
    scenario.vehicles.push_back(vehicle);

    const nlohmann::json json = ScenarioSerializer::toJson(scenario);
    const ScenarioData loaded = ScenarioSerializer::fromJson(json, "/tmp/scenario");

    ASSERT_EQ(loaded.vehicles.size(), 1U);
    EXPECT_EQ(loaded.vehicles.front().id, "agv_0");
    EXPECT_DOUBLE_EQ(loaded.vehicles.front().initial_pose.y, 3.0);
    EXPECT_DOUBLE_EQ(loaded.simulation.dt_s, 0.05);
}

TEST(ScenarioSerializerTest, RequiresVehicleId)
{
    nlohmann::json json = {
        {"version", 1},
        {"simulation", {{"dt_s", 0.05}, {"realtime", false}}},
        {"vehicles",
         {{{"model", "diff_drive"},
           {"svg", "assets/vehicles/agv_diff.svg"},
           {"pose", {{"x", 1.0}, {"y", 2.0}, {"theta", 0.0}}}}}},
        {"tasks", nlohmann::json::array()},
    };

    EXPECT_THROW(ScenarioSerializer::fromJson(json, "/tmp/scenario"), nlohmann::json::exception);
}

TEST(ScenarioSerializerTest, RoundTripTasks)
{
    ScenarioData scenario;
    Task task;
    task.id = "task_0";
    task.pickup = {1.0, 2.0, 0.0};
    task.dropoff = {5.0, 6.0, 0.0};
    task.priority = 1;
    scenario.tasks.push_back(task);

    const nlohmann::json json = ScenarioSerializer::toJson(scenario);
    const ScenarioData loaded = ScenarioSerializer::fromJson(json, "/tmp/scenario");

    ASSERT_EQ(loaded.tasks.size(), 1U);
    EXPECT_EQ(loaded.tasks.front().id, "task_0");
    EXPECT_DOUBLE_EQ(loaded.tasks.front().dropoff.x, 5.0);
    EXPECT_EQ(loaded.tasks.front().priority, 1);
}

TEST(ScenarioSerializerTest, RoundTripBehaviorModeFields)
{
    ScenarioData scenario;
    scenario.simulation.behavior_mode = "bt";
    scenario.simulation.behavior_tree_path = "navigate_replan_recovery.json";
    scenario.simulation.replan_hz = 2.0;
    scenario.simulation.recovery_wait_ticks = 15;

    const nlohmann::json json = ScenarioSerializer::toJson(scenario);
    EXPECT_EQ(json["simulation"]["behavior_mode"], "bt");
    EXPECT_EQ(json["simulation"]["behavior_tree_path"], "navigate_replan_recovery.json");
    EXPECT_DOUBLE_EQ(json["simulation"]["replan_hz"].get<double>(), 2.0);
    EXPECT_EQ(json["simulation"]["recovery_wait_ticks"], 15);

    const ScenarioData loaded = ScenarioSerializer::fromJson(json, "/tmp/scenario");
    EXPECT_EQ(loaded.simulation.behavior_mode, "bt");
    EXPECT_EQ(loaded.simulation.behavior_tree_path, "navigate_replan_recovery.json");
    EXPECT_DOUBLE_EQ(loaded.simulation.replan_hz, 2.0);
    EXPECT_EQ(loaded.simulation.recovery_wait_ticks, 15);
}
