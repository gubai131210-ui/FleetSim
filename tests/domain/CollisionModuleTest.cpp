#include "domain/collision/CollisionModule.h"

#include <gtest/gtest.h>

using fleetsim::domain::collision::CollisionModule;

TEST(CollisionModule, CanInstantiateAndTick)
{
    CollisionModule module;
    module.tick(0.05);
    EXPECT_GE(module.conflictCount(), 0);
}
