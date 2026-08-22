#include "domain/scenario/ScenarioSerializer.h"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

using fleetsim::domain::scenario::ScenarioData;
using fleetsim::domain::scenario::ScenarioSerializer;
using fleetsim::domain::scenario::VehicleConfig;

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
