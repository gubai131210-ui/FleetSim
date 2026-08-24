#include "domain/collision/CbsLiteCoordinator.h"

#include "core/types/Pose.h"
#include "core/types/Waypoint.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <deque>
#include <functional>
#include <optional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace fleetsim::domain::collision {

namespace {

using Cell = std::pair<int, int>;
using TimedPath = std::vector<Cell>;

struct SpacetimeConstraint {
    core::VehicleId agent;
    int time{0};
    int row{0};
    int col{0};
};

struct Conflict {
    int time{0};
    std::size_t agent_a{0};
    std::size_t agent_b{0};
    int row{0};
    int col{0};
};

struct CbsNode {
    std::vector<core::Path> paths;
    std::vector<SpacetimeConstraint> constraints;
    int depth{0};
};

struct SpacetimeState {
    int row{0};
    int col{0};
    int time{0};

    bool operator==(const SpacetimeState& other) const
    {
        return row == other.row && col == other.col && time == other.time;
    }
};

struct SpacetimeStateHash {
    std::size_t operator()(const SpacetimeState& state) const
    {
        return static_cast<std::size_t>((state.row * 73856093) ^ (state.col * 19349663)
                                        ^ (state.time * 83492791));
    }
};

std::int64_t cellKey(int row, int col)
{
    return (static_cast<std::int64_t>(row) << 32) ^ static_cast<std::uint32_t>(col);
}

Cell clampCell(const core::GridCell& cell, const map::OccupancyGrid& grid)
{
    return Cell{
        std::clamp(cell.row, 0, grid.rows() - 1),
        std::clamp(cell.col, 0, grid.cols() - 1),
    };
}

TimedPath pathToTimedCells(const core::Path& path, const map::OccupancyGrid& grid)
{
    TimedPath timed;
    timed.reserve(path.size());
    for (const core::Waypoint& waypoint : path.waypoints()) {
        timed.push_back(clampCell(grid.worldToCell(waypoint.x, waypoint.y), grid));
    }
    return timed;
}

void padTimedPaths(std::vector<TimedPath>& paths)
{
    std::size_t max_len = 0U;
    for (const TimedPath& path : paths) {
        max_len = std::max(max_len, path.size());
    }
    if (max_len == 0U) {
        return;
    }

    for (TimedPath& path : paths) {
        if (path.empty()) {
            continue;
        }
        const Cell last = path.back();
        while (path.size() < max_len) {
            path.push_back(last);
        }
    }
}

std::optional<Conflict> findFirstConflict(const std::vector<TimedPath>& paths)
{
    if (paths.size() < 2U) {
        return std::nullopt;
    }

    const std::size_t horizon = paths.front().size();
    for (std::size_t time = 0; time < horizon; ++time) {
        for (std::size_t i = 0; i < paths.size(); ++i) {
            for (std::size_t j = i + 1; j < paths.size(); ++j) {
                if (paths[i][time] == paths[j][time]) {
                    return Conflict{
                        static_cast<int>(time),
                        i,
                        j,
                        paths[i][time].first,
                        paths[i][time].second,
                    };
                }
            }
        }
    }
    return std::nullopt;
}

bool violatesConstraint(const core::VehicleId& agent,
                        int row,
                        int col,
                        int time,
                        const std::vector<SpacetimeConstraint>& constraints)
{
    for (const SpacetimeConstraint& constraint : constraints) {
        if (constraint.agent == agent && constraint.time == time && constraint.row == row
            && constraint.col == col) {
            return true;
        }
    }
    return false;
}

bool occupiedByOtherAgent(std::size_t self_index,
                          int row,
                          int col,
                          int time,
                          const std::vector<TimedPath>& other_paths)
{
    for (std::size_t index = 0; index < other_paths.size(); ++index) {
        if (index == self_index || other_paths[index].empty()) {
            continue;
        }
        const TimedPath& other = other_paths[index];
        const std::size_t sample_time =
            static_cast<std::size_t>(time) < other.size() ? static_cast<std::size_t>(time)
                                                          : other.size() - 1U;
        if (other[sample_time].first == row && other[sample_time].second == col) {
            return true;
        }
    }
    return false;
}

core::Path timedPathToWorldPath(const TimedPath& timed, const map::OccupancyGrid& grid)
{
    std::vector<core::Waypoint> waypoints;
    waypoints.reserve(timed.size());
    for (const Cell& cell : timed) {
        waypoints.push_back(
            {grid.cellCenterX(cell.second), grid.cellCenterY(cell.first)});
    }
    return core::Path(std::move(waypoints));
}

std::optional<TimedPath> planSpacetimePath(const map::OccupancyGrid& grid,
                                           const core::Pose& start,
                                           const core::Pose& goal,
                                           const core::VehicleId& agent_id,
                                           std::size_t agent_index,
                                           const std::vector<SpacetimeConstraint>& constraints,
                                           const std::vector<TimedPath>& other_paths,
                                           int max_time)
{
    const core::GridCell raw_start = grid.worldToCell(start.x, start.y);
    const core::GridCell raw_goal = grid.worldToCell(goal.x, goal.y);
    const Cell start_cell = clampCell(raw_start, grid);
    const Cell goal_cell = clampCell(raw_goal, grid);
    if (max_time <= 0) {
        return std::nullopt;
    }

    struct QueueNode {
        SpacetimeState state;
        double f{0.0};
    };

    auto heuristic = [&](int row, int col) {
        return grid.resolutionM()
               * (std::abs(goal_cell.first - row) + std::abs(goal_cell.second - col));
    };

    std::priority_queue<QueueNode, std::vector<QueueNode>, std::function<bool(const QueueNode&, const QueueNode&)>>
        open([](const QueueNode& lhs, const QueueNode& rhs) { return lhs.f > rhs.f; });

    std::unordered_map<SpacetimeState, SpacetimeState, SpacetimeStateHash> came_from;
    std::unordered_map<SpacetimeState, double, SpacetimeStateHash> g_score;
    std::unordered_set<SpacetimeState, SpacetimeStateHash> closed;

    const SpacetimeState start_state{start_cell.first, start_cell.second, 0};
    g_score[start_state] = 0.0;
    came_from[start_state] = start_state;
    open.push({start_state, heuristic(start_state.row, start_state.col)});

    const int directions[9][2] = {
        {0, 0},
        {-1, 0},
        {1, 0},
        {0, -1},
        {0, 1},
        {-1, -1},
        {-1, 1},
        {1, -1},
        {1, 1},
    };
    const double move_cost[9] = {
        1.0, 1.0, 1.0, 1.0, 1.0, 1.4142135623730951, 1.4142135623730951, 1.4142135623730951,
        1.4142135623730951,
    };

    while (!open.empty()) {
        const SpacetimeState current = open.top().state;
        open.pop();
        if (closed.count(current) != 0U) {
            continue;
        }
        closed.insert(current);

        if (current.row == goal_cell.first && current.col == goal_cell.second) {
            TimedPath timed;
            SpacetimeState trace = current;
            while (true) {
                timed.emplace_back(trace.row, trace.col);
                const auto parent_it = came_from.find(trace);
                if (parent_it == came_from.end()
                    || (parent_it->second.row == trace.row && parent_it->second.col == trace.col
                        && parent_it->second.time == trace.time)) {
                    break;
                }
                trace = parent_it->second;
            }
            std::reverse(timed.begin(), timed.end());
            return timed;
        }

        if (current.time + 1 > max_time) {
            continue;
        }

        const double current_g = g_score[current];

        for (int move = 0; move < 9; ++move) {
            const int next_row = current.row + directions[move][0];
            const int next_col = current.col + directions[move][1];
            const int next_time = current.time + 1;
            if (!grid.isInside(next_row, next_col) || grid.isOccupied(next_row, next_col)) {
                continue;
            }
            if (move >= 5) {
                if (grid.isOccupied(current.row + directions[move][0], current.col)
                    || grid.isOccupied(current.row, current.col + directions[move][1])) {
                    continue;
                }
            }
            if (violatesConstraint(agent_id, next_row, next_col, next_time, constraints)) {
                continue;
            }
            if (occupiedByOtherAgent(agent_index, next_row, next_col, next_time, other_paths)) {
                continue;
            }

            const SpacetimeState next_state{next_row, next_col, next_time};
            if (closed.count(next_state) != 0U) {
                continue;
            }

            const double tentative_g = current_g + move_cost[move] * grid.resolutionM();
            const auto score_it = g_score.find(next_state);
            if (score_it != g_score.end() && tentative_g >= score_it->second) {
                continue;
            }

            came_from[next_state] = current;
            g_score[next_state] = tentative_g;
            open.push({next_state, tentative_g + heuristic(next_row, next_col)});
        }
    }

    return std::nullopt;
}

core::Pose pathStartPose(const core::Path& path)
{
    core::Pose pose;
    if (!path.empty()) {
        pose.x = path.waypoints().front().x;
        pose.y = path.waypoints().front().y;
    }
    return pose;
}

core::Pose pathGoalPose(const core::Path& path)
{
    core::Pose pose;
    if (!path.empty()) {
        pose.x = path.waypoints().back().x;
        pose.y = path.waypoints().back().y;
    }
    return pose;
}

}  // namespace

CbsLiteResult CbsLiteCoordinator::resolve(const vehicle::FleetManager& fleet,
                                          const map::OccupancyGrid& grid,
                                          const CbsLiteConfig& config)
{
    CbsLiteResult result;
    result.success = false;
    result.constraints_added = 0;
    result.nodes_expanded = 0;

    if (config.max_depth <= 0) {
        result.message = "CBS-lite max_depth exhausted before search";
        return result;
    }

    if (fleet.count() < 2U || grid.rows() == 0) {
        result.message = "CBS-lite requires at least two agents and a valid map";
        return result;
    }

    std::vector<core::VehicleId> agent_ids;
    agent_ids.reserve(fleet.count());
    CbsNode root;
    root.paths.reserve(fleet.count());

    for (std::size_t i = 0; i < fleet.count(); ++i) {
        const vehicle::VehicleAgent& agent = fleet.agent(i);
        if (agent.vehicle == nullptr || agent.reference_path.empty()) {
            result.message = "All agents must have vehicles and initial reference paths";
            return result;
        }
        agent_ids.push_back(agent.vehicle->id());
        root.paths.push_back(agent.reference_path);
    }

    const auto start_time = std::chrono::steady_clock::now();
    std::deque<CbsNode> open;
    open.push_back(std::move(root));

    while (!open.empty()) {
        if (config.time_limit_ms > 0) {
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start_time);
            if (elapsed.count() >= config.time_limit_ms) {
                result.message = "CBS-lite time limit exceeded";
                result.nodes_expanded = result.nodes_expanded;
                return result;
            }
        }

        CbsNode node = std::move(open.front());
        open.pop_front();
        ++result.nodes_expanded;

        std::vector<TimedPath> timed_paths;
        timed_paths.reserve(node.paths.size());
        for (const core::Path& path : node.paths) {
            timed_paths.push_back(pathToTimedCells(path, grid));
        }
        padTimedPaths(timed_paths);

        const std::optional<Conflict> conflict = findFirstConflict(timed_paths);
        if (!conflict.has_value()) {
            result.success = true;
            result.paths = std::move(node.paths);
            result.message.clear();
            return result;
        }

        if (node.depth >= config.max_depth) {
            continue;
        }

        const int max_time =
            static_cast<int>(timed_paths.front().size()) + static_cast<int>(grid.rows());

        for (const std::size_t constrained_index : {conflict->agent_a, conflict->agent_b}) {
            CbsNode child = node;
            child.depth = node.depth + 1;
            child.constraints.push_back(SpacetimeConstraint{
                agent_ids[constrained_index],
                conflict->time,
                conflict->row,
                conflict->col,
            });
            ++result.constraints_added;

            const core::Path& original_path = child.paths[constrained_index];
            const std::optional<TimedPath> replanned = planSpacetimePath(
                grid,
                pathStartPose(original_path),
                pathGoalPose(original_path),
                agent_ids[constrained_index],
                constrained_index,
                child.constraints,
                timed_paths,
                max_time);
            if (!replanned.has_value()) {
                continue;
            }

            child.paths[constrained_index] = timedPathToWorldPath(*replanned, grid);
            open.push_back(std::move(child));
        }
    }

    result.message = "CBS-lite failed to find conflict-free joint paths";
    return result;
}

}  // namespace fleetsim::domain::collision
