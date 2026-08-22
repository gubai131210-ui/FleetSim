#include "HungarianAssigner.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace fleetsim::domain::scheduling {

namespace {

double pickupDistance(const core::VehicleState& vehicle, const core::Task& task)
{
    const double dx = task.pickup.x - vehicle.pose.x;
    const double dy = task.pickup.y - vehicle.pose.y;
    return std::sqrt(dx * dx + dy * dy);
}

// Square Hungarian minimizing cost. Returns assignment[row] = column (-1 if none).
std::vector<int> hungarianMinimize(std::vector<std::vector<double>> cost)
{
    const int n = static_cast<int>(cost.size());
    if (n == 0) {
        return {};
    }

    const double kInf = 1e100;
    std::vector<double> u(n + 1, 0.0);
    std::vector<double> v(n + 1, 0.0);
    std::vector<int> p(n + 1, 0);
    std::vector<int> way(n + 1, 0);

    for (int i = 1; i <= n; ++i) {
        p[0] = i;
        int j0 = 0;
        std::vector<double> minv(n + 1, kInf);
        std::vector<char> used(n + 1, false);
        do {
            used[j0] = true;
            const int i0 = p[j0];
            double delta = kInf;
            int j1 = 0;
            for (int j = 1; j <= n; ++j) {
                if (used[j]) {
                    continue;
                }
                const double cur = cost[static_cast<std::size_t>(i0 - 1)][static_cast<std::size_t>(j - 1)]
                                   - u[i0] - v[j];
                if (cur < minv[j]) {
                    minv[j] = cur;
                    way[j] = j0;
                }
                if (minv[j] < delta) {
                    delta = minv[j];
                    j1 = j;
                }
            }
            for (int j = 0; j <= n; ++j) {
                if (used[j]) {
                    u[p[j]] += delta;
                    v[j] -= delta;
                } else {
                    minv[j] -= delta;
                }
            }
            j0 = j1;
        } while (p[j0] != 0);

        do {
            const int j1 = way[j0];
            p[j0] = p[j1];
            j0 = j1;
        } while (j0 != 0);
    }

    std::vector<int> assignment(static_cast<std::size_t>(n), -1);
    for (int j = 1; j <= n; ++j) {
        if (p[j] != 0) {
            assignment[static_cast<std::size_t>(p[j] - 1)] = j - 1;
        }
    }
    return assignment;
}

}  // namespace

std::vector<TaskAssignment> HungarianAssigner::assign(
    const std::vector<core::Task>& pending_tasks,
    const std::vector<core::VehicleState>& idle_vehicles) const
{
    std::vector<TaskAssignment> result;
    if (pending_tasks.empty() || idle_vehicles.empty()) {
        return result;
    }

    const std::size_t n_tasks = pending_tasks.size();
    const std::size_t n_vehicles = idle_vehicles.size();
    const std::size_t n = std::max(n_tasks, n_vehicles);

    constexpr double kPad = 1e6;
    std::vector<std::vector<double>> cost(n, std::vector<double>(n, kPad));
    for (std::size_t t = 0; t < n_tasks; ++t) {
        for (std::size_t v = 0; v < n_vehicles; ++v) {
            cost[t][v] = pickupDistance(idle_vehicles[v], pending_tasks[t]);
        }
    }

    const std::vector<int> matching = hungarianMinimize(std::move(cost));
    for (std::size_t t = 0; t < n_tasks; ++t) {
        const int v = matching[t];
        if (v >= 0 && static_cast<std::size_t>(v) < n_vehicles) {
            result.push_back({pending_tasks[t].id, idle_vehicles[static_cast<std::size_t>(v)].id});
        }
    }
    return result;
}

}  // namespace fleetsim::domain::scheduling
