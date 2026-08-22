#include "MapLoader.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace fleetsim::domain::map {

namespace {

void markRect(OccupancyGrid& grid, double x, double y, double width, double height)
{
    const double x0 = x;
    const double y0 = y;
    const double x1 = x + width;
    const double y1 = y + height;

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

void markPolygon(OccupancyGrid& grid, const nlohmann::json& vertices)
{
    if (!vertices.is_array() || vertices.size() < 3) {
        return;
    }

    double min_x = vertices[0][0].get<double>();
    double max_x = min_x;
    double min_y = vertices[0][1].get<double>();
    double max_y = min_y;

    for (const auto& vertex : vertices) {
        const double x = vertex[0].get<double>();
        const double y = vertex[1].get<double>();
        min_x = std::min(min_x, x);
        max_x = std::max(max_x, x);
        min_y = std::min(min_y, y);
        max_y = std::max(max_y, y);
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
            std::size_t j = vertices.size() - 1;
            for (std::size_t i = 0; i < vertices.size(); ++i) {
                const double xi = vertices[i][0].get<double>();
                const double yi = vertices[i][1].get<double>();
                const double xj = vertices[j][0].get<double>();
                const double yj = vertices[j][1].get<double>();

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

nlohmann::json readJsonFile(const std::string& path)
{
    std::ifstream input(path);
    if (!input.is_open()) {
        throw std::runtime_error("Failed to open map file: " + path);
    }

    nlohmann::json json;
    input >> json;
    return json;
}

}  // namespace

MapMetadata MapLoader::loadMetadataFromFile(const std::string& map_json_path)
{
    const nlohmann::json json = readJsonFile(map_json_path);
    MapMetadata metadata;
    metadata.width_m = json.at("width_m").get<double>();
    metadata.height_m = json.at("height_m").get<double>();
    metadata.grid_resolution_m = json.at("grid_resolution_m").get<double>();
    return metadata;
}

OccupancyGrid MapLoader::loadFromFile(const std::string& map_json_path,
                                      double inflation_radius_m)
{
    const nlohmann::json json = readJsonFile(map_json_path);
    const double width_m = json.at("width_m").get<double>();
    const double height_m = json.at("height_m").get<double>();
    const double resolution_m = json.at("grid_resolution_m").get<double>();

    const int cols = static_cast<int>(std::ceil(width_m / resolution_m));
    const int rows = static_cast<int>(std::ceil(height_m / resolution_m));

    OccupancyGrid grid(rows, cols, resolution_m, width_m, height_m);

    if (json.contains("obstacles")) {
        for (const auto& obstacle : json.at("obstacles")) {
            const std::string type = obstacle.at("type").get<std::string>();
            if (type == "rect") {
                markRect(grid,
                         obstacle.at("x").get<double>(),
                         obstacle.at("y").get<double>(),
                         obstacle.at("width").get<double>(),
                         obstacle.at("height").get<double>());
            } else if (type == "polygon") {
                markPolygon(grid, obstacle.at("vertices"));
            }
        }
    }

    if (inflation_radius_m > 0.0) {
        grid.inflate(inflation_radius_m);
    }

    return grid;
}

}  // namespace fleetsim::domain::map
