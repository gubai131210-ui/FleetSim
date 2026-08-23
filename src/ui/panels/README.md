# UI Panels 目录

每个功能面板独立一个 Widget，由 `MainWindow` 通过 `QDockWidget` 或独立 `QDialog` 挂载。

## 已有面板

| 面板 | 文件 | 职责 |
|------|------|------|
| ControlPanel | `ControlPanel.*` | 仿真播放/暂停/单步/倍率 | Phase 1 |
| MonitorPanel | `MonitorPanel.*` | 误差/速度/ST 参考速度曲线 | Phase 2+6 |
| MapEditorPanel | `MapEditorPanel.*` | 地图编辑 | Phase 2 |
| TaskPanel | `TaskPanel.*` | 任务列表与 Add Task | Phase 3 |
| VehicleInfoPanel | `VehicleInfoPanel.*` | 选中车辆详情 | Phase 3 |
| SettingsDialog | `dialogs/SettingsDialog.*` | 模型/轴距/分配器 | Phase 4 |
| PlannerTrackerDialog | `dialogs/PlannerTrackerDialog.*` | deprecated → Algorithm Workbench | Phase 7 |
| AlgorithmWorkbenchDialog | `dialogs/AlgorithmWorkbenchDialog.*` + `dialogs/pages/*Page.*` | Planning/Control/Speed/Coordination/Routing/**Behavior** 六页 | Phase 7+8+9 |
| ExperimentComparePanel | `ExperimentComparePanel.*` | 实验对比 Current/Baseline + Export CSV | Phase 7+9 |
| LaneEditorPanel | `LaneEditorPanel.*` | lane 节点/边 CRUD（独立 dock） | Phase 8 Session 5 |
| BehaviorTreePanel | `BehaviorTreePanel.*` | BT 状态监控（只读 dock） | Phase 9 Session 4 |

## 已有图元（MapView 层）

| 图元 | 文件 | 职责 |
|------|------|------|
| LaneGraphicsItem | `graphics/LaneGraphicsItem.*` | lane 节点/边叠加（独立 lane 层） | Phase 8 Session 4 |

## Workbench 页面（`dialogs/pages/`）

| 页面 | Tab | 职责 |
|------|-----|------|
| PlanningPage | 1 | planner 选择 |
| ControlPage | 2 | tracker 选择 |
| SpeedPage | 3 | ST / speed planner |
| CoordinationPage | 4 | 多车协调 |
| RoutingPage | 5 | routing_mode |
| BehaviorPage | 6 | behavior_mode / tree / replan_hz / recovery |

## 新增面板流程

1. 在本目录新建 `XxxPanel.h/.cpp`
2. 更新 `src/ui/CMakeLists.txt`
3. 在 `MainWindow` 仅添加 dock / 菜单挂载（≤10 行）
4. 在 `SESSION_LOG.md` 记录新面板路径
