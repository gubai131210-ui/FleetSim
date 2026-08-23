#include "MonitorBridge.h"

#include "domain/experiment/ExperimentMetrics.h"
#include "core/types/Waypoint.h"
#include "domain/vehicle/FleetManager.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>

namespace fleetsim::app {

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kTwoPi = 2.0 * kPi;

double wrapToPi(double a)
{
    while (a > kPi) {
        a -= kTwoPi;
    }
    while (a < -kPi) {
        a += kTwoPi;
    }
    return a;
}

}  // namespace

class MonitorBridge::ExperimentMetricsHolder {
public:
    domain::experiment::ExperimentMetrics current;

    void reset()
    {
        current.reset();
        baseline = domain::experiment::RunSummary{};
        has_baseline = false;
    }

    void captureBaseline()
    {
        baseline = current.summarize();
        has_baseline = true;
    }

    domain::experiment::RunSummary baseline{};
    bool has_baseline{false};
};

MonitorBridge::MonitorBridge(SimController* controller, QObject* parent)
    : QObject(parent)
    , controller_(controller)
    , metrics_(new ExperimentMetricsHolder())
{
}

void MonitorBridge::bind()
{
    if (controller_ == nullptr || pose_subscription_id_ != 0) {
        return;
    }

    auto& bus = controller_->engine().eventBus();
    pose_subscription_id_ = bus.subscribe("sim/pose_updated", [this](const std::string& payload) {
        const nlohmann::json json = nlohmann::json::parse(payload);
        const double x = json.at("x").get<double>();
        const double y = json.at("y").get<double>();
        const double theta = json.value("theta", 0.0);
        const double linear_velocity = json.value("linear_velocity", 0.0);

        sim_time_s_ += controller_->engine().clock().fixedDt();
        double cross_track = 0.0;
        double heading_error = 0.0;
        computePathErrors(x, y, theta, &cross_track, &heading_error);

        double st_ref_speed = linear_velocity;
        bool mpc_ok = true;
        const auto& engine = controller_->engine();
        const domain::vehicle::VehicleAgent* agent = nullptr;
        if (!engine.selectedVehicleId().empty()) {
            agent = engine.fleet().findAgent(engine.selectedVehicleId());
        }
        if (agent == nullptr) {
            agent = engine.fleet().primaryAgent();
        }
        if (agent != nullptr && agent->speed_profile.speeds.size() == agent->reference_path.size() &&
            !agent->reference_path.empty()) {
            std::size_t best = 0;
            double best_d2 = std::numeric_limits<double>::infinity();
            const auto& w = agent->reference_path.waypoints();
            for (std::size_t i = 0; i < w.size(); ++i) {
                const double dx = x - w[i].x;
                const double dy = y - w[i].y;
                const double d2 = dx * dx + dy * dy;
                if (d2 < best_d2) {
                    best_d2 = d2;
                    best = i;
                }
            }
            st_ref_speed = agent->speed_profile.speeds[best];
            mpc_ok = agent->last_mpc_solve_ok;
        }

        domain::experiment::TickSample tick;
        tick.cross_track_error = cross_track;
        tick.heading_error = heading_error;
        tick.linear_velocity = linear_velocity;
        tick.st_ref_velocity = st_ref_speed;
        tick.mpc_last_solve_ok = mpc_ok;
        metrics_->current.recordTick(tick);
        emitSummary(metrics_->current.summarize());

        emit sampleReady(sim_time_s_, cross_track, heading_error, linear_velocity, st_ref_speed);
    });
}

void MonitorBridge::unbind()
{
    if (controller_ == nullptr || pose_subscription_id_ == 0) {
        return;
    }
    controller_->engine().eventBus().unsubscribe("sim/pose_updated", pose_subscription_id_);
    pose_subscription_id_ = 0;
}

void MonitorBridge::reset()
{
    sim_time_s_ = 0.0;
    metrics_->reset();
}

void MonitorBridge::captureBaseline()
{
    metrics_->captureBaseline();
    if (metrics_->has_baseline) {
        emit experimentBaselineUpdated(metrics_->baseline.mean_abs_cross_track,
                                       metrics_->baseline.mean_abs_heading_error,
                                       metrics_->baseline.min_st_ref_velocity,
                                       metrics_->baseline.mpc_solve_rate,
                                       static_cast<quint64>(metrics_->baseline.sample_count));
    }
}

void MonitorBridge::emitSummary(const domain::experiment::RunSummary& summary)
{
    emit experimentMetricsUpdated(summary.mean_abs_cross_track,
                                  summary.mean_abs_heading_error,
                                  summary.min_st_ref_velocity,
                                  summary.mpc_solve_rate,
                                  static_cast<quint64>(summary.sample_count));
}

void MonitorBridge::computePathErrors(double x_m,
                                      double y_m,
                                      double theta_rad,
                                      double* cross_track_error_m,
                                      double* heading_error_rad) const
{
    if (cross_track_error_m != nullptr) {
        *cross_track_error_m = 0.0;
    }
    if (heading_error_rad != nullptr) {
        *heading_error_rad = 0.0;
    }
    if (controller_ == nullptr) {
        return;
    }

    const auto& path = controller_->engine().referencePath();
    const auto& waypoints = path.waypoints();
    if (waypoints.size() < 2) {
        return;
    }

    double min_distance = std::numeric_limits<double>::max();
    double best_heading = 0.0;
    for (std::size_t i = 0; i + 1 < waypoints.size(); ++i) {
        const core::Waypoint& a = waypoints[i];
        const core::Waypoint& b = waypoints[i + 1];

        const double abx = b.x - a.x;
        const double aby = b.y - a.y;
        const double ab_len_sq = abx * abx + aby * aby;
        if (ab_len_sq < 1e-12) {
            continue;
        }

        const double t = std::clamp(((x_m - a.x) * abx + (y_m - a.y) * aby) / ab_len_sq, 0.0, 1.0);
        const double proj_x = a.x + t * abx;
        const double proj_y = a.y + t * aby;
        const double dx = x_m - proj_x;
        const double dy = y_m - proj_y;
        const double dist = std::sqrt(dx * dx + dy * dy);
        if (dist < min_distance) {
            min_distance = dist;
            best_heading = std::atan2(aby, abx);
        }
    }

    if (min_distance == std::numeric_limits<double>::max()) {
        return;
    }
    if (cross_track_error_m != nullptr) {
        *cross_track_error_m = min_distance;
    }
    if (heading_error_rad != nullptr) {
        *heading_error_rad = wrapToPi(theta_rad - best_heading);
    }
}

MonitorBridge::~MonitorBridge()
{
    delete metrics_;
}

}  // namespace fleetsim::app
