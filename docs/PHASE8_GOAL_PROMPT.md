# Phase 8 Goal 模式执行提示词（完整版）

> **用途**：新开对话后，整段复制「§0 一键粘贴 Prompt」给 Agent（Goal 模式）。  
> **仓库**：https://github.com/gubai131210-ui/FleetSim.git  
> **本地路径**：`d:\QT_CppPrograms\FleetSim\FleetSim`  
> **前置**：Phase 0–7 ✅（CV 预测 → ST、AlgorithmWorkbench 四页、ExperimentComparePanel；FleetSimTests **118/118** 绿；`verify_phase7_evidence.py` **39 PASS**）。  
> **本阶段主题**：**车道级路由图（LaneGraph）真实现** + **First/Last Mile 混合规划** + **Lane 编辑 UI 分层**；与 Phase 5–7 的 Hybrid/MPC/ST/Prediction/实验工作台 **并存、可切换、不堆控件**。  
> **禁止**：编辑 `.cursor/plans/`；完整 Lanelet2/Autoware/nav2 依赖；完整 Behavior Tree 引擎；神经网络；Agent 不代跑中文 Temp 破坏性操作；中文路径 Qt Build 由用户验证（Agent 可在 **ASCII** 路径如 `D:\build\FleetSim_phase8_*` 外置构建取证）。

---

## §0 一键粘贴 Prompt（Goal 模式直接用）

```
你是 FleetSim Phase 8 的执行 Agent（Goal 模式）。
仓库：d:\QT_CppPrograms\FleetSim\FleetSim
远程：https://github.com/gubai131210-ui/FleetSim.git
权威说明：docs/PHASE8_GOAL_PROMPT.md（必须先完整阅读再写代码）

【强制阅读顺序 — 未读完禁止写代码】
1. docs/PHASE8_GOAL_PROMPT.md（全文，尤其 §1–§12）
2. docs/DEVELOPMENT_PLAN.md、docs/learning-path.md（确认 Phase 7 ✅、Phase 8 目标）
3. docs/AGENT_SESSION_TEMPLATE.md
4. SESSION_LOG.md（Phase 7 Session 7 终审；防回归：prediction/ST/MPC/Workbench/Compare）
5. docs/decisions/016-constant-velocity-prediction.md、017-algorithm-workbench-ui.md、002-map-format.md、001-architecture.md
6. docs/UI_GUIDELINES.md + src/ui/panels/README.md + docs/MUTATION_CHECKLIST.md
7. 代码优先：LaneGraph.* / MapSerializer.* / SimEngine::planPath* / AStarPlanner / HybridAStarPlanner /
   AlgorithmWorkbenchDialog + pages/ / MapEditorPanel / MapView / MapScene /
   ExperimentComparePanel / tests/CMakeLists.txt

【Phase 8 完成定义（全部达成才可结束 Goal）】
A. LaneGraph（Domain）：map.json lanes[] → 有向图（节点+边+代价）；Dijkstra/A* 求 lane 序列；单测覆盖无路径/单通道/分叉
B. LaneRouter / RoutePlanner：输出 LanePath（lane id 序列）→ 折线 centerline → 可接 Smoother → reference_path；禁止空壳仍返回 true
C. First/Last Mile：routing_mode=hybrid 时 — 自由空间 planner 连接 pose↔最近 lane 节点；lane 段走 graph；终点同理；单测证明 hybrid ≠ 纯 freespace ≠ 纯 lane
D. SimEngine/scenario：simulation.routing_mode: freespace | lane_graph | hybrid（默认 freespace 保回归）；planPath 读 LaneGraph；禁止字段写了却不读
E. map.json lanes[] 序列化：MapSerializer/MapLoader round-trip；LaneGraphTest + MapSerializer lane 字段测
F. UI 信息架构（硬合同 — 防堆控件）：
   - 新建 LaneEditorPanel（独立 dock）：仅 lane 节点/边 CRUD、选中高亮；禁止塞进 MapEditorPanel
   - AlgorithmWorkbench 新建第 5 页 RoutingPage（仅 routing_mode / snap 半径 / first-last 策略）；禁止堆进 PlanningPage 或 SpeedPage
   - MapView/MapScene：LaneGraphicsItem 叠加显示；编辑模式与障碍物编辑分离
   - MainWindow 挂载 LaneEditor dock ≤15 行；Monitor/Compare/Control 不增业务表单
G. scenario：assets/scenarios/lane_routing_demo（含 lanes[]，≥1 分叉，routing 可演示 first/last mile）
H. CMake 登记；Domain 零 Qt；保留 target_include_directories / target_link_libraries
I. 文档：ADR-018（LaneGraph + hybrid routing）、ADR-019（Lane 编辑 UI IA）；DEVELOPMENT_PLAN Phase8✅；SESSION_LOG；MUTATION M37+
J. 测试：LaneGraphTest、LaneRouterTest、FirstLastMileIntegrationTest、MapSerializerLaneTest、Phase7/6 回归；
    ASCII Build + FleetSimTests 全绿 + verify_phase8_evidence.py 才可 complete
K. 每会话 commit + push；四角色互相监督 PASS；回复四段式

【架构硬约束】
UI → App → Domain → Core
Domain/Core 禁止 #include <Qt*> 与 rclcpp
App 层一律 domain:: 前缀命名空间
接口先于实现：先 .h 后 .cpp
新 UI = 新 Panel/Dialog + 独立 Page；MainWindow 仅挂载
禁止编辑 .cursor/plans/
中文路径：ProjectManager 测试用仓库内 ASCII test_tmp

【四角色子 Agent 团队 — 每会话强制】
1. Planner（计划）：mini-plan — 允许改动 / NOT DO / 文件清单 / 测试清单 / UI 分页验收
2. Executor（执行）：只按 mini-plan 写代码与 CMake；禁止扩 scope
3. Tester（测试）：写/跑/补 GTest（ASCII D:\build\FleetSim_phase8_*）；假 LaneGraph（仍 empty stub）→ FAIL；UI 堆控件 → FAIL
4. Reviewer（检查）：对照 §10 + UI_GUIDELINES + Phase2–7 防回归；输出 PASS/FAIL
流程：Planner → Executor → Tester → Reviewer。主 Agent 禁止自评 PASS。SESSION_LOG 须引用四角色结论。

【建议 8 会话，可连续但禁止跳验收】
0: ADR-018/019 草案 + lanes[] schema + 红灯测骨架（LaneGraph*/LaneRouter*）
1: LaneGraph 图构建 + Dijkstra + LaneGraphTest 转绿
2: LaneRouter centerline + MapSerializer lanes round-trip
3: SimEngine hybrid + first/last mile + FirstLastMileIntegrationTest
4: UI — LaneGraphicsItem + MapView 叠加
5: UI — LaneEditorPanel + RoutingPage（Workbench 第 5 页）
6: lane_routing_demo + Phase5–7 回归
7: MUTATION M37+ + verify_phase8 + 四角色终审 + push + Phase8 ✅

【禁止偷懒 — 执行 Agent 必查】
1. 禁止 LaneGraph 仍为空 stub（nodeCount 永远 0）却标 Phase8 ✅
2. 禁止 routing_mode 写了但 planPath 仍只调 A* 不读 graph
3. 禁止把 Lane 编辑工具堆进 MapEditorPanel（障碍物与车道不同层次）
4. 禁止把 Routing 配置堆进 SpeedPage/PlanningPage/ExperimentComparePanel
5. 禁止在一个 Panel 里同时放「lane 列表 + 边属性 + 路由模式 + 预测 + MPC」
6. 禁止 Domain 写 Qt / rclcpp
7. 禁止新 Domain 类无 GTest
8. 禁止削 CMake target_*
9. 禁止破坏 Phase7 prediction/ST/Workbench/Compare 回归
10. 禁止完整 Lanelet2/Autoware/nav2/BT/感知
11. 禁止 SESSION_LOG 缺四角色 / 缺「没做什么」
12. 禁止未更新 CMakeLists 就加源文件
13. 禁止 Phase8 ✅ 但未新建 LaneEditorPanel 与 RoutingPage

从会话 0 开始立刻执行。中间不要换模型。
```

---

## §1 Phase 8 目标与验收标准

### 1.1 为何是 Phase 8（与学习路径对齐）

| 已完成（Phase 7） | Phase 8 要补的缺口 | 自动驾驶对应 |
|------------------|-------------------|--------------|
| 栅格自由空间 A* / Hybrid A* | **车道级路由图**（lane-level route） | Nav2 Route Server / Lanelet2 routing |
| map.json `lanes: []` 空数组 | **lanes[] 真序列化 + 可视化** | 高精地图车道拓扑 |
| 单一 planPath 自由空间 | **First/Last Mile 混合**（graph 中段 + freespace 两端） | Nav2「Navigate on Route Graph」 |
| Workbench 四页算法配置 | **Routing 独立第 5 页** + **Lane 编辑独立 Panel** | 地图/路由/算法分层 UI |

Phase 7 已完成「预测 → ST → 实验对比」闭环；Phase 8 补工业栈中 **路由层**：在结构化车道网络上规划，再与现有 Hybrid/MPC/ST 衔接。

### 1.2 验收清单（全部勾选 = Phase 8 完成）

- [ ] `LaneGraph`：从 `lanes[]` 建图；`shortestPath(from_node, to_node)` 返回节点序列；单测
- [ ] `LaneRouter`：lane 序列 → centerline `Path`；空图/不可达返回失败
- [ ] `SimEngine`：`routing_mode=lane_graph|hybrid` 时 planPath 走 LaneGraph；默认 `freespace` 与 Phase 7 行为一致
- [ ] First/Last Mile：单测或集成测证明 hybrid 路径与纯 A*、纯 lane 均不同（可区分）
- [ ] **LaneEditorPanel** 独立 dock；**RoutingPage** 为 Workbench 第 5 页；MapEditorPanel 未显著增行数
- [ ] MapView 显示 lane 折线/节点；与障碍物编辑模式分离
- [ ] `lane_routing_demo` 场景可加载、可规划、可仿真
- [ ] ADR-018、ADR-019；DEVELOPMENT_PLAN Phase 8 ✅；MUTATION M37+；`verify_phase8_evidence.py` PASS
- [ ] GTest 全绿（≥118 + 新增测）

### 1.3 明确不做（Phase 8 范围外）

| 项目 | 原因 |
|------|------|
| 完整 Lanelet2 / OSM 导入 | 依赖与格式爆炸；本阶段 JSON lanes[] MVP |
| 完整 Nav2 Route Server 移植 | 教学简化 |
| 完整 Behavior Tree 引擎 | Phase 9+；本阶段仅 routing_mode 枚举 |
| 神经网络 / 多模态预测 | Phase 7 已界定 |
| 激光/相机/占用栅格感知 | 另阶段 |
| 非线性 MPC / EECBS 最优 MAPF | 已有 MPC/Priority 教学 |

---

## §2 当前框架结构（必须保持）

```
UI (Qt) → App (SimController / ProjectManager / MonitorBridge)
        → Domain (SimEngine + planning/map/scheduling/collision/prediction/experiment)
        → Core (Pose / Path / SpeedProfile / EventBus / SimClock)

CMake:
FleetSimCore → FleetSimDomain → FleetSimApp → FleetSimUI → FleetSim
FleetSimTests → App + Domain + Core + GTest
```

**Phase 8 增量（示意）：**

```
domain/map/LaneGraph.*              （替换 stub）
domain/map/LaneTypes.h              （LaneNode, LaneEdge, LanePath）
domain/planning/LaneRouter.*        （graph → Path）
domain/planning/FirstLastMilePlanner.* （可选独立类或 SimEngine 私有）
扩展 MapSerializer / MapLoader       （lanes[]）
扩展 SimEngine                       （routing_mode_, planPath 分支）
ui/panels/LaneEditorPanel.*         （新建，独立 dock）
ui/dialogs/pages/RoutingPage.*      （Workbench 第 5 页）
ui/graphics/LaneGraphicsItem.*      （MapView 叠加）
assets/scenarios/lane_routing_demo/
```

**数据流（Hybrid Routing，示意）：**

```
Goal Pose
  → [First Mile] A*/Hybrid 自由空间 → 最近 LaneGraph 节点
  → [Lane Mile] LaneGraph.shortestPath → LaneRouter → centerline Path
  → [Last Mile] 自由空间 → Goal
  → Smoother → ST (optional) → Tracker (MPC/Stanley/PP)
```

---

## §3 优先阅读的程序（按优先级）

### P0 — 动手前必读

| 文件 | 为什么 |
|------|--------|
| `src/domain/map/LaneGraph.h` | **当前 stub**；Phase 8 核心替换对象 |
| `src/domain/map/MapSerializer.cpp` / `MapLoader.cpp` | `lanes[]` 序列化扩展点 |
| `src/domain/map/MapData.h` | 地图数据结构 |
| `src/domain/SimEngine.cpp`（`planPathForAgent` / `planPath`） | routing 注入点 |
| `src/domain/planning/AStarPlanner.*` / `HybridAStarPlanner.*` | First/Last mile 复用 |
| `src/domain/planning/DouglasPeuckerSmoother.*` | centerline 平滑 |
| `src/ui/panels/MapEditorPanel.*` | **勿在此堆 lane 编辑** |
| `src/ui/dialogs/AlgorithmWorkbenchDialog.*` + `pages/*` | 加 RoutingPage |
| `src/ui/map/MapScene.*` / `MapView.*` | Lane 图元叠加 |
| `tests/domain/MapSerializerTest.cpp` | 扩展 lane round-trip |

### P1 — UI 与场景

| 文件 | 为什么 |
|------|--------|
| `docs/UI_GUIDELINES.md` | Reviewer 硬规则 |
| `docs/decisions/017-algorithm-workbench-ui.md` | Workbench 分页先例 |
| `src/ui/panels/README.md` | 登记 LaneEditorPanel |
| `assets/scenarios/prediction_st_demo/` | Phase 7 场景模板 |
| `tools/verify_phase7_evidence.py` | 仿写 verify_phase8 |

### P2 — 防回归（继续生效）

| ID | 对策 |
|----|------|
| P7-Pred/ST | ConstantVelocity/StGraphWithPrediction/StGraphSimEngineWiring 仍绿 |
| P7-UI | Workbench 四页 + Compare 独立；PlannerTracker 薄封装 |
| P6-MPC/ST | M31–M33 仍有效 |
| P4-EXPORT | BUILD_INTERFACE 纪律 |
| P4-TEMP | test_tmp ASCII |

---

## §4 网络调研摘要（执行 Agent 必读）

### 4.1 Nav2 Route Server：路由图 + 自由空间混合

Nav2 在 **Planner Server（自由空间）** 之外增加 **Route Server（预定义导航路由图）**，典型架构（教学简化）：

1. **ComputeRoute**：在预定义 graph 上求 lane/route 序列（大规模场景可替代长距离 freespace 规划）。  
2. **First Mile / Last Mile**：若起终点不在 graph 上，用 `ComputePathToPose`（自由空间）连接到 route 首尾节点（Nav2 BT：`navigate_on_route_graph_w_recovery`）。  
3. **三种衔接方式**（Nav2 文档）：  
   - Route 稠密路径 → 直接跟踪；  
   - Route 稀疏 → Planner 规划到下一节点；  
   - Route 作先验 + 局部 freespace 绕障。

**FleetSim Phase 8 选择**：JSON `lanes[]` 轻量 graph + `routing_mode=hybrid` 实现 first/lane/last 三段；**不**移植 nav2_route 包。

参考：[Nav2 Route Server](https://docs.nav2.org/) · [Navigate on Route Graph BT](https://docs.nav2.org/behavior_trees/trees/navigate_on_route_graph_w_recovery.html)

### 4.2 Lanelet2：工业 lane routing 对照（边界内借鉴）

| Lanelet2 概念 | FleetSim Phase 8 MVP |
|---------------|---------------------|
| LaneletMap + RoutingGraph | `LaneGraph` + `lanes[]` JSON |
| shortestPath / Dijkstra | `LaneGraph::shortestPath` |
| Route vs Path vs Sequence | `LanePath`（节点 id 序列）→ 一条 centerline Path |
| TrafficRules / 多参与者 | 不做；单一 AGV/vehicle |
| OSM 导入 | 不做；编辑器手动/JSON |

参考：[Lanelet2 routing 文档](https://fzi-forschungszentrum-informatik.github.io/Lanelet2/lanelet2_routing/)

### 4.3 车道级路由论文要点

车道级路由可在 lane 拓扑上求 **最小期望代价** 路径（含变道决策）；经典做法包括图搜索 / MDP。FleetSim 教学版：**确定型有向图 + 边权（长度）+ Dijkstra** 足够。

参考：[Lane-Level Route Planning (arXiv:2206.02883)](https://arxiv.org/html/2206.02883)

### 4.4 UI 分层启示（防 Goal Agent 堆控件）

| 层次 | 用户心智 | FleetSim 落点 | 禁止 |
|------|---------|--------------|------|
| 环境几何 | 障碍物、边界 | MapEditorPanel | 与 lane 混放 |
| 拓扑车道 | 节点、边、方向 | **LaneEditorPanel** | 塞进 MapEditor |
| 算法实验 | planner/tracker/ST/prediction | Workbench 既有四页 | 与 lane 混放 |
| 路由策略 | graph/freespace/hybrid | **RoutingPage（第 5 页）** | 塞进 SpeedPage |
| 运行监控 | 曲线、误差 | MonitorPanel | 加 lane 表单 |
| 实验对比 | 聚合指标 | ExperimentComparePanel | 加路由控件 |

---

## §5 设计决策建议（先写 ADR 再写码）

### ADR-018 — LaneGraph + Hybrid Routing

1. **map.json lanes[] schema（写死 MVP）**：

```json
"lanes": {
  "nodes": [
    { "id": "n0", "x": 1.0, "y": 2.0 },
    { "id": "n1", "x": 5.0, "y": 2.0 }
  ],
  "edges": [
    { "from": "n0", "to": "n1", "bidirectional": false }
  ]
}
```

2. **LaneGraph API**：

```cpp
class LaneGraph {
public:
    bool loadFromMap(const LaneMapData& lanes);
    std::optional<std::vector<std::string>> shortestPath(
        const std::string& from_id, const std::string& to_id) const;
    std::string nearestNodeId(double x, double y) const;
    core::Path centerlinePath(const std::vector<std::string>& node_ids) const;
};
```

3. **SimEngine**：

| `routing_mode` | 行为 |
|----------------|------|
| `freespace`（默认） | Phase 7 行为：A*/Hybrid 栅格规划 |
| `lane_graph` | 起终点 snap 到节点 → graph 路径 → centerline |
| `hybrid` | First mile (freespace→entry node) + lane mile + last mile |

4. **禁止**：`LaneGraph::loadFromFile` 永远 `return true` 且 `nodeCount()==0`。

### ADR-019 — Lane 编辑 UI 信息架构

1. **LaneEditorPanel**（独立 dock）：  
   - 子职责拆分：`LaneNodeListWidget` / `LaneEdgeInspectorWidget`（可选分文件）  
   - 工具：添加节点、连接边、删除、选中同步 MapView  
2. **RoutingPage**（Workbench Tab 5）：仅 `routing_mode`、snap 半径、first/last planner 选择。  
3. **MapEditorPanel**：保持障碍物/undo；**禁止**新增 lane 按钮列。  
4. **行数红线**：单 Panel/Page `.cpp` ≤300 行；超出必须拆 SubWidget。

---

## §6 建议新增/变更文件清单

### Domain / Map

| 文件 | 职责 |
|------|------|
| `map/LaneTypes.h` | LaneNode, LaneEdge, LaneMapData |
| `map/LaneGraph.h/.cpp` | 建图、Dijkstra、nearest、centerline |
| `planning/LaneRouter.h/.cpp` | graph → Path 管线 |
| 扩展 `MapSerializer.*` / `MapData.h` | lanes[] |
| 扩展 `SimEngine.*` | routing_mode / hybrid plan |

### UI

| 文件 | 职责 |
|------|------|
| `ui/panels/LaneEditorPanel.*` | Lane CRUD dock |
| `ui/dialogs/pages/RoutingPage.*` | Workbench 第 5 页 |
| `ui/graphics/LaneGraphicsItem.*` | 可视化 |
| 扩展 `MapScene.*` | 叠加 lane 层 |

### 测试 / 文档 / 工具

```
tests/domain/LaneGraphTest.cpp
tests/domain/LaneRouterTest.cpp
tests/domain/MapSerializerLaneTest.cpp
tests/integration/FirstLastMileIntegrationTest.cpp
docs/decisions/018-lane-graph-routing.md
docs/decisions/019-lane-editor-ui.md
tools/verify_phase8_evidence.py
tools/run_phase8_verify.ps1
assets/scenarios/lane_routing_demo/
```

---

## §7 如何继续开发（Goal 节奏 + 四角色）

1. `git pull`；确认 Phase 7：`pwsh -File tools/run_phase7_verify.ps1`；FleetSimTests 118 绿  
2. 会话 0：ADR + lanes schema + 红灯测  
3. 会话 1–3：LaneGraph → Router → SimEngine hybrid  
4. 会话 4–5：UI 分层（LaneEditor + RoutingPage + 图元）  
5. 会话 6–7：场景 + 回归 + MUTATION + 终审  

### 四角色监督协议

| 角色 | 输入 | 输出 | 否决权 |
|------|------|------|--------|
| **Planner** | Phase8 目标 + 本会话 scope | mini-plan + UI 分页清单 | 范围蔓延 |
| **Executor** | mini-plan | 代码 + CMake | — |
| **Tester** | 代码 + 测例 | 红绿 + 假 graph/UI 堆叠判定 | 无测/假 LaneGraph/UI 堆叠 → FAIL |
| **Reviewer** | diff + §10 + UI_GUIDELINES | PASS/FAIL | FAIL 阻完成 |

**UI 专项 Tester 检查**：

- `MapEditorPanel.cpp` 行数较 Phase 7 基线 **显著增加** 且未新建 `LaneEditorPanel` → **FAIL**  
- `SpeedPage.cpp` / `PlanningPage.cpp` 出现 `routing_mode` 或 lane 列表 → **FAIL**  
- Workbench 无第 5 页 `RoutingPage` 却标 Phase 8 UI 完成 → **FAIL**

---

## §8 Phase 2–7 防回归（继续生效）

| ID | 对策 |
|----|------|
| P7-Pred/ST | prediction/CV/StGraph wiring 测仍绿 |
| P7-UI | Workbench 四页 + Compare；Routing 仅第 5 页 |
| P6-MPC/ST | Mpc*/StGraph* 仍绿 |
| P5-Hybrid/Priority | 回归测绿 |
| P8-Lane | 新功能必须在 LaneEditorPanel / RoutingPage / LaneGraph |

---

## §9 关键参数默认值

| 参数 | 默认 | 说明 |
|------|------|------|
| simulation.routing_mode | freespace | 显式 hybrid 才启用 first/last mile |
| lane snap 半径 | 1.0 m | 最近节点查询 |
| 边权 | 欧氏距离 | 教学 |
| hybrid first/last planner | 与 planner_kind 一致 | auto→Hybrid/A* |

---

## §10 禁止偷懒清单（Reviewer 打印打勾）

1. 禁止 LaneGraph 空 stub 冒充完成  
2. 禁止 routing_mode 不读  
3. 禁止 Lane 编辑堆 MapEditorPanel  
4. 禁止 Routing 配置堆 Speed/Planning/Compare/Monitor  
5. 禁止同一 Panel 混合「地图几何 + 车道拓扑 + 算法 + 路由 + 对比指标」  
6. 禁止 Domain 含 Qt / rclcpp  
7. 禁止新 Domain 类无 GTest  
8. 禁止削 CMake `target_*`  
9. 禁止破坏 Phase7 prediction/ST/Workbench/Compare  
10. 禁止完整 Lanelet2/Autoware/nav2/BT/感知  
11. 禁止 SESSION_LOG 缺四角色 / 缺「没做什么」  
12. 禁止未登记 CMake 就加源  
13. 禁止跳过四角色  
14. 禁止 Phase8 ✅ 但无 LaneEditorPanel + RoutingPage  
15. 禁止 first/last mile 与纯 freespace 路径完全相同却无单测区分  

---

## §11 测试策略（最低标准）

| 层级 | 要求 |
|------|------|
| 单元 LaneGraph | 建图、最短路径、不可达、nearest |
| 单元 LaneRouter | centerline 点数 ≥ 2；空 graph 失败 |
| 单元 MapSerializer | lanes[] round-trip |
| 集成 FirstLastMile | hybrid vs freespace vs lane_graph 路径可区分 |
| 回归 | Phase 7 全部 Predictor/Experiment/StGraph/MPC + Phase 5 Priority/Hybrid |
| 突变 | M37 删 edge；M38 nearest 恒返回固定节点；M39 hybrid 不跑 lane 段 |

---

## §12 用户本地验证清单（每次交付必须附上）

1. `git pull origin main`  
2. 可选：`pwsh -File tools/run_phase8_verify.ps1`  
3. Qt Creator Configure → Build；或 ASCII：`D:\build\FleetSim_phase8`  
4. 运行 `FleetSimTests`（关注 Lane* / FirstLast* / MapSerializer* / Phase7*）  
5. Open `assets/scenarios/lane_routing_demo`  
6. **View → Lane Editor** 查看/编辑节点边；MapView 显示 lane 线  
7. **Algorithm Workbench → Routing 页** 切 `hybrid` → 设 goal → plan  
8. 确认 Monitor/Experiment Compare 仍各司其职；Phase 7 prediction demo 仍可用  

---

## §13 风险与缓解

| 风险 | 缓解 |
|------|------|
| Agent 把 lane 编辑堆进 MapEditor | ADR-019 + Reviewer 行数 diff |
| LaneGraph 假实现 | 单测 shortestPath + hybrid 路径区分 |
| hybrid 破坏 Phase 7 默认 | routing_mode 默认 freespace |
| UI 文件超 300 行 | 强制拆 SubWidget |

---

## §14 参考链接

1. [Nav2 Route Server / Route Graph](https://docs.nav2.org/)  
2. [Navigate on Route Graph with Recovery (Nav2 BT)](https://docs.nav2.org/behavior_trees/trees/navigate_on_route_graph_w_recovery.html)  
3. [Lanelet2 Routing](https://fzi-forschungszentrum-informatik.github.io/Lanelet2/lanelet2_routing/)  
4. [Lane-Level Route Planning (arXiv:2206.02883)](https://arxiv.org/html/2206.02883)  
5. Phase 7：`docs/PHASE7_GOAL_PROMPT.md`、`docs/decisions/016-*.md`、`017-*.md`

---

## §15 新对话开场白（短版）

> 执行 `docs/PHASE8_GOAL_PROMPT.md`。Goal：完成 Phase 8（LaneGraph 真实现 + hybrid first/last mile + LaneEditorPanel + Workbench RoutingPage + lane_routing_demo）。强制四角色子 Agent；GTest + verify_phase8 全绿；每会话 push。从会话 0 开始。

---

## §16 Planner mini-plan 模板（每会话必填）

```markdown
**Phase**: 8
**本会话目标**: （一句话）
**允许改动**: （目录/文件）
**NOT DO**: （≥5 条，含禁止堆 UI / 假 LaneGraph / 破坏 Phase7）
**预计新增/修改文件**:
**计划测试**（精确 TEST 名）:
**UI 分页验收**（必须列出 LaneEditorPanel / RoutingPage / 禁止改动的 Panel）:
**验收标准**:
```

---

## §17 Phase 9 展望（勿在 Phase 8 实现）

- 轻量 **Behavior Tree 教学 MVP**（yield / replan / recovery 节点）  
- **ExperimentMetrics CSV 导出**  
- **Lanelet2/OSM 导入**（可选插件）  
- 多模态预测 / 感知 — 明确另阶段  
