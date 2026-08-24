#pragma once

#include "core/types/GridCell.h"

#include <cstdint>
#include <vector>

namespace fleetsim::domain::map {

class OccupancyGrid {
public:
    OccupancyGrid() = default;

    OccupancyGrid(int rows, int cols, double resolution_m, double width_m, double height_m);

    int rows() const { return rows_; }
    int cols() const { return cols_; }
    double resolutionM() const { return resolution_m_; }
    double widthM() const { return width_m_; }
    double heightM() const { return height_m_; }

    bool isInside(int row, int col) const;
    bool isOccupied(int row, int col) const;
    void setOccupied(int row, int col, bool occupied = true);

    core::GridCell worldToCell(double x_m, double y_m) const;
    double cellCenterX(int col) const;
    double cellCenterY(int row) const;

    void inflate(double radius_m);
    void clearInflation();
    bool hasInflationBase() const { return !base_cells_.empty(); }
    std::size_t occupiedCellCount() const;

    const std::vector<uint8_t>& data() const { return cells_; }

private:
    int index(int row, int col) const;

    int rows_{0};
    int cols_{0};
    double resolution_m_{0.1};
    double width_m_{0.0};
    double height_m_{0.0};
    std::vector<uint8_t> cells_;
    std::vector<uint8_t> base_cells_;
};

}  // namespace fleetsim::domain::map
