#include "domain/SimEngine.h"
#include "domain/vehicle/Vehicle.h"
#include "domain/vehicle/VehicleModelFactory.h"
#include "domain/planning/StGraphSpeedPlanner.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <vector>

using fleetsim::core::Path;
using fleetsim::core::Pose;
using fleetsim::core::Waypoint;
using fleetsim::domain::SimEngine;
using fleetsim::domain::planning::StGraphSpeedPlanner;
using fleetsim::domain::vehicle::Vehicle;

namespace {

Path makePath(std::initializer_list<Waypoint> pts)
{
    return Path(std::vector<Waypoint>(pts));
}

double minSpeed(const std::vector<double>& speeds)
{
    return *std::min_element(speeds.begin(), speeds.end());
}

}  // namespace

TEST(StGraphSimEngineWiringTest, StGraphAfterPlanStoresEqualLengthProfile)
{
    SimEngine engine;
    engine.setSpeedPlannerKind("st_graph");
    engine.setCoordinationKind("none");
    engine.setTrackerKind("mpc");

    auto model_a = fleetsim::domain::vehicle::createVehicleModel("bicycle", 0.5, 1.0, 0.8, 0.6);
    auto a = std::make_unique<Vehicle>("ego", 1.0, Pose{0.0, 0.0, 0.0}, std::move(model_a));
    a->setModelKind("bicycle");
    a->setWheelbaseM(0.8);
    a->setMaxSteeringRad(0.6);
    engine.addVehicle(std::move(a));

    auto model_b = fleetsim::domain::vehicle::createVehicleModel("bicycle", 0.5, 1.0, 0.8, 0.6);
    auto b = std::make_unique<Vehicle>("peer", 1.0, Pose{3.0, -2.0, 1.57}, std::move(model_b));
    b->setModelKind("bicycle");
    engine.addVehicle(std::move(b));

    engine.fleet().agent(0).reference_path =
        makePath({{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}});
    engine.fleet().agent(1).reference_path =
        makePath({{3, -2}, {3, -1}, {3, 0}, {3, 1}, {3, 2}});

    engine.refreshSpeedProfiles();

    const auto& profile = engine.fleet().agent(0).speed_profile;
    ASSERT_EQ(profile.speeds.size(), engine.fleet().agent(0).reference_path.size());
    EXPECT_FALSE(profile.speeds.empty());
}

TEST(StGraphSimEngineWiringTest, WithPeersProfileDiffersFromEmptyPeers)
{
    // Hard contract: engine collectPeers must change profile vs empty peers.
    SimEngine engine;
    engine.setSpeedPlannerKind("st_graph");
    engine.setCoordinationKind("none");

    auto model_a = fleetsim::domain::vehicle::createVehicleModel("bicycle", 0.5, 1.0, 0.8, 0.6);
    auto a = std::make_unique<Vehicle>("ego", 1.0, Pose{0.0, 0.0, 0.0}, std::move(model_a));
    a->setModelKind("bicycle");
    engine.addVehicle(std::move(a));

    auto model_b = fleetsim::domain::vehicle::createVehicleModel("bicycle", 0.5, 1.0, 0.8, 0.6);
    auto b = std::make_unique<Vehicle>("peer", 1.0, Pose{3.0, -2.0, 1.57}, std::move(model_b));
    b->setModelKind("bicycle");
    engine.addVehicle(std::move(b));

    const Path ego = makePath({{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}});
    const Path peer = makePath({{3, -2}, {3, -1}, {3, 0}, {3, 1}, {3, 2}});
    engine.fleet().agent(0).reference_path = ego;
    engine.fleet().agent(1).reference_path = peer;

    engine.refreshSpeedProfiles();
    const auto with_peers = engine.fleet().agent(0).speed_profile;

    engine.fleet().agent(1).reference_path.clear();
    engine.refreshSpeedProfiles();
    const auto without_peers = engine.fleet().agent(0).speed_profile;

    ASSERT_EQ(with_peers.speeds.size(), without_peers.speeds.size());
    bool differs = false;
    for (std::size_t i = 0; i < with_peers.speeds.size(); ++i) {
        if (std::abs(with_peers.speeds[i] - without_peers.speeds[i]) > 1e-6) {
            differs = true;
            break;
        }
    }
    EXPECT_TRUE(differs) << "Ignoring peer Paths must FAIL this wiring contract";
    EXPECT_LT(minSpeed(with_peers.speeds), minSpeed(without_peers.speeds) - 1e-3);
}

TEST(StGraphSimEngineWiringTest, MpcTickUsesSetSpeedProfileNotOnlyScale)
{
    SimEngine engine;
    engine.setSpeedPlannerKind("st_graph");
    engine.setTrackerKind("mpc");
    engine.setCoordinationKind("none");

    // Start near the crossing so nearest ST sample is already decelerated.
    auto model_a = fleetsim::domain::vehicle::createVehicleModel("bicycle", 0.5, 1.0, 0.8, 0.6);
    auto a = std::make_unique<Vehicle>("ego", 1.0, Pose{2.5, 0.0, 0.0}, std::move(model_a));
    a->setModelKind("bicycle");
    a->setWheelbaseM(0.8);
    a->setMaxSteeringRad(0.6);
    engine.addVehicle(std::move(a));

    auto model_b = fleetsim::domain::vehicle::createVehicleModel("bicycle", 0.5, 1.0, 0.8, 0.6);
    auto b = std::make_unique<Vehicle>("peer", 1.0, Pose{3.0, -2.0, 0.0}, std::move(model_b));
    b->setModelKind("bicycle");
    engine.addVehicle(std::move(b));

    engine.fleet().agent(0).reference_path =
        makePath({{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}});
    engine.fleet().agent(1).reference_path =
        makePath({{3, -2}, {3, -1}, {3, 0}, {3, 1}, {3, 2}});

    engine.refreshSpeedProfiles();
    ASSERT_LT(minSpeed(engine.fleet().agent(0).speed_profile.speeds), 0.5 - 1e-3);

    engine.tick(0.05);

    EXPECT_LT(engine.fleet().agent(0).linear_velocity, 0.5 - 1e-3)
        << "ST+MPC should reduce v with speed_scale from TimeWindow (not scale-only fake ST)";
}

TEST(StGraphSimEngineWiringTest, TimeWindowScaleStacksOnStProfile)
{
    SimEngine engine;
    engine.setSpeedPlannerKind("st_graph");
    engine.setTrackerKind("stanley");
    engine.setCoordinationKind("none");

    auto model = fleetsim::domain::vehicle::createVehicleModel("bicycle", 0.5, 1.0, 0.8, 0.6);
    auto a = std::make_unique<Vehicle>("ego", 1.0, Pose{0.0, 0.0, 0.0}, std::move(model));
    a->setModelKind("bicycle");
    a->setWheelbaseM(0.8);
    engine.addVehicle(std::move(a));

    engine.fleet().agent(0).reference_path = makePath({{0, 0}, {1, 0}, {2, 0}, {3, 0}});
    engine.refreshSpeedProfiles();
    ASSERT_FALSE(engine.fleet().agent(0).speed_profile.speeds.empty());
    const double profile0 = engine.fleet().agent(0).speed_profile.speeds.front();

    engine.tick(0.05);
    // CollisionModule rewrites speed_scale each tick; stacking contract is v = profile * scale.
    EXPECT_NEAR(engine.fleet().agent(0).linear_velocity,
                profile0 * engine.fleet().agent(0).speed_scale,
                1e-3);
}

TEST(StGraphSimEngineWiringTest, SpeedPlannerNoneDoesNotFillStProfile)
{
    SimEngine engine;
    engine.setSpeedPlannerKind("none");
    engine.setCoordinationKind("none");

    auto model = fleetsim::domain::vehicle::createVehicleModel("bicycle", 0.5, 1.0, 0.8, 0.6);
    auto a = std::make_unique<Vehicle>("ego", 1.0, Pose{0.0, 0.0, 0.0}, std::move(model));
    a->setModelKind("bicycle");
    engine.addVehicle(std::move(a));

    engine.fleet().agent(0).reference_path = makePath({{0, 0}, {1, 0}, {2, 0}});
    engine.refreshSpeedProfiles();
    EXPECT_TRUE(engine.fleet().agent(0).speed_profile.speeds.empty());
}

TEST(StGraphSimEngineWiringTest, PredictionConstantVelocityChangesStProfile)
{
    SimEngine engine;
    engine.setSpeedPlannerKind("st_graph");
    engine.setCoordinationKind("none");

    auto model_a = fleetsim::domain::vehicle::createVehicleModel("bicycle", 0.5, 1.0, 0.8, 0.6);
    auto a = std::make_unique<Vehicle>("ego", 1.0, Pose{0.0, 0.0, 0.0}, std::move(model_a));
    a->setModelKind("bicycle");
    engine.addVehicle(std::move(a));

    auto model_b = fleetsim::domain::vehicle::createVehicleModel("bicycle", 0.5, 1.0, 0.8, 0.6);
    auto b = std::make_unique<Vehicle>("peer", 1.0, Pose{2.0, -1.0, 1.5707963267948966}, std::move(model_b));
    b->setModelKind("bicycle");
    engine.addVehicle(std::move(b));

    engine.fleet().agent(0).reference_path =
        makePath({{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}});
    // Static path stays below ego corridor (|lat|>0.75); CV extrapolation crosses at (2,0).
    engine.fleet().agent(1).reference_path =
        makePath({{2, -1}, {3, -1}, {4, -1}, {5, -1}});

    engine.setPredictionKind("none");
    engine.refreshSpeedProfiles();
    const auto profile_none = engine.fleet().agent(0).speed_profile;

    engine.setPredictionKind("constant_velocity");
    engine.refreshSpeedProfiles();
    const auto profile_cv = engine.fleet().agent(0).speed_profile;

    ASSERT_EQ(profile_none.speeds.size(), profile_cv.speeds.size());
    EXPECT_NEAR(minSpeed(profile_none.speeds), 0.5, 1e-3)
        << "Static peer path should not constrain ego ST profile";
    EXPECT_LT(minSpeed(profile_cv.speeds), minSpeed(profile_none.speeds) - 1e-3)
        << "CV prediction must feed crossing occupancy into ST";
}

TEST(StGraphSimEngineWiringTest, PredictionDefaultNoneMatchesPhase6PeerCollection)
{
    SimEngine engine;
    engine.setSpeedPlannerKind("st_graph");
    engine.setCoordinationKind("none");

    auto model_a = fleetsim::domain::vehicle::createVehicleModel("bicycle", 0.5, 1.0, 0.8, 0.6);
    auto a = std::make_unique<Vehicle>("ego", 1.0, Pose{0.0, 0.0, 0.0}, std::move(model_a));
    a->setModelKind("bicycle");
    engine.addVehicle(std::move(a));

    auto model_b = fleetsim::domain::vehicle::createVehicleModel("bicycle", 0.5, 1.0, 0.8, 0.6);
    auto b = std::make_unique<Vehicle>("peer", 1.0, Pose{3.0, -2.0, 1.57}, std::move(model_b));
    b->setModelKind("bicycle");
    engine.addVehicle(std::move(b));

    const Path ego = makePath({{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}});
    const Path peer = makePath({{3, -2}, {3, -1}, {3, 0}, {3, 1}, {3, 2}});
    engine.fleet().agent(0).reference_path = ego;
    engine.fleet().agent(1).reference_path = peer;

    EXPECT_EQ(engine.predictionKind(), "none");
    engine.refreshSpeedProfiles();
    const auto default_profile = engine.fleet().agent(0).speed_profile;

    engine.setPredictionKind("none");
    engine.refreshSpeedProfiles();
    const auto explicit_none = engine.fleet().agent(0).speed_profile;

    ASSERT_EQ(default_profile.speeds.size(), explicit_none.speeds.size());
    for (std::size_t i = 0; i < default_profile.speeds.size(); ++i) {
        EXPECT_NEAR(default_profile.speeds[i], explicit_none.speeds[i], 1e-9);
    }
}
