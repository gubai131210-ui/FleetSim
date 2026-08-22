#include "domain/SimEngine.h"

#include <gtest/gtest.h>

using fleetsim::domain::SimEngine;

TEST(SimEngine, CanTick)
{
    SimEngine engine;
    engine.tick(0.05);
    EXPECT_EQ(engine.tickCount(), 1);
}

TEST(SimEngine, PublishesTickEvent)
{
    SimEngine engine;
    int events = 0;
    engine.eventBus().subscribe("sim/tick", [&events](const std::string&) {
        ++events;
    });

    engine.tick(0.05);
    engine.tick(0.05);
    EXPECT_EQ(events, 2);
}
