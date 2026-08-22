#include "domain/scenario/ScenarioLoader.h"
#include "domain/vehicle/Vehicle.h"
#include "domain/vehicle/VehicleModelFactory.h"
#include "domain/control/PurePursuitTracker.h"

#include <filesystem>
#include <gtest/gtest.h>
#include <vector>

using fleetsim::core::ControlCommand;
using fleetsim::core::Path;
using fleetsim::core::Pose;
using fleetsim::core::Waypoint;
using fleetsim::domain::control::PurePursuitTracker;
using fleetsim::domain::scenario::ScenarioLoader;
using fleetsim::domain::vehicle::Vehicle;

namespace {

std::filesystem::path findBicycleDemoDir()
{
    const std::filesystem::path candidates[] = {
        std::filesystem::path("assets/scenarios/bicycle_demo"),
        std::filesystem::path("../assets/scenarios/bicycle_demo"),
        std::filesystem::path("../../assets/scenarios/bicycle_demo"),
        std::filesystem::path("../../../assets/scenarios/bicycle_demo"),
        std::filesystem::path("../../../../assets/scenarios/bicycle_demo"),
    };
    for (const auto& path : candidates) {
        if (std::filesystem::exists(path / "scenario.json")) {
            return path;
        }
    }
    return {};
}

}  // namespace

TEST(BicycleScenarioTest, LoadsBicycleModelField)
{
    const auto dir = findBicycleDemoDir();
    if (dir.empty()) {
        GTEST_SKIP() << "bicycle_demo assets not found from cwd";
    }

    const auto data = ScenarioLoader::loadFromDirectory(dir.string());
    ASSERT_FALSE(data.vehicles.empty());
    EXPECT_EQ(data.vehicles.front().model, "bicycle");
    EXPECT_NEAR(data.vehicles.front().wheelbase_m, 0.9, 1e-9);
}

TEST(BicycleScenarioTest, PlanTicksMovePoseWithSteering)
{
    auto model = fleetsim::domain::vehicle::createVehicleModel("bicycle", 0.5, 1.0, 0.9, 0.6);
    Vehicle vehicle("car_0", 1.2, Pose{0.0, 0.0, 0.0}, std::move(model));
    vehicle.setModelKind("bicycle");
    vehicle.setWheelbaseM(0.9);
    vehicle.setMaxSteeringRad(0.6);

    std::vector<Waypoint> points;
    for (int i = 0; i <= 40; ++i) {
        const double t = static_cast<double>(i) * 0.25;
        points.push_back({t, 0.05 * t * t});
    }
    Path path(points);

    PurePursuitTracker tracker;
    const Pose start = vehicle.pose();
    for (int i = 0; i < 80; ++i) {
        const ControlCommand cmd = tracker.compute(
            vehicle.pose(), path, 0.05, vehicle.wheelbaseM(), vehicle.maxSteeringRad());
        vehicle.integrate(cmd, 0.05);
    }

    EXPECT_GT(vehicle.pose().x, start.x + 0.5);
    EXPECT_NE(vehicle.pose().theta, start.theta);
}
