#include "BtTreeLoader.h"

#include "BtControlNodes.h"
#include "BtDecoratorNodes.h"
#include "BtFleetActionNodes.h"
#include "BtFleetConditionNodes.h"
#include "BtTypes.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>
#include <utility>

namespace fleetsim::domain::behavior {

namespace {

void setError(BtTreeLoadError* error, std::string message)
{
    if (error != nullptr) {
        error->message = std::move(message);
    }
}

std::string readNodeName(const nlohmann::json& node, const std::string& fallback)
{
    if (node.contains("name") && node.at("name").is_string()) {
        return node.at("name").get<std::string>();
    }
    return fallback;
}

BtNodePtr createActionNode(const std::string& name, BtTreeLoadError* error)
{
    if (name == BtActionName::kPlanPath) {
        return std::make_unique<BtPlanPathNode>(name);
    }
    if (name == BtActionName::kFollowUntilGoal) {
        return std::make_unique<BtFollowUntilGoalNode>(name);
    }
    if (name == BtActionName::kWaitRecovery) {
        return std::make_unique<BtWaitRecoveryNode>(name);
    }
    if (name == BtActionName::kReplanIfTimer) {
        return std::make_unique<BtReplanIfTimerNode>(name);
    }
    setError(error, "Unknown action node: " + name);
    return nullptr;
}

BtNodePtr createConditionNode(const std::string& name, BtTreeLoadError* error)
{
    if (name == BtConditionName::kIsGoalUpdated) {
        return std::make_unique<BtIsGoalUpdatedNode>(name);
    }
    if (name == BtConditionName::kIsPathValid) {
        return std::make_unique<BtIsPathValidNode>(name);
    }
    if (name == BtConditionName::kYieldIfBlocked) {
        return std::make_unique<BtYieldIfBlockedNode>(name);
    }
    setError(error, "Unknown condition node: " + name);
    return nullptr;
}

std::vector<BtNodePtr> parseChildren(const nlohmann::json& node, BtTreeLoadError* error);

BtNodePtr parseNode(const nlohmann::json& node, BtTreeLoadError* error)
{
    if (!node.is_object() || !node.contains("type")) {
        setError(error, "BT node must be an object with 'type'");
        return nullptr;
    }

    const std::string type = node.at("type").get<std::string>();
    const std::string name = readNodeName(node, type);

    if (type == "Sequence" || type == "Fallback") {
        auto children = parseChildren(node, error);
        if (error != nullptr && !error->message.empty()) {
            return nullptr;
        }
        if (type == "Sequence") {
            return std::make_unique<BtSequenceNode>(name, std::move(children));
        }
        return std::make_unique<BtFallbackNode>(name, std::move(children));
    }

    if (type == "Recovery") {
        if (!node.contains("children") || !node.at("children").is_array()) {
            setError(error, "Recovery node requires 'children' array");
            return nullptr;
        }
        const auto& children_json = node.at("children");
        if (children_json.size() != 2) {
            setError(error, "Recovery node requires exactly two children");
            return nullptr;
        }
        BtNodePtr primary = parseNode(children_json.at(0), error);
        if (primary == nullptr) {
            return nullptr;
        }
        BtNodePtr recovery = parseNode(children_json.at(1), error);
        if (recovery == nullptr) {
            return nullptr;
        }
        const int retries = node.value("retries", 3);
        return std::make_unique<BtRecoveryNode>(
            name, std::move(primary), std::move(recovery), retries);
    }

    if (type == "Rate") {
        if (!node.contains("child")) {
            setError(error, "Rate node requires 'child'");
            return nullptr;
        }
        BtNodePtr child = parseNode(node.at("child"), error);
        if (child == nullptr) {
            return nullptr;
        }
        const double hz = node.value("hz", 1.0);
        return std::make_unique<BtRateDecoratorNode>(name, hz, std::move(child));
    }

    if (type == "Action") {
        if (!node.contains("name")) {
            setError(error, "Action node requires 'name'");
            return nullptr;
        }
        return createActionNode(node.at("name").get<std::string>(), error);
    }

    if (type == "Condition") {
        if (!node.contains("name")) {
            setError(error, "Condition node requires 'name'");
            return nullptr;
        }
        return createConditionNode(node.at("name").get<std::string>(), error);
    }

    setError(error, "Unknown BT node type: " + type);
    return nullptr;
}

std::vector<BtNodePtr> parseChildren(const nlohmann::json& node, BtTreeLoadError* error)
{
    std::vector<BtNodePtr> children;
    if (!node.contains("children")) {
        return children;
    }
    if (!node.at("children").is_array()) {
        setError(error, "'children' must be an array");
        return children;
    }
    for (const auto& child_json : node.at("children")) {
        BtNodePtr child = parseNode(child_json, error);
        if (child == nullptr) {
            children.clear();
            return children;
        }
        children.push_back(std::move(child));
    }
    return children;
}

BtNodePtr parseRootDocument(const nlohmann::json& document, BtTreeLoadError* error)
{
    if (!document.is_object()) {
        setError(error, "BT JSON root must be an object");
        return nullptr;
    }
    const int version = document.value("version", 0);
    if (version != 1) {
        setError(error, "Unsupported BT JSON version (expected 1)");
        return nullptr;
    }
    if (!document.contains("root")) {
        setError(error, "BT JSON missing 'root'");
        return nullptr;
    }
    return parseNode(document.at("root"), error);
}

}  // namespace

std::optional<BtNodePtr> BtTreeLoader::loadFromFile(const std::string& path,
                                                      BtTreeLoadError* error)
{
    std::ifstream input(path);
    if (!input.is_open()) {
        setError(error, "Failed to open BT JSON file: " + path);
        return std::nullopt;
    }
    std::stringstream buffer;
    buffer << input.rdbuf();
    return loadFromJsonString(buffer.str(), error);
}

std::optional<BtNodePtr> BtTreeLoader::loadFromJsonString(const std::string& json,
                                                          BtTreeLoadError* error)
{
    if (error != nullptr) {
        error->message.clear();
    }
    nlohmann::json document;
    try {
        document = nlohmann::json::parse(json);
    } catch (const nlohmann::json::exception& ex) {
        setError(error, std::string("BT JSON parse error: ") + ex.what());
        return std::nullopt;
    }

    BtNodePtr root = parseRootDocument(document, error);
    if (root == nullptr) {
        return std::nullopt;
    }
    return root;
}

}  // namespace fleetsim::domain::behavior
