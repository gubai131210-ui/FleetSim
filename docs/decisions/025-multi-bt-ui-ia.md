# ADR 025: 多车独立 BT + Phase 10 UI 信息架构

## 状态

草案（2026-08-24，Phase 10 Session 0；Session 4/6 实装）

## 背景

Phase 9 单车 `BtNavigator` + `BehaviorTreePanel` + `BehaviorPage`（Tab 6）。Phase 10 引入 OSM 导入、BT XML、CBS-lite、多车 BT 监控。Goal Agent 易把控件堆进现有 Panel — 本 ADR 写死 **UI 分层硬合同**。

## 决策

### 1. 多车 BT 隔离（Domain）

```cpp
class MultiBtNavigator {
public:
    BtNavigator& navigatorFor(const std::string& agent_id);
    BtBlackboard& blackboardFor(const std::string& agent_id);
    // per-agent tick; 禁止多车共用同一 blackboard 写 agent_id
};
```

- 每启用 BT 的 agent：**独立** `BtNavigator` + **独立** `BtBlackboard`
- scenario 默认树 + 可选 per-vehicle `behavior_tree_path`（vehicle 字段扩展）

### 2. UI 职责表（硬合同）

| 用户心智 | FleetSim 落点 | 禁止塞入 |
|---------|--------------|---------|
| 障碍几何 | MapEditorPanel | OSM / BT / CBS |
| 手工车道 | LaneEditorPanel | OSM 导入表单 |
| **HD Map 导入** | **OsmImportPanel** + **MapImportPage（Tab 7）** | Map/Lane 编辑器 |
| 路由策略 | RoutingPage | XML / CBS / OSM |
| 行为 JSON 时代 | BehaviorPage（Tab 6） | bt_format / spin / xml |
| **行为 XML / Recovery** | **BehaviorXmlPage（Tab 8）** | BehaviorPage / Routing |
| **多车 BT 监控** | **MultiAgentBehaviorPanel** | BehaviorTreePanel 混堆 |
| 协调策略 | CoordinationPage（+ cbs_lite） | behavior_tree / osm |
| 实验对比 | ExperimentComparePanel | OSM/BT |

### 3. Reviewer 量化红线

| 检查 | 条件 → FAIL |
|------|------------|
| MapEditorPanel.cpp / LaneEditorPanel.cpp | 出现 `osm` / `OsmImport` / `.osm` |
| BehaviorPage.cpp | 出现 `bt_format` / `RoundRobin` / `spin_rad` / `backup_dist` |
| RoutingPage.cpp | 出现 `cbs_lite` / `osm_path` |
| CoordinationPage.cpp | 出现 `behavior_tree` / `osm` |
| 任一新 Panel/Page .cpp | >300 行未拆 SubWidget |
| MainWindow | 单功能新增业务逻辑 >15 行 |

### 4. MainWindow 规则

- **仅挂载**新 dock / Workbench Tab；业务逻辑在 Panel/Page 内
- AlgorithmWorkbenchDialog 扩展 Tab 7–8；不合并 Tab

### 5. Session 0 范围

本 ADR Session 0 仅文档 + Domain 接口草案；UI 四件套 **Session 6** 实装：

- `OsmImportPanel`
- `MapImportPage`
- `BehaviorXmlPage`
- `MultiAgentBehaviorPanel`

## 后果

- 正面：可导航 UI；Reviewer 可静态审计
- 负面：文件数增加；需 verify_phase10 负向检查

## 禁止

- 禁止多车共用 Blackboard
- 禁止 Phase 10 UI 堆控件偷懒
- 禁止 Session 0 改 MainWindow（UI Session 6）

## 参考

- ADR-017、ADR-019、ADR-021
- `docs/UI_GUIDELINES.md`
- `docs/PHASE10_GOAL_PROMPT.md` §4.5、§5
