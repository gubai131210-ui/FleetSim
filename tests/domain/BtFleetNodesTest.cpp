#include "domain/behavior/BtBlackboard.h"
#include "domain/behavior/BtFleetActionNodes.h"
#include "domain/behavior/BtFleetConditionNodes.h"
#include "domain/behavior/BtTypes.h"
#include "domain/behavior/IBtSimContext.h"

#include <gtest/gtest.h>

using fleetsim::domain::behavior::BtBlackboard;
using fleetsim::domain::behavior::BtFollowUntilGoalNode;
using fleetsim::domain::behavior::BtIsGoalUpdatedNode;
using fleetsim::domain::behavior::BtPlanPathNode;
using fleetsim::domain::behavior::BtWaitRecoveryNode;
using fleetsim::domain::behavior::IBtSimContext;
using fleetsim::domain::behavior::NodeStatus;

namespace BbKey = fleetsim::domain::behavior::BbKey;
namespace BtActionName = fleetsim::domain::behavior::BtActionName;
namespace BtConditionName = fleetsim::domain::behavior::BtConditionName;

namespace {

class MockBtSimContext final : public IBtSimContext {
public:
    bool planPathForAgent() override
    {
        ++plan_calls;
        path_valid = plan_ok;
        return plan_ok;
    }

    bool hasValidPath() const override { return path_valid; }
    bool isGoalReached() const override { return goal_reached; }

    bool isGoalUpdated() override
    {
        if (!goal_initialized) {
            goal_initialized = true;
            return false;
        }
        return goal_updated;
    }

    void acknowledgeGoal() override { goal_updated = false; }
    bool hasReservationConflict() const override { return reservation_conflict; }
    void requestReplan() override { replan_requested = true; }
    bool needsReplan() const override { return replan_requested; }
    void clearReplanRequest() override { replan_requested = false; }
    double simDt() const override { return 0.05; }

    bool plan_ok{true};
    bool path_valid{false};
    bool goal_reached{false};
    bool goal_updated{false};
    bool goal_initialized{false};
    bool reservation_conflict{false};
    bool replan_requested{false};
    int plan_calls{0};
};

}  // namespace

TEST(BtFleetNodesTest, PlanPathWritesPathValidOnSuccess)
{
    MockBtSimContext context;
    context.plan_ok = true;
    context.path_valid = true;

    BtBlackboard bb;
    bb.setSimContext(&context);

    BtPlanPathNode node(BtActionName::kPlanPath);
    EXPECT_EQ(node.tick(bb), NodeStatus::Success);
    EXPECT_EQ(context.plan_calls, 1);
    ASSERT_TRUE(bb.getBool(BbKey::kPathValid).has_value());
    EXPECT_TRUE(bb.getBool(BbKey::kPathValid).value());
}

TEST(BtFleetNodesTest, PlanPathFailureClearsPathValid)
{
    MockBtSimContext context;
    context.plan_ok = false;
    context.path_valid = false;

    BtBlackboard bb;
    bb.setSimContext(&context);

    BtPlanPathNode node(BtActionName::kPlanPath);
    EXPECT_EQ(node.tick(bb), NodeStatus::Failure);
    ASSERT_TRUE(bb.getBool(BbKey::kPathValid).has_value());
    EXPECT_FALSE(bb.getBool(BbKey::kPathValid).value());
}

TEST(BtFleetNodesTest, FollowUntilGoalRunningWhenPathValidAndNotReached)
{
    MockBtSimContext context;
    context.path_valid = true;
    context.goal_reached = false;

    BtBlackboard bb;
    bb.setSimContext(&context);

    BtFollowUntilGoalNode node(BtActionName::kFollowUntilGoal);
    EXPECT_EQ(node.tick(bb), NodeStatus::Running);
}

TEST(BtFleetNodesTest, FollowUntilGoalSuccessWhenGoalReached)
{
    MockBtSimContext context;
    context.path_valid = true;
    context.goal_reached = true;

    BtBlackboard bb;
    bb.setSimContext(&context);

    BtFollowUntilGoalNode node(BtActionName::kFollowUntilGoal);
    EXPECT_EQ(node.tick(bb), NodeStatus::Success);
}

TEST(BtFleetNodesTest, WaitRecoveryRunsThenSucceeds)
{
    BtBlackboard bb;
    bb.setInt(BbKey::kRecoveryWaitTicks, 2);

    BtWaitRecoveryNode node(BtActionName::kWaitRecovery);
    EXPECT_EQ(node.tick(bb), NodeStatus::Running);
    EXPECT_EQ(node.tick(bb), NodeStatus::Running);
    EXPECT_EQ(node.tick(bb), NodeStatus::Success);
}

TEST(BtFleetNodesTest, IsGoalUpdatedReturnsSuccessWhenGoalChanged)
{
    MockBtSimContext context;
    context.goal_initialized = true;
    context.goal_updated = true;

    BtBlackboard bb;
    bb.setSimContext(&context);

    BtIsGoalUpdatedNode node(BtConditionName::kIsGoalUpdated);
    EXPECT_EQ(node.tick(bb), NodeStatus::Success);
    ASSERT_TRUE(bb.getBool(BbKey::kGoalUpdated).has_value());
    EXPECT_TRUE(bb.getBool(BbKey::kGoalUpdated).value());
}
