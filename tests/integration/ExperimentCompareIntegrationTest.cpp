#include "domain/SimEngine.h"
#include "domain/experiment/ExperimentMetrics.h"
#include "domain/scenario/ScenarioLoader.h"
#include "domain/vehicle/Vehicle.h"
#include "domain/vehicle/VehicleModelFactory.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <vector>

using fleetsim::core::Path;
using fleetsim::core::Pose;
using fleetsim::core::Waypoint;
using fleetsim::domain::SimEngine;
using fleetsim::domain::experiment::ExperimentMetrics;
using fleetsim::domain::experiment::TickSample;
using fleetsim::domain::scenario::ScenarioLoader;
using fleetsim::domain::vehicle::Vehicle;

namespace {

std::string predictionStDemoDir()
{
    const std::filesystem::path source_dir =
        std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    return (source_dir / "assets" / "scenarios" / "prediction_st_demo").string();
}

Path makePath(std::initializer_list<Waypoint> pts)
{
    return Path(std::vector<Waypoint>(pts));
}

double minSpeed(const std::vector<double>& speeds)
{
    return *std::min_element(speeds.begin(), speeds.end());
}

}  // namespace

TEST(ExperimentCompareIntegrationTest, PredictionStDemoScenarioLoadsTwoVehicles)
{
    const auto scenario = ScenarioLoader::loadFromDirectory(predictionStDemoDir());
    EXPECT_EQ(scenario.simulation.speed_planner, "st_graph");
    EXPECT_EQ(scenario.simulation.prediction, "constant_velocity");
    ASSERT_EQ(scenario.vehicles.size(), 2u);
}

TEST(ExperimentCompareIntegrationTest, MetricsAggregateAfterSimTicks)
{
    const auto scenario = ScenarioLoader::loadFromDirectory(predictionStDemoDir());

    SimEngine engine;
    engine.setMap(scenario.map);
    engine.setSpeedPlannerKind(scenario.simulation.speed_planner);
    engine.setPredictionKind(scenario.simulation.prediction);
    engine.setTrackerKind("stanley");
    engine.setCoordinationKind("none");

    for (const auto& cfg : scenario.vehicles) {
        auto model = fleetsim::domain::vehicle::createVehicleModel(
            cfg.model, 0.5, 1.0, cfg.wheelbase_m, cfg.max_steering_rad);
        auto vehicle = std::make_unique<Vehicle>(
            cfg.id, cfg.length_m, cfg.initial_pose, std::move(model));
        vehicle->setModelKind(cfg.model);
        engine.addVehicle(std::move(vehicle));
    }

    engine.fleet().agent(0).reference_path =
        makePath({{1.2, 2.5}, {2.5, 2.5}, {4.0, 2.5}, {5.5, 2.5}, {7.0, 2.5}});
    engine.fleet().agent(1).reference_path =
        makePath({{6.5, 2.5}, {5.0, 2.5}, {4.0, 2.5}, {3.0, 2.5}});
    engine.refreshSpeedProfiles();

    ExperimentMetrics metrics;
    for (int i = 0; i < 20; ++i) {
        engine.tick(0.05);
        const auto& agent = engine.fleet().agent(0);
        metrics.recordTick(TickSample{
            0.05,
            0.02,
            agent.linear_velocity,
            agent.speed_profile.speeds.empty() ? 0.5 : agent.speed_profile.speeds.front(),
            agent.last_mpc_solve_ok});
    }

    const auto summary = metrics.summarize();
    EXPECT_GE(summary.sample_count, 10u);
    EXPECT_GT(summary.mean_abs_cross_track, 0.0);
}

TEST(ExperimentCompareIntegrationTest, LoadedPredictionChangesStProfile)
{
    const auto scenario = ScenarioLoader::loadFromDirectory(predictionStDemoDir());

    SimEngine engine;
    engine.setMap(scenario.map);
    engine.setSpeedPlannerKind("st_graph");
    engine.setCoordinationKind("none");

    auto model_a = fleetsim::domain::vehicle::createVehicleModel("bicycle", 0.5, 1.0, 0.8, 0.6);
    auto ego = std::make_unique<Vehicle>("ego", 1.0, Pose{0.0, 0.0, 0.0}, std::move(model_a));
    ego->setModelKind("bicycle");
    engine.addVehicle(std::move(ego));

    auto model_b = fleetsim::domain::vehicle::createVehicleModel("bicycle", 0.5, 1.0, 0.8, 0.6);
    auto peer = std::make_unique<Vehicle>(
        "peer", 1.0, Pose{2.0, -1.0, 1.5707963267948966}, std::move(model_b));
    peer->setModelKind("bicycle");
    engine.addVehicle(std::move(peer));

    engine.fleet().agent(0).reference_path =
        makePath({{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}});
    engine.fleet().agent(1).reference_path =
        makePath({{2, -1}, {3, -1}, {4, -1}, {5, -1}});

    engine.setPredictionKind("none");
    engine.refreshSpeedProfiles();
    const double min_none = minSpeed(engine.fleet().agent(0).speed_profile.speeds);

    engine.setPredictionKind(scenario.simulation.prediction);
    engine.refreshSpeedProfiles();
    const double min_cv = minSpeed(engine.fleet().agent(0).speed_profile.speeds);

    EXPECT_LT(min_cv, min_none - 1e-3);
}
