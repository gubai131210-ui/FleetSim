#include "domain/control/DenseQpSolver.h"

#include <algorithm>
#include <cmath>

namespace fleetsim::domain::control {

DenseQpSolver::Result DenseQpSolver::solveBox(const Eigen::MatrixXd& H,
                                              const Eigen::VectorXd& g,
                                              const Eigen::VectorXd& lo,
                                              const Eigen::VectorXd& hi,
                                              int max_iters,
                                              double tol)
{
    Result result;
    const int n = static_cast<int>(g.size());
    if (n <= 0 || H.rows() != n || H.cols() != n || lo.size() != n || hi.size() != n) {
        return result;
    }
    for (int i = 0; i < n; ++i) {
        if (!std::isfinite(g(i)) || !std::isfinite(lo(i)) || !std::isfinite(hi(i)) ||
            lo(i) > hi(i)) {
            return result;
        }
        for (int j = 0; j < n; ++j) {
            if (!std::isfinite(H(i, j))) {
                return result;
            }
        }
    }

    Eigen::VectorXd u = Eigen::VectorXd::Zero(n);
    for (int i = 0; i < n; ++i) {
        u(i) = std::clamp(0.0, lo(i), hi(i));
    }

    for (int iter = 0; iter < max_iters; ++iter) {
        double max_delta = 0.0;
        for (int i = 0; i < n; ++i) {
            const double Hii = H(i, i);
            if (!(Hii > 1e-12)) {
                // Flat direction: keep clamped previous.
                const double clamped = std::clamp(u(i), lo(i), hi(i));
                max_delta = std::max(max_delta, std::abs(clamped - u(i)));
                u(i) = clamped;
                continue;
            }
            double linear = g(i);
            for (int j = 0; j < n; ++j) {
                if (j == i) {
                    continue;
                }
                linear += H(i, j) * u(j);
            }
            const double u_star = std::clamp(-linear / Hii, lo(i), hi(i));
            max_delta = std::max(max_delta, std::abs(u_star - u(i)));
            u(i) = u_star;
        }
        if (max_delta < tol) {
            result.u = u;
            result.ok = true;
            return result;
        }
    }

    result.u = u;
    result.ok = u.allFinite();
    return result;
}

}  // namespace fleetsim::domain::control
