#include "domain/collision/PathReservationTable.h"

#include <gtest/gtest.h>

using fleetsim::core::TimeWindow;
using fleetsim::domain::collision::PathReservationTable;

TEST(PathReservationTableTest, DetectsConflictForSameCellAndTime)
{
    PathReservationTable table;
    TimeWindow window{1.0, 2.0};
    table.addReservation(3, 4, "agv_0", window, 1);

    EXPECT_TRUE(table.hasConflict(3, 4, window, "agv_1", 0));
    EXPECT_FALSE(table.hasConflict(3, 4, window, "agv_0", 1));
}

TEST(PathReservationTableTest, ReturnsSlowdownScaleOnConflict)
{
    PathReservationTable table;
    TimeWindow window{0.0, 1.0};
    table.addReservation(1, 1, "agv_0", window, 2);

    const double scale = table.speedScaleFor(1, 1, window, "agv_1", 1, 0.3);
    EXPECT_DOUBLE_EQ(scale, 0.3);
}
