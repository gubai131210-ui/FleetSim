#include "domain/scheduling/SchedulingModule.h"

#include <gtest/gtest.h>

using fleetsim::domain::scheduling::SchedulingModule;

TEST(SchedulingModule, CanInstantiateAndTick)
{
    SchedulingModule module;
    module.tick(0.05);
    EXPECT_GE(module.pendingTaskCount(), 0);
}
