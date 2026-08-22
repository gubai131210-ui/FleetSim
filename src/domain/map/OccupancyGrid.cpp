#include "OccupancyGrid.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

namespace fleetsim::domain::map {

OccupancyGrid::OccupancyGrid(int rows, int cols, double resolution_m,
                             double width_m, double height_m)
    : rows_(rows)
    , cols_(cols)
    , resolution_m_(resolution_m)
    , width_m_(width_m)
    , height_m_(height_m)
    , cells_(static_cast<std::size_t>(rows * cols), 0)
{
}

bool OccupancyGrid::isInside(int row, int col) const
{
    return row >= 0 && row < rows_ && col >= 0 && col < cols_;
}

bool OccupancyGrid::isOccupied(int row, int col) const
{
    if (!isInside(row, col)) {
        return true;
    }
    return cells_[static_cast<std::size_t>(index(row, col))] != 0;
}

void OccupancyGrid::setOccupied(int row, int col, bool occupied)
{
    if (!isInside(row, col)) {
        return;
    }
    cells_[static_cast<std::size_t>(index(row, col))] = occupied ? 1 : 0;
}

core::GridCell OccupancyGrid::worldToCell(double x_m, double y_m) const
{
    return core::worldToGrid(x_m, y_m, resolution_m_);
}

double OccupancyGrid::cellCenterX(int col) const
{
    return core::gridToWorldX(col, resolution_m_);
}

double OccupancyGrid::cellCenterY(int row) const
{
    return core::gridToWorldY(row, resolution_m_);
}

void OccupancyGrid::inflate(double radius_m)
{
    if (radius_m <= 0.0) {
        return;
    }

    const int radius_cells = static_cast<int>(std::ceil(radius_m / resolution_m_));
    std::vector<uint8_t> inflated = cells_;

    for (int row = 0; row < rows_; ++row) {
        for (int col = 0; col < cols_; ++col) {
            if (!isOccupied(row, col)) {
                continue;
            }

            for (int dr = -radius_cells; dr <= radius_cells; ++dr) {
                for (int dc = -radius_cells; dc <= radius_cells; ++dc) {
                    const int nr = row + dr;
                    const int nc = col + dc;
                    if (!isInside(nr, nc)) {
                        continue;
                    }
                    const double dist = std::sqrt(static_cast<double>(dr * dr + dc * dc)) * resolution_m_;
                    if (dist <= radius_m) {
                        inflated[static_cast<std::size_t>(index(nr, nc))] = 1;
                    }
                }
            }
        }
    }

    cells_ = std::move(inflated);
}

int OccupancyGrid::index(int row, int col) const
{
    return row * cols_ + col;
}

}  // namespace fleetsim::domain::map
