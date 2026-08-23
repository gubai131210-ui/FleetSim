#pragma once

#include <Eigen/Dense>

namespace fleetsim::domain::control {

/// Small dense QP: minimize 0.5 U'H U + g'U s.t. lo <= U <= hi (ADR-014).
/// Teaching MVP: cyclic coordinate descent with exact 1D solves (not single-shot
/// projected gradient).
class DenseQpSolver {
public:
    struct Result {
        Eigen::VectorXd u;
        bool ok{false};
    };

    static Result solveBox(const Eigen::MatrixXd& H,
                           const Eigen::VectorXd& g,
                           const Eigen::VectorXd& lo,
                           const Eigen::VectorXd& hi,
                           int max_iters = 200,
                           double tol = 1e-9);
};

}  // namespace fleetsim::domain::control
