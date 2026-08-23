#include "MapSerializer.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace fleetsim::domain::map {

namespace {

void markRect(OccupancyGrid& grid, const RectObstacle& rect)
{
    const double x0 = rect.x;
    const double y0 = rect.y;
    const double x1 = rect.x + rect.width;
    const double y1 = rect.y + rect.height;

    const auto min_cell = grid.worldToCell(x0, y0);
    const auto max_cell = grid.worldToCell(x1, y1);

    const int row_min = std::max(0, std::min(min_cell.row, max_cell.row));
    const int row_max = std::min(grid.rows() - 1, std::max(min_cell.row, max_cell.row));
    const int col_min = std::max(0, std::min(min_cell.col, max_cell.col));
    const int col_max = std::min(grid.cols() - 1, std::max(min_cell.col, max_cell.col));

    for (int row = row_min; row <= row_max; ++row) {
        for (int col = col_min; col <= col_max; ++col) {
            const double cx = grid.cellCenterX(col);
            const double cy = grid.cellCenterY(row);
            if (cx >= x0 && cx <= x1 && cy >= y0 && cy <= y1) {
                grid.setOccupied(row, col, true);
            }
        }
    }
}

void markPolygon(OccupancyGrid& grid, const PolygonObstacle& polygon)
{
    if (polygon.vertices.size() < 3) {
        return;
    }

    double min_x = polygon.vertices.front().first;
    double max_x = min_x;
    double min_y = polygon.vertices.front().second;
    double max_y = min_y;

    for (const auto& vertex : polygon.vertices) {
        min_x = std::min(min_x, vertex.first);
        max_x = std::max(max_x, vertex.first);
        min_y = std::min(min_y, vertex.second);
        max_y = std::max(max_y, vertex.second);
    }

    const auto min_cell = grid.worldToCell(min_x, min_y);
    const auto max_cell = grid.worldToCell(max_x, max_y);

    const int row_min = std::max(0, std::min(min_cell.row, max_cell.row));
    const int row_max = std::min(grid.rows() - 1, std::max(min_cell.row, max_cell.row));
    const int col_min = std::max(0, std::min(min_cell.col, max_cell.col));
    const int col_max = std::min(grid.cols() - 1, std::max(min_cell.col, max_cell.col));

    for (int row = row_min; row <= row_max; ++row) {
        for (int col = col_min; col <= col_max; ++col) {
            const double px = grid.cellCenterX(col);
            const double py = grid.cellCenterY(row);

            bool inside = false;
            std::size_t j = polygon.vertices.size() - 1;
            for (std::size_t i = 0; i < polygon.vertices.size(); ++i) {
                const double xi = polygon.vertices[i].first;
                const double yi = polygon.vertices[i].second;
                const double xj = polygon.vertices[j].first;
                const double yj = polygon.vertices[j].second;

                const bool intersect = ((yi > py) != (yj > py))
                    && (px < (xj - xi) * (py - yi) / ((yj - yi) + 1e-12) + xi);
                if (intersect) {
                    inside = !inside;
                }
                j = i;
            }

            if (inside) {
                grid.setOccupied(row, col, true);
            }
        }
    }
}

LaneMapData parseLanesJson(const nlohmann::json& lanes_json)
{
    LaneMapData lanes;
    if (lanes_json.is_array()) {
        return lanes;
    }
    if (!lanes_json.is_object()) {
        return lanes;
    }

    if (lanes_json.contains("nodes")) {
        for (const auto& node_json : lanes_json.at("nodes")) {
            LaneNode node;
            node.id = node_json.at("id").get<std::string>();
            node.x = node_json.at("x").get<double>();
            node.y = node_json.at("y").get<double>();
            lanes.nodes.push_back(std::move(node));
        }
    }

    if (lanes_json.contains("edges")) {
        for (const auto& edge_json : lanes_json.at("edges")) {
            LaneEdge edge;
            edge.from = edge_json.at("from").get<std::string>();
            edge.to = edge_json.at("to").get<std::string>();
            edge.bidirectional = edge_json.value("bidirectional", false);
            lanes.edges.push_back(std::move(edge));
        }
    }

    return lanes;
}

nlohmann::json lanesToJson(const LaneMapData& lanes)
{
    nlohmann::json nodes = nlohmann::json::array();
    for (const LaneNode& node : lanes.nodes) {
        nodes.push_back({
            {"id", node.id},
            {"x", node.x},
            {"y", node.y},
        });
    }

    nlohmann::json edges = nlohmann::json::array();
    for (const LaneEdge& edge : lanes.edges) {
        edges.push_back({
            {"from", edge.from},
            {"to", edge.to},
            {"bidirectional", edge.bidirectional},
        });
    }

    return {
        {"nodes", nodes},
        {"edges", edges},
    };
}

}  // namespace

bool MapSerializer::isValidRect(const RectObstacle& rect)
{
    return rect.width > 0.0 && rect.height > 0.0;
}

bool MapSerializer::isValidPolygon(const PolygonObstacle& polygon)
{
    return polygon.vertices.size() >= 3;
}

MapDocument MapSerializer::fromJson(const nlohmann::json& json)
{
    MapDocument document;
    document.version = json.value("version", 1);
    document.width_m = json.at("width_m").get<double>();
    document.height_m = json.at("height_m").get<double>();
    document.grid_resolution_m = json.at("grid_resolution_m").get<double>();

    if (json.contains("obstacles")) {
        for (const auto& obstacle_json : json.at("obstacles")) {
            const std::string type = obstacle_json.at("type").get<std::string>();
            Obstacle obstacle;
            if (type == "rect") {
                obstacle.type = ObstacleType::Rect;
                obstacle.rect.x = obstacle_json.at("x").get<double>();
                obstacle.rect.y = obstacle_json.at("y").get<double>();
                obstacle.rect.width = obstacle_json.at("width").get<double>();
                obstacle.rect.height = obstacle_json.at("height").get<double>();
                if (!isValidRect(obstacle.rect)) {
                    continue;
                }
            } else if (type == "polygon") {
                obstacle.type = ObstacleType::Polygon;
                for (const auto& vertex : obstacle_json.at("vertices")) {
                    obstacle.polygon.vertices.emplace_back(vertex[0].get<double>(), vertex[1].get<double>());
                }
                if (!isValidPolygon(obstacle.polygon)) {
                    continue;
                }
            } else {
                continue;
            }
            document.obstacles.push_back(std::move(obstacle));
        }
    }

    if (json.contains("lanes")) {
        document.lanes = parseLanesJson(json.at("lanes"));
    }

    return document;
}

MapDocument MapSerializer::fromFile(const std::string& map_json_path)
{
    std::ifstream input(map_json_path);
    if (!input.is_open()) {
        throw std::runtime_error("Failed to open map file: " + map_json_path);
    }

    nlohmann::json json;
    input >> json;
    return fromJson(json);
}

nlohmann::json MapSerializer::toJson(const MapDocument& document)
{
    nlohmann::json json;
    json["version"] = document.version;
    json["width_m"] = document.width_m;
    json["height_m"] = document.height_m;
    json["grid_resolution_m"] = document.grid_resolution_m;
    json["lanes"] = lanesToJson(document.lanes);

    nlohmann::json obstacles = nlohmann::json::array();
    for (const Obstacle& obstacle : document.obstacles) {
        if (obstacle.type == ObstacleType::Rect) {
            obstacles.push_back({
                {"type", "rect"},
                {"x", obstacle.rect.x},
                {"y", obstacle.rect.y},
                {"width", obstacle.rect.width},
                {"height", obstacle.rect.height},
            });
        } else if (obstacle.type == ObstacleType::Polygon) {
            nlohmann::json vertices = nlohmann::json::array();
            for (const auto& vertex : obstacle.polygon.vertices) {
                vertices.push_back({vertex.first, vertex.second});
            }
            obstacles.push_back({
                {"type", "polygon"},
                {"vertices", vertices},
            });
        }
    }
    json["obstacles"] = obstacles;
    return json;
}

bool MapSerializer::saveToFile(const MapDocument& document, const std::string& map_json_path)
{
    std::ofstream output(map_json_path);
    if (!output.is_open()) {
        return false;
    }
    output << toJson(document).dump(2);
    return output.good();
}

OccupancyGrid MapSerializer::toOccupancyGrid(const MapDocument& document, double inflation_radius_m)
{
    const int cols = static_cast<int>(std::ceil(document.width_m / document.grid_resolution_m));
    const int rows = static_cast<int>(std::ceil(document.height_m / document.grid_resolution_m));

    OccupancyGrid grid(rows, cols, document.grid_resolution_m, document.width_m, document.height_m);

    for (const Obstacle& obstacle : document.obstacles) {
        if (obstacle.type == ObstacleType::Rect) {
            markRect(grid, obstacle.rect);
        } else if (obstacle.type == ObstacleType::Polygon) {
            markPolygon(grid, obstacle.polygon);
        }
    }

    if (inflation_radius_m > 0.0) {
        grid.inflate(inflation_radius_m);
    }

    return grid;
}

}  // namespace fleetsim::domain::map
