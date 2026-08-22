#include "core/types/TimeWindow.h"

#include <gtest/gtest.h>

using fleetsim::core::TimeWindow;

TEST(TimeWindowTest, OverlapsWhenSharedInterval)
{
    TimeWindow a{0.0, 1.0};
    TimeWindow b{0.5, 1.5};
    EXPECT_TRUE(a.overlaps(b));
}

TEST(TimeWindowTest, NoOverlapWhenSeparated)
{
    TimeWindow a{0.0, 1.0};
    TimeWindow b{1.1, 2.0};
    EXPECT_FALSE(a.overlaps(b));
}

TEST(TimeWindowTest, ContainsTimeInsideWindow)
{
    TimeWindow window{2.0, 4.0};
    EXPECT_TRUE(window.contains(3.0));
    EXPECT_FALSE(window.contains(5.0));
}
