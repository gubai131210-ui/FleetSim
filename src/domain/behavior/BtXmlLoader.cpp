#include "domain/behavior/BtXmlLoader.h"

#include "BtControlNodes.h"
#include "BtDecoratorNodes.h"
#include "BtFleetActionNodes.h"
#include "BtFleetConditionNodes.h"
#include "BtMotionRecoveryNodes.h"
#include "BtTypes.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <utility>

namespace fleetsim::domain::behavior {

namespace {

void setError(BtXmlLoadError* error, std::string message)
{
    if (error != nullptr) {
        error->message = std::move(message);
    }
}

std::optional<std::string> xmlAttribute(const std::string& snippet, const std::string& name)
{
    const std::string pattern = name + "=\"";
    const std::size_t start = snippet.find(pattern);
    if (start == std::string::npos) {
        return std::nullopt;
    }
    const std::size_t value_start = start + pattern.size();
    const std::size_t value_end = snippet.find('"', value_start);
    if (value_end == std::string::npos) {
        return std::nullopt;
    }
    return snippet.substr(value_start, value_end - value_start);
}

void parseAllAttributes(const std::string& header, std::unordered_map<std::string, std::string>& attrs)
{
    std::size_t pos = 0;
    while (pos < header.size()) {
        while (pos < header.size()
               && (std::isspace(static_cast<unsigned char>(header[pos])) || header[pos] == '/')) {
            ++pos;
        }
        if (pos >= header.size()) {
            break;
        }

        const std::size_t eq = header.find('=', pos);
        if (eq == std::string::npos) {
            break;
        }

        std::string key = header.substr(pos, eq - pos);
        while (!key.empty() && std::isspace(static_cast<unsigned char>(key.back()))) {
            key.pop_back();
        }

        if (eq + 1 >= header.size() || header[eq + 1] != '"') {
            break;
        }

        const std::size_t value_start = eq + 2;
        const std::size_t value_end = header.find('"', value_start);
        if (value_end == std::string::npos) {
            break;
        }

        attrs[key] = header.substr(value_start, value_end - value_start);
        pos = value_end + 1;
    }
}

std::string trimCopy(std::string value)
{
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
        value.erase(value.begin());
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
        value.pop_back();
    }
    return value;
}

void skipWhitespace(const std::string& text, std::size_t& pos)
{
    while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos])) != 0) {
        ++pos;
    }
}

struct XmlElement {
    std::string tag;
    std::string raw_header;
    std::unordered_map<std::string, std::string> attributes;
    std::vector<XmlElement> children;
};

bool parseXmlElement(const std::string& text, std::size_t& pos, XmlElement& element, std::string& error)
{
    element = XmlElement{};
    skipWhitespace(text, pos);
    if (pos >= text.size() || text[pos] != '<') {
        error = "Expected XML element";
        return false;
    }

    const std::size_t tag_start = pos + 1;
    const std::size_t tag_end = text.find('>', tag_start);
    if (tag_end == std::string::npos) {
        error = "Malformed XML tag";
        return false;
    }

    const std::string tag_header = text.substr(tag_start, tag_end - tag_start);
    element.raw_header = tag_header;
    if (!tag_header.empty() && tag_header[0] == '/') {
        error = "Unexpected closing tag";
        return false;
    }

    if (!tag_header.empty() && tag_header[0] == '?') {
        pos = tag_end + 1;
        error.clear();
        return false;
    }

    if (!tag_header.empty() && tag_header[0] == '!') {
        pos = tag_end + 1;
        error.clear();
        return false;
    }

    const bool self_closing =
        tag_header.size() >= 1 && tag_header[tag_header.size() - 1] == '/';
    const std::string header_body =
        self_closing ? tag_header.substr(0, tag_header.size() - 1) : tag_header;

    const std::size_t space_pos = header_body.find(' ');
    element.tag = trimCopy((space_pos == std::string::npos) ? header_body
                                                            : header_body.substr(0, space_pos));

    element.attributes.clear();
    if (space_pos != std::string::npos) {
        parseAllAttributes(header_body.substr(space_pos), element.attributes);
    }

    pos = tag_end + 1;
    if (self_closing) {
        return true;
    }

    const std::string close_tag = "</" + element.tag + ">";
    const std::size_t close_pos = text.find(close_tag, pos);
    if (close_pos == std::string::npos) {
        error = "Missing closing tag for " + element.tag;
        return false;
    }

    const std::string inner = text.substr(pos, close_pos - pos);
    std::size_t child_pos = 0;
    int parse_guard = 0;
    while (child_pos < inner.size()) {
        if (++parse_guard > 512) {
            error = "XML child parse iteration limit exceeded for " + element.tag;
            return false;
        }
        skipWhitespace(inner, child_pos);
        if (child_pos >= inner.size() || inner[child_pos] != '<') {
            break;
        }
        if (inner[child_pos + 1] == '/' || inner[child_pos + 1] == '!'
            || inner[child_pos + 1] == '?') {
            const std::size_t skip_end = inner.find('>', child_pos);
            if (skip_end == std::string::npos) {
                break;
            }
            child_pos = skip_end + 1;
            continue;
        }

        XmlElement child;
        if (!parseXmlElement(inner, child_pos, child, error)) {
            if (!error.empty()) {
                return false;
            }
            continue;
        }
        element.children.push_back(std::move(child));
    }

    pos = close_pos + close_tag.size();
    return true;
}

std::vector<XmlElement> extractBehaviorTrees(const std::string& xml)
{
    std::vector<XmlElement> trees;
    const std::string open = "<BehaviorTree";
    std::size_t search_from = 0;

    while (true) {
        const std::size_t open_pos = xml.find(open, search_from);
        if (open_pos == std::string::npos) {
            break;
        }
        std::size_t pos = open_pos;
        XmlElement tree;
        std::string error;
        if (parseXmlElement(xml, pos, tree, error)) {
            trees.push_back(std::move(tree));
        }
        search_from = pos > open_pos ? pos : open_pos + 1;
    }
    return trees;
}

std::string nodeDisplayName(const XmlElement& element, const std::string& fallback)
{
    const auto name_it = element.attributes.find("name");
    if (name_it != element.attributes.end()) {
        return name_it->second;
    }
    const auto id_it = element.attributes.find("ID");
    if (id_it != element.attributes.end()) {
        return id_it->second;
    }
    return fallback;
}

double parseDoubleAttr(const XmlElement& element,
                       const std::string& primary,
                       const std::string& alternate,
                       double default_value)
{
    const auto primary_it = element.attributes.find(primary);
    if (primary_it != element.attributes.end()) {
        return std::stod(primary_it->second);
    }
    const auto alternate_it = element.attributes.find(alternate);
    if (alternate_it != element.attributes.end()) {
        return std::stod(alternate_it->second);
    }
    return default_value;
}

BtNodePtr createActionFromXml(const XmlElement& element, BtXmlLoadError* error);
BtNodePtr createConditionFromXml(const XmlElement& element, BtXmlLoadError* error);
std::vector<BtNodePtr> parseXmlChildren(const std::vector<XmlElement>& children,
                                        BtXmlLoadError* error);

BtNodePtr createActionFromXml(const XmlElement& element, BtXmlLoadError* error)
{
    std::string action_id;
    if (const auto id_it = element.attributes.find("ID"); id_it != element.attributes.end()) {
        action_id = id_it->second;
    } else if (const auto id_it = element.attributes.find("id"); id_it != element.attributes.end()) {
        action_id = id_it->second;
    } else if (const auto name_it = element.attributes.find("name");
               name_it != element.attributes.end()) {
        action_id = name_it->second;
    }

    if (action_id.empty()) {
        if (const auto id = xmlAttribute(element.raw_header, "ID")) {
            action_id = *id;
        } else if (const auto id = xmlAttribute(element.raw_header, "id")) {
            action_id = *id;
        }
    }

    if (action_id.empty()) {
        setError(error, "Action element missing ID attribute");
        return nullptr;
    }
    const std::string name = nodeDisplayName(element, action_id);

    if (action_id == BtActionName::kPlanPath) {
        return std::make_unique<BtPlanPathNode>(name);
    }
    if (action_id == BtActionName::kFollowUntilGoal) {
        return std::make_unique<BtFollowUntilGoalNode>(name);
    }
    if (action_id == BtActionName::kWaitRecovery) {
        return std::make_unique<BtWaitRecoveryNode>(name);
    }
    if (action_id == BtActionName::kReplanIfTimer) {
        return std::make_unique<BtReplanIfTimerNode>(name);
    }
    if (action_id == BtActionName::kSpin) {
        const double spin_rad = parseDoubleAttr(element, "spin_rad", "spin_rad", 1.5707963267948966);
        return std::make_unique<BtSpinRecoveryNode>(name, spin_rad);
    }
    if (action_id == BtActionName::kBackUp) {
        const double backup_dist = parseDoubleAttr(element, "backup_dist", "backup_dist_m", 0.3);
        const double backup_speed =
            parseDoubleAttr(element, "backup_speed", "backup_speed_mps", 0.1);
        return std::make_unique<BtBackUpRecoveryNode>(name, backup_dist, backup_speed);
    }
    if (action_id == BtActionName::kClearInflation) {
        return std::make_unique<BtClearInflationNode>(name);
    }

    setError(error, "Unknown action ID: " + action_id);
    return nullptr;
}

BtNodePtr createConditionFromXml(const XmlElement& element, BtXmlLoadError* error)
{
    const auto id_it = element.attributes.find("ID");
    if (id_it == element.attributes.end()) {
        setError(error, "Condition element missing ID attribute");
        return nullptr;
    }

    const std::string& condition_id = id_it->second;
    const std::string name = nodeDisplayName(element, condition_id);

    if (condition_id == BtConditionName::kIsGoalUpdated) {
        return std::make_unique<BtIsGoalUpdatedNode>(name);
    }
    if (condition_id == BtConditionName::kIsPathValid) {
        return std::make_unique<BtIsPathValidNode>(name);
    }
    if (condition_id == BtConditionName::kYieldIfBlocked) {
        return std::make_unique<BtYieldIfBlockedNode>(name);
    }

    setError(error, "Unknown condition ID: " + condition_id);
    return nullptr;
}

BtNodePtr parseXmlNode(const XmlElement& element, BtXmlLoadError* error)
{
    const std::string& tag = element.tag;
    const std::string name = nodeDisplayName(element, tag);

    if (tag == "Sequence") {
        auto children = parseXmlChildren(element.children, error);
        if (error != nullptr && !error->message.empty()) {
            return nullptr;
        }
        return std::make_unique<BtSequenceNode>(name, std::move(children));
    }

    if (tag == "Fallback") {
        auto children = parseXmlChildren(element.children, error);
        if (error != nullptr && !error->message.empty()) {
            return nullptr;
        }
        return std::make_unique<BtFallbackNode>(name, std::move(children));
    }

    if (tag == "ReactiveFallback") {
        auto children = parseXmlChildren(element.children, error);
        if (error != nullptr && !error->message.empty()) {
            return nullptr;
        }
        return std::make_unique<BtReactiveFallbackNode>(name, std::move(children));
    }

    if (tag == "RoundRobin") {
        auto children = parseXmlChildren(element.children, error);
        if (error != nullptr && !error->message.empty()) {
            return nullptr;
        }
        return std::make_unique<BtRoundRobinNode>(name, std::move(children));
    }

    if (tag == "RecoveryNode") {
        if (element.children.size() != 2) {
            setError(error, "RecoveryNode requires exactly two children");
            return nullptr;
        }
        BtNodePtr primary = parseXmlNode(element.children[0], error);
        if (primary == nullptr) {
            return nullptr;
        }
        BtNodePtr recovery = parseXmlNode(element.children[1], error);
        if (recovery == nullptr) {
            return nullptr;
        }
        const int retries = static_cast<int>(
            parseDoubleAttr(element, "number_of_retries", "retries", 3.0));
        return std::make_unique<BtRecoveryNode>(
            name, std::move(primary), std::move(recovery), retries);
    }

    if (tag == "RateController" || tag == "Rate") {
        if (element.children.size() != 1) {
            setError(error, tag + " requires exactly one child");
            return nullptr;
        }
        BtNodePtr child = parseXmlNode(element.children[0], error);
        if (child == nullptr) {
            return nullptr;
        }
        const double hz = parseDoubleAttr(element, "hz", "frequency", 1.0);
        return std::make_unique<BtRateDecoratorNode>(name, hz, std::move(child));
    }

    if (tag == "Action") {
        return createActionFromXml(element, error);
    }

    if (tag == "Condition") {
        return createConditionFromXml(element, error);
    }

    if (tag == BtActionName::kPlanPath || tag == BtActionName::kFollowUntilGoal
        || tag == BtActionName::kWaitRecovery || tag == BtActionName::kReplanIfTimer
        || tag == BtActionName::kSpin || tag == BtActionName::kBackUp
        || tag == BtActionName::kClearInflation) {
        XmlElement action_element = element;
        action_element.attributes["ID"] = tag;
        return createActionFromXml(action_element, error);
    }

    setError(error, "Unknown BT XML tag: " + tag);
    return nullptr;
}

std::vector<BtNodePtr> parseXmlChildren(const std::vector<XmlElement>& children,
                                        BtXmlLoadError* error)
{
    std::vector<BtNodePtr> nodes;
    nodes.reserve(children.size());
    for (const XmlElement& child : children) {
        BtNodePtr node = parseXmlNode(child, error);
        if (node == nullptr) {
            nodes.clear();
            return nodes;
        }
        nodes.push_back(std::move(node));
    }
    return nodes;
}

std::optional<XmlElement> findBehaviorTreeRoot(const std::string& xml,
                                               const std::string& main_tree_id,
                                               std::string& error)
{
    const std::vector<XmlElement> trees = extractBehaviorTrees(xml);
    if (trees.empty()) {
        error = "No BehaviorTree element found";
        return std::nullopt;
    }

    if (!main_tree_id.empty()) {
        for (const XmlElement& tree : trees) {
            const auto id_it = tree.attributes.find("ID");
            if (id_it != tree.attributes.end() && id_it->second == main_tree_id) {
                if (tree.children.empty()) {
                    error = "BehaviorTree " + main_tree_id + " has no root node";
                    return std::nullopt;
                }
                return tree;
            }
        }
        error = "BehaviorTree ID not found: " + main_tree_id;
        return std::nullopt;
    }

    if (trees.front().children.empty()) {
        error = "BehaviorTree has no root node";
        return std::nullopt;
    }
    return trees.front();
}

std::optional<BtNodePtr> loadXmlContent(const std::string& xml, BtXmlLoadError* error)
{
    if (error != nullptr) {
        error->message.clear();
    }

    if (xml.find("<root") == std::string::npos && xml.find("<BehaviorTree") == std::string::npos) {
        setError(error, "Missing BT XML root element");
        return std::nullopt;
    }

    std::string main_tree_id;
    const std::size_t root_pos = xml.find("<root");
    if (root_pos != std::string::npos) {
        const std::size_t root_end = xml.find('>', root_pos);
        if (root_end != std::string::npos) {
            const std::string root_header = xml.substr(root_pos, root_end - root_pos);
            main_tree_id = xmlAttribute(root_header, "main_tree_to_execute").value_or("");
        }
    }

    std::string lookup_error;
    const std::optional<XmlElement> tree = findBehaviorTreeRoot(xml, main_tree_id, lookup_error);
    if (!tree.has_value()) {
        setError(error, lookup_error);
        return std::nullopt;
    }

    BtNodePtr root = parseXmlNode(tree->children.front(), error);
    if (root == nullptr) {
        return std::nullopt;
    }
    return root;
}

}  // namespace

std::optional<BtNodePtr> BtXmlLoader::loadFromFile(const std::string& path,
                                                   BtXmlLoadError* error)
{
    std::ifstream input(path);
    if (!input.is_open()) {
        setError(error, "Failed to open BT XML file: " + path);
        return std::nullopt;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return loadXmlContent(buffer.str(), error);
}

std::optional<BtNodePtr> BtXmlLoader::loadFromXmlString(const std::string& xml,
                                                        BtXmlLoadError* error)
{
    return loadXmlContent(xml, error);
}

}  // namespace fleetsim::domain::behavior
