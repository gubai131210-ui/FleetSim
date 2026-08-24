#pragma once

#include <QWidget>
#include <QString>
#include <QVector>

class QTableWidget;

namespace fleetsim::ui {

struct MultiAgentBehaviorRow {
    QString agent_id;
    QString tree_name;
    QString active_node;
    QString node_status;
    bool path_valid{false};
};

/// Read-only multi-agent BT monitor dock (ADR-025).
class MultiAgentBehaviorPanel : public QWidget {
    Q_OBJECT

public:
    explicit MultiAgentBehaviorPanel(QWidget* parent = nullptr);

public slots:
    void updateAgents(const QVector<MultiAgentBehaviorRow>& rows);

private:
    QTableWidget* table_{nullptr};
};

}  // namespace fleetsim::ui
