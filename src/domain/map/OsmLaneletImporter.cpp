#include "domain/map/OsmLaneletImporter.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fleetsim::domain::map {

namespace {

void setError(OsmImportError* error, const std::string& message)
{
    if (error != nullptr) {
        error->message = message;
    }
}

struct Point2D {
    double x{0.0};
    double y{0.0};
};

struct ParsedNode {
    std::string id;
    double x{0.0};
    double y{0.0};
    bool has_local_coords{false};
};

struct ParsedWay {
    std::string id;
    std::vector<std::string> node_refs;
};

struct ParsedLanelet {
    std::string id;
    std::string left_way_id;
    std::string right_way_id;
    std::string centerline_way_id;
};

struct ParsedOsmDocument {
    std::unordered_map<std::string, ParsedNode> nodes;
    std::unordered_map<std::string, ParsedWay> ways;
    std::vector<ParsedLanelet> lanelets;
};

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

std::vector<std::string> extractElements(const std::string& xml, const std::string& tag)
{
    std::vector<std::string> elements;
    const std::string open = "<" + tag;
    const std::string close = "</" + tag + ">";
    std::size_t search_from = 0;

    while (true) {
        const std::size_t open_pos = xml.find(open, search_from);
        if (open_pos == std::string::npos) {
            break;
        }
        const std::size_t close_pos = xml.find(close, open_pos);
        if (close_pos == std::string::npos) {
            break;
        }
        elements.push_back(xml.substr(open_pos, close_pos - open_pos + close.size()));
        search_from = close_pos + close.size();
    }
    return elements;
}

std::vector<std::string> extractSelfClosingTags(const std::string& parent, const std::string& tag)
{
    std::vector<std::string> tags;
    const std::string open = "<" + tag;
    std::size_t search_from = 0;

    while (true) {
        const std::size_t open_pos = parent.find(open, search_from);
        if (open_pos == std::string::npos) {
            break;
        }
        const std::size_t close_pos = parent.find("/>", open_pos);
        if (close_pos == std::string::npos) {
            break;
        }
        tags.push_back(parent.substr(open_pos, close_pos - open_pos + 2));
        search_from = close_pos + 2;
    }
    return tags;
}

bool parseOsmDocument(const std::string& xml, ParsedOsmDocument& document, std::string& error)
{
    if (xml.find("<osm") == std::string::npos) {
        error = "Missing root <osm> element";
        return false;
    }

    for (const std::string& element : extractElements(xml, "node")) {
        const auto id = xmlAttribute(element, "id");
        if (!id.has_value()) {
            continue;
        }

        ParsedNode node;
        node.id = *id;

        for (const std::string& tag_element : extractSelfClosingTags(element, "tag")) {
            const auto key = xmlAttribute(tag_element, "k");
            const auto value = xmlAttribute(tag_element, "v");
            if (!key.has_value() || !value.has_value()) {
                continue;
            }
            if (*key == "local_x" || *key == "x") {
                node.x = std::stod(*value);
                node.has_local_coords = true;
            } else if (*key == "local_y" || *key == "y") {
                node.y = std::stod(*value);
                node.has_local_coords = true;
            }
        }

        if (node.has_local_coords) {
            document.nodes.emplace(node.id, node);
        }
    }

    for (const std::string& element : extractElements(xml, "way")) {
        const auto id = xmlAttribute(element, "id");
        if (!id.has_value()) {
            continue;
        }

        ParsedWay way;
        way.id = *id;
        for (const std::string& nd_element : extractSelfClosingTags(element, "nd")) {
            const auto ref = xmlAttribute(nd_element, "ref");
            if (ref.has_value()) {
                way.node_refs.push_back(*ref);
            }
        }
        if (!way.node_refs.empty()) {
            document.ways.emplace(way.id, std::move(way));
        }
    }

    for (const std::string& element : extractElements(xml, "relation")) {
        bool is_lanelet = false;
        for (const std::string& tag_element : extractSelfClosingTags(element, "tag")) {
            const auto key = xmlAttribute(tag_element, "k");
            const auto value = xmlAttribute(tag_element, "v");
            if (key.has_value() && value.has_value() && *key == "type" && *value == "lanelet") {
                is_lanelet = true;
                break;
            }
        }
        if (!is_lanelet) {
            continue;
        }

        const auto id = xmlAttribute(element, "id");
        if (!id.has_value()) {
            continue;
        }

        ParsedLanelet lanelet;
        lanelet.id = *id;

        for (const std::string& member_element : extractSelfClosingTags(element, "member")) {
            const auto member_type = xmlAttribute(member_element, "type");
            const auto ref = xmlAttribute(member_element, "ref");
            const auto role = xmlAttribute(member_element, "role");
            if (!member_type.has_value() || *member_type != "way" || !ref.has_value()
                || !role.has_value()) {
                continue;
            }
            if (*role == "left") {
                lanelet.left_way_id = *ref;
            } else if (*role == "right") {
                lanelet.right_way_id = *ref;
            } else if (*role == "centerline") {
                lanelet.centerline_way_id = *ref;
            }
        }

        if (!lanelet.left_way_id.empty() && !lanelet.right_way_id.empty()) {
            document.lanelets.push_back(std::move(lanelet));
        }
    }

    if (document.lanelets.empty()) {
        error = "No lanelet relations with left/right bounds found";
        return false;
    }

    return true;
}

std::vector<Point2D> wayPolyline(const ParsedWay& way,
                                 const std::unordered_map<std::string, ParsedNode>& nodes,
                                 std::string& error)
{
    std::vector<Point2D> polyline;
    polyline.reserve(way.node_refs.size());

    for (const std::string& node_ref : way.node_refs) {
        const auto it = nodes.find(node_ref);
        if (it == nodes.end() || !it->second.has_local_coords) {
            error = "Way " + way.id + " references unknown or uncoordinated node " + node_ref;
            return {};
        }
        polyline.push_back({it->second.x, it->second.y});
    }
    return polyline;
}

double polylineLength(const std::vector<Point2D>& polyline)
{
    double length = 0.0;
    for (std::size_t i = 1; i < polyline.size(); ++i) {
        const double dx = polyline[i].x - polyline[i - 1].x;
        const double dy = polyline[i].y - polyline[i - 1].y;
        length += std::sqrt(dx * dx + dy * dy);
    }
    return length;
}

Point2D interpolatePolyline(const std::vector<Point2D>& polyline, double t)
{
    if (polyline.empty()) {
        return {};
    }
    if (polyline.size() == 1 || t <= 0.0) {
        return polyline.front();
    }

    const double total = polylineLength(polyline);
    if (total <= 0.0 || t >= 1.0) {
        return polyline.back();
    }

    const double target = t * total;
    double walked = 0.0;
    for (std::size_t i = 1; i < polyline.size(); ++i) {
        const double dx = polyline[i].x - polyline[i - 1].x;
        const double dy = polyline[i].y - polyline[i - 1].y;
        const double segment = std::sqrt(dx * dx + dy * dy);
        if (walked + segment >= target) {
            const double ratio = segment > 0.0 ? (target - walked) / segment : 0.0;
            return {polyline[i - 1].x + ratio * dx, polyline[i - 1].y + ratio * dy};
        }
        walked += segment;
    }
    return polyline.back();
}

std::vector<Point2D> buildCenterlineFromBounds(const std::vector<Point2D>& left,
                                               const std::vector<Point2D>& right)
{
    const std::size_t sample_count = std::max(left.size(), right.size());
    if (sample_count < 2) {
        return {};
    }

    std::vector<Point2D> centerline;
    centerline.reserve(sample_count);

    for (std::size_t i = 0; i < sample_count; ++i) {
        const double t =
            sample_count > 1 ? static_cast<double>(i) / static_cast<double>(sample_count - 1)
                             : 0.0;
        const Point2D left_point = interpolatePolyline(left, t);
        const Point2D right_point = interpolatePolyline(right, t);
        centerline.push_back({(left_point.x + right_point.x) * 0.5,
                              (left_point.y + right_point.y) * 0.5});
    }
    return centerline;
}

struct BuiltLanelet {
    std::string relation_id;
    std::vector<std::string> lane_node_ids;
    std::string left_entry_osm;
    std::string left_exit_osm;
    std::string right_entry_osm;
    std::string right_exit_osm;
};

std::optional<LaneMapData> buildLaneMap(const ParsedOsmDocument& document, std::string& error)
{
    LaneMapData lanes;
    std::vector<BuiltLanelet> built_lanelets;
    built_lanelets.reserve(document.lanelets.size());

    for (const ParsedLanelet& lanelet : document.lanelets) {
        const auto left_it = document.ways.find(lanelet.left_way_id);
        const auto right_it = document.ways.find(lanelet.right_way_id);
        if (left_it == document.ways.end() || right_it == document.ways.end()) {
            error = "Lanelet " + lanelet.id + " missing left/right way";
            return std::nullopt;
        }

        std::vector<Point2D> centerline;
        if (!lanelet.centerline_way_id.empty()) {
            const auto center_it = document.ways.find(lanelet.centerline_way_id);
            if (center_it == document.ways.end()) {
                error = "Lanelet " + lanelet.id + " missing centerline way";
                return std::nullopt;
            }
            centerline = wayPolyline(center_it->second, document.nodes, error);
            if (centerline.empty()) {
                return std::nullopt;
            }
        } else {
            const std::vector<Point2D> left_poly =
                wayPolyline(left_it->second, document.nodes, error);
            const std::vector<Point2D> right_poly =
                wayPolyline(right_it->second, document.nodes, error);
            if (left_poly.empty() || right_poly.empty()) {
                return std::nullopt;
            }
            centerline = buildCenterlineFromBounds(left_poly, right_poly);
            if (centerline.size() < 2) {
                error = "Lanelet " + lanelet.id + " centerline requires at least 2 points";
                return std::nullopt;
            }
        }

        BuiltLanelet built;
        built.relation_id = lanelet.id;
        built.left_entry_osm = left_it->second.node_refs.front();
        built.left_exit_osm = left_it->second.node_refs.back();
        built.right_entry_osm = right_it->second.node_refs.front();
        built.right_exit_osm = right_it->second.node_refs.back();

        for (std::size_t i = 0; i < centerline.size(); ++i) {
            const std::string node_id = "ll_" + lanelet.id + "_n" + std::to_string(i);
            lanes.nodes.push_back({node_id, centerline[i].x, centerline[i].y});
            built.lane_node_ids.push_back(node_id);

            if (i > 0) {
                lanes.edges.push_back({built.lane_node_ids[i - 1], node_id, false});
            }
        }

        built_lanelets.push_back(std::move(built));
    }

    for (const BuiltLanelet& from_lanelet : built_lanelets) {
        for (const BuiltLanelet& to_lanelet : built_lanelets) {
            if (from_lanelet.relation_id == to_lanelet.relation_id) {
                continue;
            }
            if (from_lanelet.left_exit_osm == to_lanelet.left_entry_osm
                && from_lanelet.right_exit_osm == to_lanelet.right_entry_osm) {
                lanes.edges.push_back(
                    {from_lanelet.lane_node_ids.back(), to_lanelet.lane_node_ids.front(), false});
            }
        }
    }

    if (lanes.nodes.size() < 2 || lanes.edges.empty()) {
        error = "Import produced insufficient lane graph";
        return std::nullopt;
    }

    return lanes;
}

std::optional<LaneMapData> importXmlContent(const std::string& xml, OsmImportError* error)
{
    ParsedOsmDocument document;
    std::string parse_error;
    if (!parseOsmDocument(xml, document, parse_error)) {
        setError(error, parse_error);
        return std::nullopt;
    }

    std::string build_error;
    const auto lanes = buildLaneMap(document, build_error);
    if (!lanes.has_value()) {
        setError(error, build_error);
        return std::nullopt;
    }
    return lanes;
}

}  // namespace

std::optional<LaneMapData> OsmLaneletImporter::importFromFile(const std::string& path,
                                                              OsmImportError* error)
{
    std::ifstream input(path);
    if (!input.is_open()) {
        setError(error, "Cannot open OSM file: " + path);
        return std::nullopt;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return importXmlContent(buffer.str(), error);
}

std::optional<LaneMapData> OsmLaneletImporter::importFromXmlString(const std::string& xml,
                                                                   OsmImportError* error)
{
    return importXmlContent(xml, error);
}

}  // namespace fleetsim::domain::map
