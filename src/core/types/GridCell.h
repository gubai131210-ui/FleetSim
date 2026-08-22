#pragma once

#include <cmath>

namespace fleetsim::core {

struct GridCell {
    int row{0};
    int col{0};

    bool operator==(const GridCell& other) const
    {
        return row == other.row && col == other.col;
    }
};

inline GridCell worldToGrid(double x_m, double y_m, double resolution_m)
{
    return GridCell{
        static_cast<int>(std::floor(y_m / resolution_m)),
        static_cast<int>(std::floor(x_m / resolution_m))};
}

inline double gridToWorldX(int col, double resolution_m)
{
    return (static_cast<double>(col) + 0.5) * resolution_m;
}

inline double gridToWorldY(int row, double resolution_m)
{
    return (static_cast<double>(row) + 0.5) * resolution_m;
}

}  // namespace fleetsim::core
