# ADR 019: Lane 编辑 UI 信息架构

## 状态

已接受（2026-08-23，Phase 8 Session 7）

## 背景

Phase 7 ADR-017 将算法配置拆为 Algorithm Workbench 四页 + 独立 ExperimentComparePanel。`map.json` 预留 `lanes[]` 但未可视化/编辑。Phase 8 需 **车道拓扑** 与 **障碍物几何**、**路由策略**、**算法实验** 分层，防止 Agent 将 lane 工具堆进 MapEditorPanel 或 Workbench 既有页面。

## 决策

### 1. UI 分层表（写死）

| 层次 | 用户心智 | FleetSim 落点 | 禁止 |
|------|---------|--------------|------|
| 环境几何 | 障碍物、边界 | `MapEditorPanel` | 与 lane 混放 |
| 拓扑车道 | 节点、边、方向 | **`LaneEditorPanel`**（新 dock） | 塞进 MapEditor |
| 算法实验 | planner/tracker/ST/prediction | Workbench 既有四页 | 与 routing 混放 |
| 路由策略 | graph/freespace/hybrid | **`RoutingPage`**（Workbench 第 5 页） | 塞进 Speed/Planning |
| 运行监控 | 曲线、误差 | `MonitorPanel` | 加 lane 表单 |
| 实验对比 | 聚合指标 | `ExperimentComparePanel` | 加路由控件 |

### 2. LaneEditorPanel（独立 Dock）

- 路径：`src/ui/panels/LaneEditorPanel.*`
- 职责：**仅** lane 节点/边 CRUD、列表、选中高亮；与 MapView 双向同步。
- 可选子组件：`LaneNodeListWidget`、`LaneEdgeInspectorWidget`（超 300 行时拆分）。
- **禁止**在 `MapEditorPanel` 新增 lane 按钮列或边属性表单。

### 3. RoutingPage（Algorithm Workbench Tab 5）

- 路径：`src/ui/dialogs/pages/RoutingPage.*`
- 职责：**仅** `routing_mode`（freespace / lane_graph / hybrid）、lane snap 半径、first/last mile planner 选择。
- **禁止**在 `PlanningPage`、`SpeedPage`、`CoordinationPage` 增加 routing 控件。

### 4. MapView / MapScene

- `LaneGraphicsItem` 叠加显示节点与边折线。
- Lane 编辑模式与障碍物编辑模式 **分离**（不同 Panel 驱动不同 scene 交互层）。

### 5. MainWindow 挂载合同

```cpp
// MainWindow.cpp — LaneEditor dock 增量 ≤15 行
addDockWidget(Qt::LeftDockWidgetArea, lane_editor_panel_);
// View 菜单：View → Lane Editor
```

- Monitor / Compare / Control **不增** lane/routing 业务表单。

### 6. 行数红线

- 单 Panel/Page `.cpp` ≤ **300 行**；超出必须拆 SubWidget。

## 后果

- Session 4–5 实装 UI；Session 0 仅 ADR + Domain 测骨架。
- Reviewer 对照本 ADR + `docs/UI_GUIDELINES.md` 检查 MapEditorPanel 行数基线。

## 参考

- ADR-017 Algorithm Workbench 四页先例
- ADR-005 UI 页面结构
- `docs/PHASE8_GOAL_PROMPT.md` §4.4
