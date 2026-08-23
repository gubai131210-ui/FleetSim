#include "domain/control/DenseQpSolver.h"

#include <gtest/gtest.h>

#include <Eigen/Dense>

using fleetsim::domain::control::DenseQpSolver;

TEST(DenseQpSolverTest, BoxQpPullsTowardUnconstrainedThenClamps)
{
    // min 0.5 (u-3)^2 on [-1,1]  <=>  H=1, g=-3, lo=-1, hi=1 → u*=1
    Eigen::MatrixXd H(1, 1);
    H(0, 0) = 1.0;
    Eigen::VectorXd g(1);
    g(0) = -3.0;
    Eigen::VectorXd lo(1);
    lo(0) = -1.0;
    Eigen::VectorXd hi(1);
    hi(0) = 1.0;

    const auto result = DenseQpSolver::solveBox(H, g, lo, hi);
    ASSERT_TRUE(result.ok);
    ASSERT_EQ(result.u.size(), 1);
    EXPECT_NEAR(result.u(0), 1.0, 1e-6);
}

TEST(DenseQpSolverTest, RejectsInvalidBounds)
{
    Eigen::MatrixXd H = Eigen::MatrixXd::Identity(1, 1);
    Eigen::VectorXd g = Eigen::VectorXd::Zero(1);
    Eigen::VectorXd lo(1);
    lo(0) = 2.0;
    Eigen::VectorXd hi(1);
    hi(0) = -2.0;
    const auto result = DenseQpSolver::solveBox(H, g, lo, hi);
    EXPECT_FALSE(result.ok);
}
