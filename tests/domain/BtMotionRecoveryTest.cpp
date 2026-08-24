#include "domain/behavior/BtBlackboard.h"
#include "domain/behavior/BtMotionRecoveryNodes.h"
#include "domain/behavior/BtNode.h"
#include "domain/behavior/BtTypes.h"
#include "domain/behavior/IBtSimContext.h"
#include "domain/map/OccupancyGrid.h"

#include <gtest/gtest.h>

#include <cmath>

using fleetsim::core::Pose;
using fleetsim::domain::behavior::BtBackUpRecoveryNode;
using fleetsim::domain::behavior::BtBlackboard;
using fleetsim::domain::behavior::BtClearInflationNode;
using fleetsim::domain::behavior::BtSpinRecoveryNode;
using fleetsim::domain::behavior::IBtSimContext;
using fleetsim::domain::behavior::NodeStatus;
using fleetsim::domain::map::OccupancyGrid;

namespace BbKey = fleetsim::domain::behavior::BbKey;

namespace {

class MockMotionSimContext final : public IBtSimContext {
public:
    bool planPathForAgent() override { return false; }
    bool hasValidPath() const override { return false; }
    bool isGoalReached() const override { return false; }
    bool isGoalUpdated() override { return false; }
    void acknowledgeGoal() override {}
    bool hasReservationConflict() const override { return false; }
    void requestReplan() override {}
    bool needsReplan() const override { return false; }
    void clearReplanRequest() override {}
    double simDt() const override { return 0.05; }

    Pose agentPose() const override { return pose_; }
    void applyYawDelta(double delta_rad) override { pose_.theta += delta_rad; }

    void applyBodyTranslation(double forward_m, double lateral_m) override
    {
        const double cos_theta = std::cos(pose_.theta);
        const double sin_theta = std::sin(pose_.theta);
        pose_.x += forward_m * cos_theta - lateral_m * sin_theta;
        pose_.y += forward_m * sin_theta + lateral_m * cos_theta;
    }

    bool clearInflationLayer() override
    {
        if (grid_ == nullptr || !grid_->hasInflationBase()) {
            return false;
        }
        grid_->clearInflation();
        return true;
    }

    std::size_t occupiedCellCount() const override
    {
        return grid_ != nullptr ? grid_->occupiedCellCount() : 0U;
    }

    Pose pose_{0.0, 0.0, 0.0};
    OccupancyGrid* grid_{nullptr};
};

NodeStatus tickUntilTerminal(fleetsim::domain::behavior::BtNode& node,
                             BtBlackboard& bb,
                             int max_ticks = 200)
{
    NodeStatus status = NodeStatus::Running;
    for (int tick = 0; tick < max_ticks && status == NodeStatus::Running; ++tick) {
        status = node.tick(bb);
    }
    return status;
}

OccupancyGrid makeInflatedGrid()
{
    OccupancyGrid grid(20, 20, 0.1, 2.0, 2.0);
    grid.setOccupied(10, 10, true);
    grid.inflate(0.3);
    return grid;
}

}  // namespace

TEST(BtMotionRecoveryTest, SpinChangesYaw)
{
    MockMotionSimContext context;
    BtBlackboard bb;
    bb.setSimContext(&context);
    bb.setString(BbKey::kAgentId, "agv_0");

    const double yaw0 = context.pose_.theta;
    BtSpinRecoveryNode spin("Spin", 0.5);
    EXPECT_EQ(tickUntilTerminal(spin, bb), NodeStatus::Success);
    EXPECT_GE(std::abs(context.pose_.theta - yaw0), 0.45);
}

TEST(BtMotionRecoveryTest, BackUpMovesAlongRearHeading)
{
    MockMotionSimContext context;
    context.pose_.theta = 0.0;
    BtBlackboard bb;
    bb.setSimContext(&context);
    bb.setString(BbKey::kAgentId, "agv_0");

    BtBackUpRecoveryNode backup("BackUp", 0.3, 0.1);
    EXPECT_EQ(tickUntilTerminal(backup, bb), NodeStatus::Success);
    EXPECT_LE(context.pose_.x, -0.25);
    EXPECT_NEAR(context.pose_.y, 0.0, 0.05);
}

TEST(BtMotionRecoveryTest, ClearInflationHasSideEffect)
{
    OccupancyGrid grid = makeInflatedGrid();
    const std::size_t occupied_before = grid.occupiedCellCount();
    ASSERT_GT(occupied_before, 1U);

    MockMotionSimContext context;
    context.grid_ = &grid;
    BtBlackboard bb;
    bb.setSimContext(&context);
    bb.setString(BbKey::kAgentId, "agv_0");

    BtClearInflationNode clear_node("ClearInflation");
    EXPECT_EQ(clear_node.tick(bb), NodeStatus::Success);
    EXPECT_LT(grid.occupiedCellCount(), occupied_before);
    ASSERT_TRUE(bb.getBool(BbKey::kInflationCleared).has_value());
    EXPECT_TRUE(bb.getBool(BbKey::kInflationCleared).value());
}
