# ADR 021: Behavior UI 信息架构

## 状态

草案（2026-08-23，Phase 9 Session 0）

## 背景

Phase 7 将算法配置拆为 AlgorithmWorkbench 5 页 + 独立 ExperimentComparePanel。Phase 8 新增 LaneEditorPanel + RoutingPage（Tab 5）。Phase 9 引入 Behavior Tree **编排层**，须避免 Goal Agent 将 BT 监控/配置堆进 RoutingPage、MonitorPanel 或 MainWindow toolbar（Phase 7–8 已踩过的坑）。

## 决策

### 1. 用户心智分层

| 层次 | 用户心智 | FleetSim 落点 | 禁止 |
|------|---------|--------------|------|
| 环境几何 | 障碍物、边界 | MapEditorPanel | 与 lane/BT 混放 |
| 拓扑车道 | 节点、边 | LaneEditorPanel | 与 BT 混放 |
| 路由策略 | graph/freespace/hybrid | RoutingPage（Tab 5） | 与 BT 混放 |
| **行为编排** | 何时 plan/replan/recovery | **BehaviorPage（Tab 6）** + **BehaviorTreePanel** | 塞进 Routing/Planning |
| 算法参数 | planner/tracker/ST/pred | Workbench Tab 1–4 | 与 BT 树结构混放 |
| 运行监控 | 曲线、误差 | MonitorPanel | 加 BT 表单 |
| 实验对比 | 聚合指标、CSV 导出 | ExperimentComparePanel | 加 routing/BT 控件 |

### 2. BehaviorTreePanel（独立 Dock，Session 4）

路径：`src/ui/panels/BehaviorTreePanel.*`

**职责（只读监控）**：
- 当前树名 / 场景 behavior_tree_path
- active 节点路径 / 名称
- 上次 NodeStatus（SUCCESS / FAILURE / RUNNING）
- blackboard 摘要（path_valid、replan_requested、recovery_count…）

**禁止**：
- 在此编辑 JSON 树（编辑走场景文件 + 未来专用编辑器）
- 单 `.cpp` >300 行未拆 `BtStatusWidget`

### 3. BehaviorPage（Workbench Tab 6，Session 4）

路径：`src/ui/dialogs/pages/BehaviorPage.*`

**控件（仅此范围）**：
- `behavior_mode` 下拉：`legacy` | `bt`
- `behavior_tree_path` 文本或文件 picker
- `replan_hz`（double spin）
- `recovery_wait_ticks`（int spin）
- recovery 开关（可选，映射 scenario 字段）

**禁止**：
- planner、routing_mode、prediction、lane、tracker 控件
- `routing_mode` / `lane_snap` 关键词出现 → Reviewer **FAIL**

### 4. ExperimentComparePanel — CSV 导出（Session 5）

- **仅**增加 Export CSV 按钮 + 文件对话框（≤20 行增量）
- Domain `ExperimentMetrics::exportCsv(path)` 实现 CSV 写入
- **禁止** CSV 按钮出现在 MainWindow toolbar、ControlPanel、MonitorPanel、Workbench 其它页

### 5. MainWindow 挂载合同

```cpp
// MainWindow.cpp — BehaviorTreePanel dock，增量 ≤15 行
addDockWidget(Qt::RightDockWidgetArea, behavior_tree_panel_dock_);
```

- AlgorithmWorkbenchDialog 增加第 6 Tab `BehaviorPage`（Session 4）
- MainWindow **不**承载 BT 业务逻辑 >15 行

### 6. Reviewer 量化红线

| 检查 | 条件 | 结果 |
|------|------|------|
| RoutingPage | 出现 `behavior_mode` / `RecoveryNode` / `replan_hz` | **FAIL** |
| BehaviorPage | 出现 `routing_mode` / `lane_snap` / planner 下拉 | **FAIL** |
| ControlPanel / MonitorPanel | Export CSV 或 BT 表单 | **FAIL** |
| BehaviorTreePanel.cpp | >300 行且未拆 widget | **FAIL** |

### 7. 明确不做（Phase 9）

- BT 可视化树编辑器（Phase 10+）
- 在 RoutingPage 加 behavior_mode 快捷切换
- 在 LaneEditorPanel 显示 BT active 节点

## 后果

- Session 4 前 UI 文件仅 ADR + 接口占位；Session 0 不写 UI 实现
- `verify_phase9_evidence.py` 负向 grep：RoutingPage 无 behavior；BehaviorPage 无 routing_mode

## 参考

- ADR-017 Algorithm Workbench 分页先例
- ADR-019 LaneEditorPanel 独立 dock 先例
- Phase 9：`docs/PHASE9_GOAL_PROMPT.md` §4.5、§5
