#include "domain/behavior/BtBlackboard.h"
#include "domain/behavior/BtControlNodes.h"
#include "domain/behavior/BtDecoratorNodes.h"
#include "domain/behavior/BtNavigator.h"
#include "domain/behavior/BtTreeLoader.h"
#include "domain/behavior/BtTypes.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

using fleetsim::domain::behavior::BtBlackboard;
using fleetsim::domain::behavior::BtFallbackNode;
using fleetsim::domain::behavior::BtNavigator;
using fleetsim::domain::behavior::BtNode;
using fleetsim::domain::behavior::BtNodePtr;
using fleetsim::domain::behavior::BtNodeType;
using fleetsim::domain::behavior::BtRateDecoratorNode;
using fleetsim::domain::behavior::BtRecoveryNode;
using fleetsim::domain::behavior::BtSequenceNode;
using fleetsim::domain::behavior::BtTreeLoadError;
using fleetsim::domain::behavior::BtTreeLoader;
using fleetsim::domain::behavior::NodeStatus;

namespace BbKey = fleetsim::domain::behavior::BbKey;

namespace {

class MockLeafNode final : public BtNode {
public:
    MockLeafNode(std::string name, NodeStatus status)
        : name_(std::move(name))
        , status_(status)
    {
    }

    NodeStatus tick(BtBlackboard& /*blackboard*/) override { return status_; }
    std::string name() const override { return name_; }
    BtNodeType type() const override { return BtNodeType::Action; }

private:
    std::string name_;
    NodeStatus status_;
};

BtNodePtr makeLeaf(const std::string& name, NodeStatus status)
{
    return std::make_unique<MockLeafNode>(name, status);
}

}  // namespace

TEST(BehaviorTreeTest, BlackboardStoresTypedValues)
{
    BtBlackboard bb;
    bb.setBool(BbKey::kPathValid, true);
    bb.setInt(BbKey::kRecoveryCount, 2);
    bb.setDouble(BbKey::kReplanHz, 1.5);
    bb.setString(BbKey::kAgentId, "agv_0");

    ASSERT_TRUE(bb.getBool(BbKey::kPathValid).has_value());
    EXPECT_TRUE(bb.getBool(BbKey::kPathValid).value());
    ASSERT_TRUE(bb.getInt(BbKey::kRecoveryCount).has_value());
    EXPECT_EQ(bb.getInt(BbKey::kRecoveryCount).value(), 2);
    ASSERT_TRUE(bb.getDouble(BbKey::kReplanHz).has_value());
    EXPECT_NEAR(bb.getDouble(BbKey::kReplanHz).value(), 1.5, 1e-9);
    ASSERT_TRUE(bb.getString(BbKey::kAgentId).has_value());
    EXPECT_EQ(bb.getString(BbKey::kAgentId).value(), "agv_0");
}

TEST(BehaviorTreeTest, SequenceAllSuccessReturnsSuccess)
{
    BtBlackboard bb;
    std::vector<BtNodePtr> children;
    children.push_back(makeLeaf("a", NodeStatus::Success));
    children.push_back(makeLeaf("b", NodeStatus::Success));

    BtSequenceNode seq("Seq", std::move(children));
    EXPECT_EQ(seq.tick(bb), NodeStatus::Success);
}

TEST(BehaviorTreeTest, SequenceStopsOnFirstFailure)
{
    BtBlackboard bb;
    std::vector<BtNodePtr> children;
    children.push_back(makeLeaf("ok", NodeStatus::Success));
    children.push_back(makeLeaf("fail", NodeStatus::Failure));
    children.push_back(makeLeaf("never", NodeStatus::Success));

    BtSequenceNode seq("SeqFail", std::move(children));
    EXPECT_EQ(seq.tick(bb), NodeStatus::Failure);
    EXPECT_EQ(seq.activeChildName(), "fail");
}

TEST(BehaviorTreeTest, SequenceReturnsRunningWhenChildRunning)
{
    BtBlackboard bb;
    std::vector<BtNodePtr> children;
    children.push_back(makeLeaf("run", NodeStatus::Running));

    BtSequenceNode seq("SeqRun", std::move(children));
    EXPECT_EQ(seq.tick(bb), NodeStatus::Running);
}

TEST(BehaviorTreeTest, FallbackReturnsFirstSuccess)
{
    BtBlackboard bb;
    std::vector<BtNodePtr> children;
    children.push_back(makeLeaf("fail", NodeStatus::Failure));
    children.push_back(makeLeaf("ok", NodeStatus::Success));
    children.push_back(makeLeaf("later", NodeStatus::Success));

    BtFallbackNode fb("Fb", std::move(children));
    EXPECT_EQ(fb.tick(bb), NodeStatus::Success);
    EXPECT_EQ(fb.activeChildName(), "ok");
}

TEST(BehaviorTreeTest, FallbackAllFailureReturnsFailure)
{
    BtBlackboard bb;
    std::vector<BtNodePtr> children;
    children.push_back(makeLeaf("a", NodeStatus::Failure));
    children.push_back(makeLeaf("b", NodeStatus::Failure));

    BtFallbackNode fb("FbAllFail", std::move(children));
    EXPECT_EQ(fb.tick(bb), NodeStatus::Failure);
}

TEST(BehaviorTreeTest, RecoveryPrimaryFailThenRecoverySuccessRetriesPrimary)
{
    BtBlackboard bb;

    struct StatefulPrimary final : public BtNode {
        explicit StatefulPrimary(int* calls) : calls_(calls) {}

        NodeStatus tick(BtBlackboard& /*bb*/) override
        {
            ++(*calls_);
            return (*calls_ >= 2) ? NodeStatus::Success : NodeStatus::Failure;
        }
        std::string name() const override { return "Primary"; }
        BtNodeType type() const override { return BtNodeType::Sequence; }

        int* calls_;
    };

    int primary_calls = 0;
    BtNodePtr primary = std::make_unique<StatefulPrimary>(&primary_calls);
    BtNodePtr recovery = makeLeaf("RecoveryWait", NodeStatus::Success);

    BtRecoveryNode recovery_node("NavRecovery", std::move(primary), std::move(recovery), 3);
    EXPECT_EQ(recovery_node.tick(bb), NodeStatus::Success);
    EXPECT_GE(primary_calls, 2);
}

TEST(BehaviorTreeTest, RecoveryExhaustsRetriesReturnsFailure)
{
    BtBlackboard bb;
    BtNodePtr primary = makeLeaf("AlwaysFail", NodeStatus::Failure);
    BtNodePtr recovery = makeLeaf("RecoveryOk", NodeStatus::Success);

    BtRecoveryNode recovery_node("NavRecoveryFail", std::move(primary), std::move(recovery), 1);
    EXPECT_EQ(recovery_node.tick(bb), NodeStatus::Failure);
}

TEST(BehaviorTreeTest, RateDecoratorSkipsChildWithinInterval)
{
    BtBlackboard bb;
    struct CountingLeaf final : public BtNode {
        explicit CountingLeaf(int* ticks) : ticks_(ticks) {}
        NodeStatus tick(BtBlackboard& /*bb*/) override
        {
            ++(*ticks_);
            return NodeStatus::Success;
        }
        std::string name() const override { return "Counted"; }
        BtNodeType type() const override { return BtNodeType::Action; }
        int* ticks_;
    };

    int tick_count = 0;
    BtRateDecoratorNode rate("Rate1Hz", 1.0, std::make_unique<CountingLeaf>(&tick_count));

    rate.tick(bb);
    rate.tick(bb);
    EXPECT_EQ(tick_count, 1) << "Rate decorator should tick child at most once per interval";
}

TEST(BehaviorTreeTest, TreeLoaderNotYetImplementedReturnsNullopt)
{
    BtTreeLoadError error;
    const auto tree = BtTreeLoader::loadFromJsonString("{}", &error);
    EXPECT_FALSE(tree.has_value());
    EXPECT_FALSE(error.message.empty());
}

TEST(BehaviorTreeTest, NavigatorWithoutTreeReturnsFailure)
{
    BtNavigator nav;
    BtBlackboard bb;
    const auto result = nav.tick(bb);
    EXPECT_EQ(result.status, NodeStatus::Failure);
    EXPECT_FALSE(nav.hasTree());
}
