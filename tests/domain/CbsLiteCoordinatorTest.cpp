#include "domain/collision/CbsLiteCoordinator.h"
#include "domain/map/OccupancyGrid.h"
#include "domain/vehicle/FleetManager.h"
#include "domain/vehicle/Vehicle.h"

#include <gtest/gtest.h>

#include <memory>

using fleetsim::core::Path;
using fleetsim::core::Pose;
using fleetsim::core::VehicleId;
using fleetsim::core::Waypoint;
using fleetsim::domain::collision::CbsLiteConfig;
using fleetsim::domain::collision::CbsLiteCoordinator;
using fleetsim::domain::map::OccupancyGrid;
using fleetsim::domain::vehicle::FleetManager;
using fleetsim::domain::vehicle::Vehicle;

namespace {

Path straightPath(double y, double x0, double x1, int steps)
{
    std::vector<Waypoint> points;
    for (int i = 0; i <= steps; ++i) {
        const double t = static_cast<double>(i) / static_cast<double>(steps);
        points.push_back(Waypoint{x0 + t * (x1 - x0), y});
    }
    return Path(std::move(points));
}

OccupancyGrid makeOpenGrid(int rows = 40, int cols = 40, double res = 0.25)
{
    OccupancyGrid grid(rows, cols, res, cols * res, rows * res);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            grid.setOccupied(r, c, false);
        }
    }
    return grid;
}

}  // namespace

TEST(CbsLiteCoordinatorTest, HeadOnConflictProducesConstraints)
{
    FleetManager fleet;
    fleet.addVehicle(std::make_unique<Vehicle>(VehicleId{"agv_0"}, 1.0, Pose{0.0, 2.0, 0.0}));
    fleet.addVehicle(std::make_unique<Vehicle>(VehicleId{"agv_1"}, 1.0, Pose{10.0, 2.0, 3.141592653589793}));
    fleet.agent(0).reference_path = straightPath(2.0, 0.0, 10.0, 10);
    fleet.agent(1).reference_path = straightPath(2.0, 10.0, 0.0, 10);

    CbsLiteConfig config;
    config.max_depth = 5;
    const auto result = CbsLiteCoordinator::resolve(fleet, makeOpenGrid(), config);

    EXPECT_TRUE(result.success);
    EXPECT_GT(result.constraints_added, 0);
    EXPECT_EQ(result.paths.size(), 2U);
}

TEST(CbsLiteCoordinatorTest, ZeroDepthFailsImmediately)
{
    FleetManager fleet;
    fleet.addVehicle(std::make_unique<Vehicle>(VehicleId{"agv_0"}, 1.0, Pose{0.0, 2.0, 0.0}));
    fleet.addVehicle(std::make_unique<Vehicle>(VehicleId{"agv_1"}, 1.0, Pose{10.0, 2.0, 3.141592653589793}));
    fleet.agent(0).reference_path = straightPath(2.0, 0.0, 10.0, 10);
    fleet.agent(1).reference_path = straightPath(2.0, 10.0, 0.0, 10);

    CbsLiteConfig config;
    config.max_depth = 0;
    const auto result = CbsLiteCoordinator::resolve(fleet, makeOpenGrid(), config);

    EXPECT_FALSE(result.success);
}
