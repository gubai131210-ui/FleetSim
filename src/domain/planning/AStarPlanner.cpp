#include "AStarPlanner.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <queue>
#include <unordered_map>
#include <vector>

namespace fleetsim::domain::planning {

namespace {

struct QueueNode {
    int row{0};
    int col{0};
    double f{0.0};

    bool operator>(const QueueNode& other) const
    {
        return f > other.f;
    }
};

std::int64_t cellKey(int row, int col)
{
    return (static_cast<std::int64_t>(row) << 32) ^ static_cast<std::uint32_t>(col);
}

}  // namespace

double AStarPlanner::octileHeuristic(int row0, int col0, int row1, int col1, double resolution_m)
{
    const int dx = std::abs(col1 - col0);
    const int dy = std::abs(row1 - row0);
    const int d_min = std::min(dx, dy);
    const int d_max = std::max(dx, dy);
    const double straight = resolution_m;
    const double diagonal = resolution_m * std::sqrt(2.0);
    return d_min * diagonal + (d_max - d_min) * straight;
}

core::Path AStarPlanner::plan(const map::OccupancyGrid& grid,
                            const core::Pose& start,
                            const core::Pose& goal) const
{
    const auto start_cell = grid.worldToCell(start.x, start.y);
    const auto goal_cell = grid.worldToCell(goal.x, goal.y);

    if (grid.isOccupied(start_cell.row, start_cell.col)
        || grid.isOccupied(goal_cell.row, goal_cell.col)) {
        return core::Path{};
    }

    const double resolution = grid.resolutionM();

    std::unordered_map<std::int64_t, std::pair<int, int>> came_from;
    std::unordered_map<std::int64_t, double> g_score;

    const auto start_key = cellKey(start_cell.row, start_cell.col);
    g_score[start_key] = 0.0;
    came_from[start_key] = {start_cell.row, start_cell.col};

    std::priority_queue<QueueNode, std::vector<QueueNode>, std::greater<QueueNode>> open;
    open.push({start_cell.row, start_cell.col,
               octileHeuristic(start_cell.row, start_cell.col,
                               goal_cell.row, goal_cell.col, resolution)});

    const int directions[8][2] = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1},
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
    };
    const double move_cost[8] = {
        1.0, 1.0, 1.0, 1.0,
        std::sqrt(2.0), std::sqrt(2.0), std::sqrt(2.0), std::sqrt(2.0)
    };

    std::unordered_map<std::int64_t, uint8_t> closed;

    while (!open.empty()) {
        const QueueNode current = open.top();
        open.pop();

        const auto current_key = cellKey(current.row, current.col);
        if (closed[current_key]) {
            continue;
        }
        closed[current_key] = 1;

        if (current.row == goal_cell.row && current.col == goal_cell.col) {
            std::vector<core::Waypoint> waypoints;
            int row = goal_cell.row;
            int col = goal_cell.col;
            while (true) {
                waypoints.push_back({grid.cellCenterX(col), grid.cellCenterY(row)});
                const auto key = cellKey(row, col);
                const auto parent = came_from[key];
                if (parent.first == row && parent.second == col) {
                    break;
                }
                row = parent.first;
                col = parent.second;
            }
            std::reverse(waypoints.begin(), waypoints.end());
            return core::Path(std::move(waypoints));
        }

        const double current_g = g_score[current_key];

        for (int i = 0; i < 8; ++i) {
            const int nr = current.row + directions[i][0];
            const int nc = current.col + directions[i][1];
            if (!grid.isInside(nr, nc) || grid.isOccupied(nr, nc)) {
                continue;
            }

            if (directions[i][0] != 0 && directions[i][1] != 0) {
                if (grid.isOccupied(current.row + directions[i][0], current.col)
                    || grid.isOccupied(current.row, current.col + directions[i][1])) {
                    continue;
                }
            }

            const auto neighbor_key = cellKey(nr, nc);
            const double tentative_g = current_g + move_cost[i] * resolution;
            const auto g_it = g_score.find(neighbor_key);
            if (g_it != g_score.end() && tentative_g >= g_it->second) {
                continue;
            }

            came_from[neighbor_key] = {current.row, current.col};
            g_score[neighbor_key] = tentative_g;
            const double f = tentative_g + octileHeuristic(nr, nc, goal_cell.row, goal_cell.col, resolution);
            open.push({nr, nc, f});
        }
    }

    return core::Path{};
}

}  // namespace fleetsim::domain::planning
