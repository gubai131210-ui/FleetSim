#include "core/EventBus.h"

#include <gtest/gtest.h>

using fleetsim::core::EventBus;

TEST(EventBus, PublishDeliveredToSubscriber)
{
    EventBus bus;
    int count = 0;
    bus.subscribe("test/topic", [&count](const std::string& payload) {
        ++count;
        EXPECT_EQ(payload, "hello");
    });

    bus.publish("test/topic", "hello");
    EXPECT_EQ(count, 1);
}

TEST(EventBus, UnsubscribeStopsDelivery)
{
    EventBus bus;
    int count = 0;
    const int id = bus.subscribe("test/topic", [&count](const std::string&) {
        ++count;
    });

    bus.unsubscribe("test/topic", id);
    bus.publish("test/topic");
    EXPECT_EQ(count, 0);
}
