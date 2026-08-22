#include "core/SimClock.h"

#include <gtest/gtest.h>

using fleetsim::core::SimClock;
using fleetsim::core::SimClockMode;

TEST(SimClock, FixedStepTickAdvancesTime)
{
    SimClock clock(0.05);
    EXPECT_DOUBLE_EQ(clock.tick(), 0.05);
    EXPECT_DOUBLE_EQ(clock.simTime(), 0.05);
    EXPECT_EQ(clock.tickCount(), 1);
}

TEST(SimClock, PauseStopsAdvance)
{
    SimClock clock(0.1);
    clock.pause();
    EXPECT_DOUBLE_EQ(clock.tick(), 0.0);
    EXPECT_DOUBLE_EQ(clock.simTime(), 0.0);
}

TEST(SimClock, TimeScaleApplies)
{
    SimClock clock(0.1);
    clock.setTimeScale(2.0);
    EXPECT_DOUBLE_EQ(clock.tick(), 0.2);
}
