#include "domain/behavior/BtBlackboard.h"
#include "domain/behavior/BtMotionRecoveryNodes.h"
#include "domain/behavior/BtTypes.h"

#include <gtest/gtest.h>

using fleetsim::domain::behavior::BtBackUpRecoveryNode;
using fleetsim::domain::behavior::BtBlackboard;
using fleetsim::domain::behavior::BtClearInflationNode;
using fleetsim::domain::behavior::BtSpinRecoveryNode;
using fleetsim::domain::behavior::NodeStatus;

namespace BbKey = fleetsim::domain::behavior::BbKey;

TEST(BtMotionRecoveryTest, SpinChangesYaw)
{
    BtBlackboard bb;
    bb.setString(BbKey::kAgentId, "agv_0");

    BtSpinRecoveryNode spin("Spin", 0.5);
    EXPECT_EQ(spin.tick(bb), NodeStatus::Success);
}

TEST(BtMotionRecoveryTest, BackUpMovesAlongRearHeading)
{
    BtBlackboard bb;
    bb.setString(BbKey::kAgentId, "agv_0");

    BtBackUpRecoveryNode backup("BackUp", 0.3, 0.1);
    EXPECT_EQ(backup.tick(bb), NodeStatus::Success);
}

TEST(BtMotionRecoveryTest, ClearInflationHasSideEffect)
{
    BtBlackboard bb;
    bb.setString(BbKey::kAgentId, "agv_0");

    BtClearInflationNode clear_node("ClearInflation");
    EXPECT_EQ(clear_node.tick(bb), NodeStatus::Success);
}
