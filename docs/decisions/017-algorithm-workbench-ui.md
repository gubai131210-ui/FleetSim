# ADR 017: 算法实验工作台 UI 信息架构

## 状态

已接受（2026-08-23，Phase 7 Session 0 草案；Session 4–5 实装 Workbench + Compare Panel）

## 背景

Phase 6 将 planner / tracker / speed_planner / coordination 配置堆在单一 `PlannerTrackerDialog` 表单中，不利于系统化对比算法组合。Monitor 面板仅展示单车轨迹误差与速度，缺 **实验对比指标** 独立区域。Phase 7 重构 UI 信息架构，借鉴 Nav2 插件分层思想（**不实现** Behavior Tree 引擎）。

## 决策

### 1. AlgorithmWorkbenchDialog（写死结构）

```
AlgorithmWorkbenchDialog (QDialog + QStackedWidget / QTabWidget)
├── pages/PlanningPage.*       — planner: auto | astar | hybrid_astar
├── pages/ControlPage.*          — tracker: auto | pure_pursuit | stanley | mpc
├── pages/SpeedPage.*            — speed_planner: none | st_graph; prediction: none | constant_velocity
└── pages/CoordinationPage.*     — coordination: priority | none
```

| 规则 | 说明 |
|------|------|
| 每页单文件 | `.h/.cpp` 各一；单文件 ≤300 行 |
| 禁止 | 四类配置堆在同一 Form / 同一 ScrollArea |
| 禁止 | 在 `PlannerTrackerDialog` 继续加长表单 |
| 入口 | `PlannerTrackerDialog` 仅保留「打开 Algorithm Workbench…」转发（≤15 行）或 deprecated 注释 |

### 2. ExperimentComparePanel（独立 Dock）

- 路径：`src/ui/panels/ExperimentComparePanel.*`
- 职责：展示当前 run 与 baseline 的聚合指标（表格或迷你曲线），如 mean |cross-track|、min ST ref v、MPC solve rate。
- **禁止**塞进 `MonitorPanel` 底部、`ControlPanel` 或 `MainWindow` centralWidget。
- `MainWindow` 新增 dock 挂载 ≤15 行。

### 3. MonitorPanel 边界（不变）

- 保持：cross-track error、heading error、actual v、ST ref v 曲线。
- 实验 A vs B 对比 → **仅** `ExperimentComparePanel`。

### 4. App 桥接（可选 Session 5）

- `ExperimentBridge` 或扩展 `MonitorBridge`：将 `ExperimentMetrics::RunSummary` 推到 UI。
- Domain `ExperimentMetrics` 零 Qt；若在 App 层做 CSV 导出，放 `app/`。

### 5. MainWindow 挂载合同

```cpp
// MainWindow.cpp — 示例，总行数增量 ≤15
addDockWidget(..., experiment_compare_panel_dock_);
connect(workbench_dialog_, &AlgorithmWorkbenchDialog::settingsApplied, ...);
```

### 6. 明确不做

- 完整 Nav2 Behavior Tree 引擎
- Autoware Generator–Selector 多轨迹并行
- centralWidget 表单 / 曲线
- 在 Phase 7 删除 `PlannerTrackerDialog` 文件（可 deprecated，Session 4+ 薄封装）

## 后果

- 算法实验流程可分页配置、独立对比；Reviewer 以 **新建 Page/Panel 文件** 为硬验收。
- CMake 登记 `FleetSimUI` 新源文件；`src/ui/panels/README.md` 更新。

## 参考

- ADR-005 UI 页面与面板结构
- `docs/UI_GUIDELINES.md`
- Nav2 插件化 planner/controller 分层（边界内借鉴）
- Phase 7 `docs/PHASE7_GOAL_PROMPT.md` §5、§1.2-D
