#pragma once

#include <QWidget>
#include <QString>

class QComboBox;
class QSpinBox;
class QWidget;

namespace fleetsim::ui {

class CoordinationPage : public QWidget {
    Q_OBJECT

public:
    explicit CoordinationPage(QWidget* parent = nullptr);

    QString coordination() const;
    int cbsMaxDepth() const;
    int cbsTimeLimitMs() const;

    void setCoordination(const QString& coordination);
    void setCbsMaxDepth(int depth);
    void setCbsTimeLimitMs(int milliseconds);

private:
    void refreshCbsFieldsVisibility();

    QComboBox* coordination_combo_{nullptr};
    QWidget* cbs_fields_{nullptr};
    QSpinBox* cbs_depth_spin_{nullptr};
    QSpinBox* cbs_time_spin_{nullptr};
};

}  // namespace fleetsim::ui
