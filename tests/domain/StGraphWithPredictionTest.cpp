#include "domain/planning/StGraphSpeedPlanner.h"
#include "domain/prediction/ConstantVelocityPredictor.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>

using fleetsim::core::Path;
using fleetsim::core::Pose;
using fleetsim::core::SpeedProfile;
using fleetsim::core::Waypoint;
using fleetsim::domain::planning::PeerTrajectory;
using fleetsim::domain::planning::StGraphSpeedPlanner;
using fleetsim::domain::prediction::ConstantVelocityPredictor;

namespace {

Path makePath(std::initializer_list<Waypoint> pts)
{
    return Path(std::vector<Waypoint>(pts));
}

double minSpeed(const SpeedProfile& profile)
{
    if (profile.speeds.empty()) {
        return 0.0;
    }
    return *std::min_element(profile.speeds.begin(), profile.speeds.end());
}

}  // namespace

TEST(StGraphWithPredictionTest, PredictedPeerPathChangesStProfile)
{
    StGraphSpeedPlanner planner(0.5, 0.8, 0.1);
    const Path ego = makePath({{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}});

    // Static peer: short path crossing ego at x=2
    PeerTrajectory static_peer;
    static_peer.path = makePath({{2, -1}, {2, 0}, {2, 1}});
    static_peer.nominal_speed = 0.5;
    static_peer.from_prediction = false;

    const SpeedProfile static_profile = planner.plan(ego, {static_peer});

    // CV-predicted peer: extrapolate from pose along +y
    ConstantVelocityPredictor predictor;
    const Pose peer_pose{2.0, -1.0, 1.5707963267948966};
    PeerTrajectory predicted_peer;
    predicted_peer.path = predictor.predictPath(peer_pose, 0.5, 3.0, 0.1);
    predicted_peer.nominal_speed = 0.5;
    predicted_peer.from_prediction = true;

    ASSERT_FALSE(predicted_peer.path.empty())
        << "Session 1: predictor must produce path for ST projection";

    const SpeedProfile predicted_profile = planner.plan(ego, {predicted_peer});

    EXPECT_NE(minSpeed(static_profile), minSpeed(predicted_profile))
        << "ST profile must differ when peer path comes from CV prediction vs static path";
}
