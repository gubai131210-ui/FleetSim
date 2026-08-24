#pragma once

#include "SimController.h"

#include <QObject>
#include <QString>
#include <QVector>

namespace fleetsim::domain::experiment {
struct RunSummary;
}

namespace fleetsim::app {

struct AgentBehaviorSnapshot {
    QString agent_id;
    QString tree_name;
    QString active_node;
    QString node_status;
    bool path_valid{false};
};

class MonitorBridge : public QObject {
    Q_OBJECT

public:
    explicit MonitorBridge(SimController* controller, QObject* parent = nullptr);
    ~MonitorBridge() override;

    void bind();
    void unbind();
    void reset();
    void captureBaseline();

    bool exportCurrentMetricsCsv(const std::string& path) const;

signals:
    void sampleReady(double sim_time_s,
                     double cross_track_error_m,
                     double heading_error_rad,
                     double linear_velocity_mps,
                     double st_reference_speed_mps);
    void experimentMetricsUpdated(double mean_abs_cross_track,
                                  double mean_abs_heading_error,
                                  double min_st_ref_velocity,
                                  double mpc_solve_rate,
                                  quint64 sample_count);
    void experimentBaselineUpdated(double mean_abs_cross_track,
                                   double mean_abs_heading_error,
                                   double min_st_ref_velocity,
                                   double mpc_solve_rate,
                                   quint64 sample_count);

    void behaviorTreeStatusUpdated(const QString& behavior_mode,
                                   const QString& tree_name,
                                   const QString& active_node,
                                   const QString& node_status,
                                   bool path_valid,
                                   bool replan_requested,
                                   int recovery_count);

    void multiAgentBehaviorUpdated(const QVector<AgentBehaviorSnapshot>& agents);

private:
    void emitSummary(const domain::experiment::RunSummary& summary);
    void emitBehaviorTreeStatus();
    void emitMultiAgentBehaviorStatus();
    void computePathErrors(double x_m,
                           double y_m,
                           double theta_rad,
                           double* cross_track_error_m,
                           double* heading_error_rad) const;

    SimController* controller_{nullptr};
    int pose_subscription_id_{0};
    int tick_subscription_id_{0};
    double sim_time_s_{0.0};
    class ExperimentMetricsHolder;
    ExperimentMetricsHolder* metrics_{nullptr};
};

}  // namespace fleetsim::app
