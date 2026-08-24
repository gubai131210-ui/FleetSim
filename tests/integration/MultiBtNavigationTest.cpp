#include "domain/behavior/MultiBtNavigator.h"

#include <gtest/gtest.h>

using fleetsim::domain::behavior::MultiBtNavigator;

namespace BbKey = fleetsim::domain::behavior::BbKey;

TEST(MultiBtNavigationTest, SeparateBlackboardsDoNotOverwriteAgentId)
{
    MultiBtNavigator multi;

    auto& bb0 = multi.blackboardFor("agv_0");
    auto& bb1 = multi.blackboardFor("agv_1");

    EXPECT_EQ(bb0.getString(BbKey::kAgentId).value_or(""), "agv_0");
    EXPECT_EQ(bb1.getString(BbKey::kAgentId).value_or(""), "agv_1");
}

TEST(MultiBtNavigationTest, DistinctNavigatorInstances)
{
    MultiBtNavigator multi;
    EXPECT_NE(&multi.navigatorFor("a"), &multi.navigatorFor("b"));
    EXPECT_NE(&multi.blackboardFor("a"), &multi.blackboardFor("b"));
    EXPECT_EQ(multi.agentCount(), 2U);
}
