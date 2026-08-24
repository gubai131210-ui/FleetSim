#include "domain/behavior/BtBlackboard.h"
#include "domain/behavior/BtControlNodes.h"
#include "domain/behavior/BtNavigator.h"
#include "domain/behavior/BtTypes.h"
#include "domain/behavior/BtXmlLoader.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

using fleetsim::domain::behavior::BtBlackboard;
using fleetsim::domain::behavior::BtNode;
using fleetsim::domain::behavior::BtNodePtr;
using fleetsim::domain::behavior::BtNodeType;
using fleetsim::domain::behavior::BtRoundRobinNode;
using fleetsim::domain::behavior::BtXmlLoadError;
using fleetsim::domain::behavior::BtXmlLoader;
using fleetsim::domain::behavior::NodeStatus;

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

std::string xmlAssetPath()
{
    const std::filesystem::path source_root =
        std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    return (source_root / "assets" / "behavior_trees" / "navigate_spin_backup_recovery.xml")
        .string();
}

}  // namespace

TEST(BtXmlLoaderTest, UnknownTagFailsLoad)
{
    const std::string xml = R"(<?xml version="1.0"?>
<root main_tree_to_execute="MainTree">
  <BehaviorTree ID="MainTree">
    <NotARealNode name="x"/>
  </BehaviorTree>
</root>)";
    BtXmlLoadError error;
    const auto root = BtXmlLoader::loadFromXmlString(xml, &error);
    EXPECT_FALSE(root.has_value());
    EXPECT_FALSE(error.message.empty());
}

TEST(BtXmlLoaderTest, LoadsNavigateSpinBackupAsset)
{
    BtXmlLoadError error;
    const auto root = BtXmlLoader::loadFromFile(xmlAssetPath(), &error);
    ASSERT_TRUE(root.has_value()) << error.message;
    EXPECT_FALSE(root.value()->name().empty());
}

TEST(BtXmlLoaderTest, RoundRobinRotatesChildIndex)
{
    fleetsim::domain::behavior::BtBlackboard bb;
    std::vector<BtNodePtr> children;
    children.push_back(std::make_unique<MockLeafNode>("a", NodeStatus::Failure));
    children.push_back(std::make_unique<MockLeafNode>("b", NodeStatus::Success));

    BtRoundRobinNode rr("RR", std::move(children));
    EXPECT_EQ(rr.currentChildIndex(), 0U);
    EXPECT_EQ(rr.tick(bb), NodeStatus::Success);
    EXPECT_EQ(rr.currentChildIndex(), 1U);
}
