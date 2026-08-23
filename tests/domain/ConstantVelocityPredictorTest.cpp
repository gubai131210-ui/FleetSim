#include "domain/prediction/ConstantVelocityPredictor.h"

#include <gtest/gtest.h>

#include <cmath>

using fleetsim::core::Path;
using fleetsim::core::Pose;
using fleetsim::domain::prediction::ConstantVelocityPredictor;

TEST(ConstantVelocityPredictorTest, StraightLineExtrapolationLength)
{
    ConstantVelocityPredictor predictor;
    const Pose start{0.0, 0.0, 0.0};
    constexpr double v = 0.5;
    constexpr double horizon = 3.0;
    constexpr double dt = 0.1;

    const Path predicted = predictor.predictPath(start, v, horizon, dt);

    ASSERT_FALSE(predicted.empty()) << "Session 1: CV predictor must emit samples";
    EXPECT_GE(predicted.size(), static_cast<std::size_t>(horizon / dt));
    EXPECT_NEAR(predicted.length(), v * horizon, 0.15)
        << "CV path length should approximate v * T along heading";
}

TEST(ConstantVelocityPredictorTest, TurningNominalSpeedUsesHeading)
{
    ConstantVelocityPredictor predictor;
    const Pose start{1.0, 2.0, 1.5707963267948966};
    constexpr double v = 0.4;
    constexpr double horizon = 2.0;
    constexpr double dt = 0.1;

    const Path predicted = predictor.predictPath(start, v, horizon, dt);
    ASSERT_GE(predicted.size(), 2u);

    const auto& end = predicted.waypoints().back();
    EXPECT_NEAR(end.x, start.x, 0.2);
    EXPECT_GT(end.y, start.y);
}

TEST(ConstantVelocityPredictorTest, ZeroHorizonYieldsMinimalPath)
{
    ConstantVelocityPredictor predictor;
    const Path predicted = predictor.predictPath(Pose{}, 0.5, 0.0, 0.1);
    EXPECT_GE(predicted.size(), 1u);
}
