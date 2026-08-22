#pragma once

#include "SimController.h"

#include <QObject>

namespace fleetsim::app {

class MonitorBridge : public QObject {
    Q_OBJECT

public:
    explicit MonitorBridge(SimController* controller, QObject* parent = nullptr);

    void bind();
    void unbind();
    void reset();

signals:
    void sampleReady(double sim_time_s,
                     double cross_track_error_m,
                     double heading_error_rad,
                     double linear_velocity_mps);

private:
    void computePathErrors(double x_m,
                           double y_m,
                           double theta_rad,
                           double* cross_track_error_m,
                           double* heading_error_rad) const;

    SimController* controller_{nullptr};
    int pose_subscription_id_{0};
    double sim_time_s_{0.0};
};

}  // namespace fleetsim::app
