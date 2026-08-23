#include "domain/behavior/BtNavigator.h"
#include "domain/behavior/BtTreeLoader.h"
#include "domain/behavior/BtTypes.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

using fleetsim::domain::behavior::BtNavigator;
using fleetsim::domain::behavior::BtNodeType;
using fleetsim::domain::behavior::BtTreeLoadError;
using fleetsim::domain::behavior::BtTreeLoader;

namespace {

std::string behaviorTreeAssetPath()
{
    const std::filesystem::path source_root =
        std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    return (source_root / "assets" / "behavior_trees" / "navigate_replan_recovery.json").string();
}

}  // namespace

TEST(BtTreeLoaderTest, LoadsNavigateRecoveryAsset)
{
    BtTreeLoadError error;
    const auto root = BtTreeLoader::loadFromFile(behaviorTreeAssetPath(), &error);
    ASSERT_TRUE(root.has_value()) << error.message;
    EXPECT_EQ((*root)->name(), "NavigateRecovery");
    EXPECT_EQ((*root)->type(), BtNodeType::Recovery);
}

TEST(BtTreeLoaderTest, RejectsUnsupportedVersion)
{
    const std::string json = R"({"version": 99, "root": {"type": "Sequence", "name": "X", "children": []}})";
    BtTreeLoadError error;
    const auto root = BtTreeLoader::loadFromJsonString(json, &error);
    EXPECT_FALSE(root.has_value());
    EXPECT_FALSE(error.message.empty());
}

TEST(BtTreeLoaderTest, NavigatorLoadsTreeFromFile)
{
    BtNavigator navigator;
    ASSERT_TRUE(navigator.loadFromJsonFile(behaviorTreeAssetPath()));
    EXPECT_TRUE(navigator.hasTree());
    EXPECT_EQ(navigator.treeName(), "NavigateRecovery");
    ASSERT_NE(navigator.root(), nullptr);
    EXPECT_EQ(navigator.root()->type(), BtNodeType::Recovery);
}

TEST(BtTreeLoaderTest, UnknownActionFailsLoad)
{
    const std::string json = R"({
      "version": 1,
      "root": { "type": "Action", "name": "NotARealAction" }
    })";
    BtTreeLoadError error;
    const auto root = BtTreeLoader::loadFromJsonString(json, &error);
    EXPECT_FALSE(root.has_value());
    EXPECT_NE(error.message.find("Unknown action"), std::string::npos);
}
