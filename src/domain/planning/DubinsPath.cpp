#include "domain/planning/DubinsPath.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace fleetsim::domain::planning {

namespace {

constexpr double kTwoPi = 6.28318530717958647692;

double mod2pi(double angle)
{
    double a = std::fmod(angle, kTwoPi);
    if (a < 0.0) {
        a += kTwoPi;
    }
    return a;
}

double wrapToPi(double a)
{
    while (a > 3.14159265358979323846) {
        a -= kTwoPi;
    }
    while (a < -3.14159265358979323846) {
        a += kTwoPi;
    }
    return a;
}

using ST = DubinsPath::SegmentType;

struct Triple {
    double t{0.0};
    double p{0.0};
    double q{0.0};
    ST types[3]{ST::kStraight, ST::kStraight, ST::kStraight};
    bool ok{false};
};

bool dubinsLSL(double alpha, double beta, double d, Triple* out)
{
    const double tmp0 = d + std::sin(alpha) - std::sin(beta);
    const double p_sq = 2.0 + d * d - 2.0 * std::cos(alpha - beta)
        + 2.0 * d * (std::sin(alpha) - std::sin(beta));
    if (p_sq < 0.0) {
        return false;
    }
    const double tmp1 = std::atan2(std::cos(beta) - std::cos(alpha), tmp0);
    out->t = mod2pi(tmp1 - alpha);
    out->p = std::sqrt(p_sq);
    out->q = mod2pi(beta - tmp1);
    out->types[0] = ST::kLeft;
    out->types[1] = ST::kStraight;
    out->types[2] = ST::kLeft;
    out->ok = true;
    return true;
}

bool dubinsRSR(double alpha, double beta, double d, Triple* out)
{
    const double tmp0 = d - std::sin(alpha) + std::sin(beta);
    const double p_sq = 2.0 + d * d - 2.0 * std::cos(alpha - beta)
        - 2.0 * d * (std::sin(alpha) - std::sin(beta));
    if (p_sq < 0.0) {
        return false;
    }
    const double tmp1 = std::atan2(std::cos(alpha) - std::cos(beta), tmp0);
    out->t = mod2pi(alpha - tmp1);
    out->p = std::sqrt(p_sq);
    out->q = mod2pi(tmp1 - beta);
    out->types[0] = ST::kRight;
    out->types[1] = ST::kStraight;
    out->types[2] = ST::kRight;
    out->ok = true;
    return true;
}

bool dubinsLSR(double alpha, double beta, double d, Triple* out)
{
    const double p_sq = -2.0 + d * d + 2.0 * std::cos(alpha - beta)
        + 2.0 * d * (std::sin(alpha) + std::sin(beta));
    if (p_sq < 0.0) {
        return false;
    }
    const double p = std::sqrt(p_sq);
    const double tmp2 = std::atan2(-std::cos(alpha) - std::cos(beta),
                                   d + std::sin(alpha) + std::sin(beta))
        - std::atan2(-2.0, p);
    out->t = mod2pi(tmp2 - alpha);
    out->p = p;
    out->q = mod2pi(tmp2 - beta);
    out->types[0] = ST::kLeft;
    out->types[1] = ST::kStraight;
    out->types[2] = ST::kRight;
    out->ok = true;
    return true;
}

bool dubinsRSL(double alpha, double beta, double d, Triple* out)
{
    const double p_sq = -2.0 + d * d + 2.0 * std::cos(alpha - beta)
        - 2.0 * d * (std::sin(alpha) + std::sin(beta));
    if (p_sq < 0.0) {
        return false;
    }
    const double p = std::sqrt(p_sq);
    const double tmp2 = std::atan2(std::cos(alpha) + std::cos(beta),
                                   d - std::sin(alpha) - std::sin(beta))
        - std::atan2(2.0, p);
    out->t = mod2pi(alpha - tmp2);
    out->p = p;
    out->q = mod2pi(beta - tmp2);
    out->types[0] = ST::kRight;
    out->types[1] = ST::kStraight;
    out->types[2] = ST::kLeft;
    out->ok = true;
    return true;
}

bool dubinsRLR(double alpha, double beta, double d, Triple* out)
{
    const double tmp0 = (6.0 - d * d + 2.0 * std::cos(alpha - beta)
                         + 2.0 * d * (std::sin(alpha) - std::sin(beta)))
        / 8.0;
    if (std::abs(tmp0) > 1.0) {
        return false;
    }
    out->p = mod2pi(kTwoPi - std::acos(tmp0));
    out->t = mod2pi(alpha - std::atan2(std::cos(alpha) - std::cos(beta),
                                       d - std::sin(alpha) + std::sin(beta))
                    + mod2pi(out->p / 2.0));
    out->q = mod2pi(alpha - beta - out->t + mod2pi(out->p));
    out->types[0] = ST::kRight;
    out->types[1] = ST::kLeft;
    out->types[2] = ST::kRight;
    out->ok = true;
    return true;
}

bool dubinsLRL(double alpha, double beta, double d, Triple* out)
{
    const double tmp0 = (6.0 - d * d + 2.0 * std::cos(alpha - beta)
                         + 2.0 * d * (-std::sin(alpha) + std::sin(beta)))
        / 8.0;
    if (std::abs(tmp0) > 1.0) {
        return false;
    }
    out->p = mod2pi(kTwoPi - std::acos(tmp0));
    out->t = mod2pi(-alpha - std::atan2(std::cos(alpha) - std::cos(beta),
                                        d + std::sin(alpha) - std::sin(beta))
                    + out->p / 2.0);
    out->q = mod2pi(mod2pi(beta) - alpha - out->t + mod2pi(out->p));
    out->types[0] = ST::kLeft;
    out->types[1] = ST::kRight;
    out->types[2] = ST::kLeft;
    out->ok = true;
    return true;
}

}  // namespace

std::vector<DubinsPath::Segment> DubinsPath::shortestPath(const core::Pose& start,
                                                          const core::Pose& goal,
                                                          double turning_radius)
{
    if (turning_radius <= 1e-6) {
        return {};
    }

    const double dx = goal.x - start.x;
    const double dy = goal.y - start.y;
    const double D = std::hypot(dx, dy);
    const double d = D / turning_radius;
    const double theta = mod2pi(std::atan2(dy, dx));
    const double alpha = mod2pi(start.theta - theta);
    const double beta = mod2pi(goal.theta - theta);

    Triple candidates[6];
    dubinsLSL(alpha, beta, d, &candidates[0]);
    dubinsLSR(alpha, beta, d, &candidates[1]);
    dubinsRSL(alpha, beta, d, &candidates[2]);
    dubinsRSR(alpha, beta, d, &candidates[3]);
    dubinsRLR(alpha, beta, d, &candidates[4]);
    dubinsLRL(alpha, beta, d, &candidates[5]);

    const Triple* best = nullptr;
    double best_cost = std::numeric_limits<double>::infinity();
    for (const Triple& c : candidates) {
        if (!c.ok) {
            continue;
        }
        const double cost = c.t + c.p + c.q;
        if (cost < best_cost) {
            best_cost = cost;
            best = &c;
        }
    }
    if (best == nullptr) {
        return {};
    }

    // Scale: t,q are angles (rad); p is length in units of rho. Arc length = angle * rho.
    std::vector<Segment> out(3);
    const double params[3] = {best->t, best->p, best->q};
    for (int i = 0; i < 3; ++i) {
        out[i].type = best->types[i];
        if (out[i].type == SegmentType::kStraight) {
            out[i].length = params[i] * turning_radius;
        } else {
            out[i].length = params[i] * turning_radius;  // arc length
        }
    }
    return out;
}

double DubinsPath::length(const std::vector<Segment>& segments)
{
    double total = 0.0;
    for (const Segment& s : segments) {
        total += s.length;
    }
    return total;
}

core::Path DubinsPath::sample(const core::Pose& start,
                              const std::vector<Segment>& segments,
                              double turning_radius,
                              double step_m)
{
    if (segments.empty() || turning_radius <= 1e-6 || step_m <= 1e-9) {
        return {};
    }

    std::vector<core::Waypoint> pts;
    double x = start.x;
    double y = start.y;
    double th = start.theta;
    pts.push_back({x, y});

    for (const Segment& seg : segments) {
        if (seg.length <= 1e-12) {
            continue;
        }
        const int steps = std::max(1, static_cast<int>(std::ceil(seg.length / step_m)));
        const double ds = seg.length / static_cast<double>(steps);
        for (int i = 0; i < steps; ++i) {
            if (seg.type == SegmentType::kStraight) {
                x += ds * std::cos(th);
                y += ds * std::sin(th);
            } else {
                const double kappa = (seg.type == SegmentType::kLeft)
                    ? (1.0 / turning_radius)
                    : (-1.0 / turning_radius);
                const double dth = kappa * ds;
                x += (std::sin(th + dth) - std::sin(th)) / kappa;
                y += (-std::cos(th + dth) + std::cos(th)) / kappa;
                th = wrapToPi(th + dth);
            }
            pts.push_back({x, y});
        }
    }
    return core::Path(std::move(pts));
}

}  // namespace fleetsim::domain::planning
