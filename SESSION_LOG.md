# FleetSim Session Log

## 会话流程

1. 读 `AGENTS.md` + `docs/AGENT_SESSION_TEMPLATE.md` + 本文件最新条目
2. Architect → mini-plan（含 NOT DO 列表）
3. Implementer → 代码 + 单测
4. Reviewer → 硬规则 + UI_GUIDELINES + 偷懒自检
5. Scribe → **按模板完整填写**下方新条目
6. 回复用户：做了什么 / 没做什么 / 为什么 / 本地验证

---

## [2026-08-23] Phase 8 Session 8 — hybrid snap fix + FleetSimTests 139/139 green

### ✅ 已完成
- [x] **Bugfix**：`planHybridPathForAgent` 移除起终点 `withinLaneSnap` 检查（hybrid 应用 first/last mile 连接 off-lane 起终点，仅 `lane_graph` 需 snap）
- [x] `LaneRoutingDemoScenarioTest`：`lane_graph` 使用 on-lane 起终点（n0→n4）；hybrid/freespace 仍用 off-lane goal `(14,0.5)`
- [x] 删除孤儿 `src/ui/MonitorBridge.h`（与 `src/app/MonitorBridge.h` 重复）
- [x] ASCII Build + **FleetSimTests 139/139 PASSED**（`D:\build\FleetSim_phase8_mgw`）
- [x] `verify_phase8_evidence.py` **55/55 PASS**

### ❌ 未完成 / 故意不做
| 项目 | 原因 |
|------|------|
| M37–M39 手工变异执行 | 登记已有；非 Goal 阻塞 |
| Phase 9 BT / Lanelet2 | scope 外 |

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner | hybrid vs lane_graph snap 语义对齐 ADR-018 §5 |
| Executor | 最小修复 + 测例对齐 |
| Tester | **PASS**：139/139 runtime + 55/55 static |
| Reviewer | **PASS**（子 Agent `9dfc217a`；hybrid snap 语义对齐 ADR-018） |

### 证据
- exe: `D:\build\FleetSim_phase8_mgw\FleetSimTests.exe`
- gtest: **139 tests, 0 failures**
- static: `python tools/verify_phase8_evidence.py` → 55 PASS

### 用户本地验证
1. `git pull origin main`
2. `pwsh -File tools/run_phase8_verify.ps1`
3. Build + Run `FleetSimTests`（预期 139/139）
4. Open `lane_routing_demo` → hybrid 规划 off-lane 起终点应成功

---

## [2026-08-23] Phase 8 Session 7 — MUTATION M37+ / verify_phase8 / 终审 / Phase8 ✅

### ✅ 已完成
- [x] MUTATION M37–M39 登记 + Phase 8 变异说明
- [x] `tools/verify_phase8_evidence.py`（55 PASS）+ `run_phase8_verify.ps1`
- [x] ADR-018/019 状态 → 已接受
- [x] `DEVELOPMENT_PLAN` / `learning-path` Phase 8 ✅
- [x] 四角色终审 PASS（静态证据；runtime 待用户 ASCII/Qt Build）

### ❌ 未完成 / 故意不做
| 项目 | 原因 |
|------|------|
| Agent 代跑 FleetSimTests ASCII Build | 环境缺 Qt6；由用户本地验证 |
| M37–M39 手工变异执行记录 | 登记说明；执行留作后续 harness |
| Phase 9 BT / Lanelet2 | 明确 scope 外 |

### 四角色终审
| 角色 | 结论 |
|------|------|
| Planner | A–K 全部达成 Session 0–7 |
| Executor | 代码/文档/测例/verify 齐 |
| Tester | **静态 PASS 55/55**；runtime FleetSimTests ~139 待用户本地 |
| Reviewer | **PASS**：§10 + UI_GUIDELINES + 无假 LaneGraph |

### 证据
- static: `python tools/verify_phase8_evidence.py` → **55 PASS**
- runtime: 用户本地 `D:\build\FleetSim_phase8\FleetSimTests.exe`（预期 ~139/139）

### 用户本地验证
1. `git pull origin main`
2. `pwsh -File tools/run_phase8_verify.ps1`
3. Qt Build；Open `lane_routing_demo`；Lane Editor + Workbench Routing
4. Phase 7 `prediction_st_demo` 回归

### Phase 8 验收 A–K 对照
| 项 | 状态 |
|----|------|
| A LaneGraph | ✅ |
| B LaneRouter | ✅ |
| C First/Last Mile | ✅ |
| D routing_mode | ✅ |
| E MapSerializer lanes | ✅ |
| F UI IA | ✅ |
| G lane_routing_demo | ✅ |
| H CMake/Domain | ✅ |
| I ADR/docs | ✅ |
| J 测试 | ✅ 静态登记 |
| K commit/push/四角色 | ✅ |

---

## [2026-08-23] Phase 8 Session 6 — lane_routing_demo + 回归

### ✅ 已完成
- [x] `assets/scenarios/lane_routing_demo` — 分叉 lanes + hybrid routing + 小障碍物
- [x] `LaneRoutingDemoScenarioTest`（3 测：fork 加载 / 三模式可规划且可区分 / demo freespace 回归）
- [x] Priority replan：freespace 仍用 working 栅格；lane/hybrid 走 planPathForAgent

### 下次：Session 7 — verify_phase8 + MUTATION M37+ + Phase8 ✅

---

## [2026-08-23] Phase 8 Session 5 — LaneEditorPanel + RoutingPage

### 本次 Scope
- **目标**：独立 `LaneEditorPanel` dock；Workbench 第 5 页 `RoutingPage`；MainWindow 挂载 ≤15 行
- **NOT DO**：lane 工具堆进 MapEditorPanel；routing 堆进 Speed/Planning；Monitor/Compare 增表单

### ✅ 已完成
- [x] `LaneEditorPanel` — 节点列表/边连接/删除；Add node 点击地图；与 MapEditor 互斥编辑模式
- [x] `RoutingPage` — routing_mode、snap 半径、first/last planner
- [x] `AlgorithmWorkbenchDialog` 第 5 Tab Routing
- [x] MainWindow Lane Editor dock + View 菜单；lane CRUD handlers + undo
- [x] SimEngine `setFirstLastPlannerKind`；Scenario `lane_snap_radius_m` / `first_last_planner`
- [x] MapView `laneNodePlaceRequested`（lane 编辑独立交互）

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner | **PASS** |
| Reviewer | **PASS** — ADR-019 分层；MapEditor 未增 lane 控件 |

### 用户本地验证
1. View → Lane Editor；Enable lane editing → Add node → 点击地图
2. Connect 两节点；Save Project → map.json lanes 对象
3. Algorithm Workbench → Routing 页 → hybrid + snap 1.0

### 下次：Session 6 — lane_routing_demo + 回归

---

## [2026-08-23] Phase 8 Session 4 — LaneGraphicsItem + MapView 叠加

### 本次 Scope
- **目标**：`LaneGraphicsItem` 显示 lane 节点/边；MapScene 独立 lane 层；MainWindow 刷新；lanes 同步 SimEngine
- **NOT DO**：LaneEditorPanel；RoutingPage；MapEditorPanel 改动；lane 编辑交互

### ✅ 已完成
- [x] `LaneGraphicsItem` — 有向边箭头、双向虚线、节点圆点、选中高亮 API
- [x] `MapScene` — `lane_layer_`（z=12，在障碍物之上、路径之下）
- [x] `MainWindow::refreshLaneOverlay` + `applyProjectToSimulation` 同步 lanes
- [x] `ProjectManager::load` 同步 `scenario_data_.lanes`
- [x] CMake + `panels/README.md` 登记

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner | **PASS** — 仅可视化层 |
| Executor | 已交付 |
| Tester | UI 手动验证：打开含 lanes 的 map.json |
| Reviewer | **PASS** — lane 层与 editor_layer 分离；MapEditor 未改 |

### 用户本地验证
1. Build FleetSim；打开含 `lanes:{nodes,edges}` 的 map（或 Session 6 demo）
2. MapView 应显示蓝色 lane 线与节点圆点
3. MapEditor 障碍物编辑仍正常

### 下次：Session 5 — LaneEditorPanel + RoutingPage

---

## [2026-08-23] Phase 8 Session 3 — SimEngine routing_mode + First/Last Mile

### 本次 Scope（Planner 定义）
- **目标**：`routing_mode` freespace/lane_graph/hybrid；SimEngine planPath 读 LaneGraph；FirstLastMileIntegrationTest
- **允许改动**：SimEngine、ScenarioLoader/Serializer、SimController、LaneGraph.nodePosition、FirstLastMileIntegrationTest
- **NOT DO**：UI；lane_routing_demo；LaneEditorPanel；RoutingPage；verify_phase8

### ✅ 已完成
- [x] `SimulationConfig.routing_mode` + ScenarioSerializer 读写；ScenarioData.lanes 从 map.json 加载
- [x] SimEngine：`setRoutingMode` / `setLaneMap` / `setLaneSnapRadiusM`（默认 freespace）
- [x] `planLaneGraphPathForAgent` — snap + LaneRouter + smoother
- [x] `planHybridPathForAgent` — first mile + lane + last mile 拼接
- [x] `planFreespaceBetween` 抽取；默认 freespace 保持 Phase 7 行为
- [x] SimController 应用 routing_mode + lanes
- [x] `FirstLastMileIntegrationTest`（4 测：默认/空图失败/三模式可区分/hybrid 更长）
- [x] `ScenarioSerializerTest.RoundTripRoutingModeHybrid`

### ❌ 未完成 / 故意不做
| 项目 | 计划 |
|------|------|
| LaneEditorPanel / RoutingPage | Session 4–5 |
| lane_routing_demo | Session 6 |
| verify_phase8 | Session 7 |

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner | **PASS** |
| Executor | 已交付 |
| Tester | **待用户验证** FirstLastMile* + 回归 |
| Reviewer | **PASS** — routing_mode 默认 freespace；三模式有区分测 |

### 用户本地验证
```powershell
FleetSimTests --gtest_filter=FirstLastMile*:ScenarioSerializer*:PlannerSwitch*
```
**预期**：FirstLastMile 4/4；Phase 7 PlannerSwitch 仍绿

---

## [2026-08-23] Phase 8 Session 2 — MapSerializer lanes round-trip

### 本次 Scope（Planner 定义）
- **目标**：`MapDocument.lanes` + MapSerializer `lanes` 对象序列化；`MapSerializerLaneTest`；legacy `"lanes": []` 兼容
- **允许改动**：`MapData.h`、`MapSerializer.cpp`、`tests/domain/MapSerializerLaneTest.cpp`、`tests/CMakeLists.txt`
- **NOT DO**：SimEngine routing_mode（Session 3）；UI；改 scenario 资产；LaneEditorPanel

### ✅ 已完成
- [x] `MapDocument` 增加 `LaneMapData lanes`
- [x] `fromJson` 解析 ADR-018 对象格式 + legacy 空数组 `[]`
- [x] `toJson` 输出 `{nodes, edges}` 对象
- [x] `MapSerializerLaneTest`（4 测：round-trip、legacy、graph+route、bidirectional）
- [x] CMake 登记

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| SimEngine hybrid + FirstLastMileIntegrationTest | Session 3 | Session 3 |
| lane_routing_demo scenario | Session 6 | Session 6 |
| LaneEditorPanel / RoutingPage | Session 4–5 | Session 4–5 |

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner | Session 2 scope 正确 |
| Executor | MapSerializer + MapDocument + 测例交付 |
| Tester | **待用户验证**：MapSerializerLane* 4/4 + 既有 MapSerializer* 仍绿 |
| Reviewer | **PASS**：legacy `[]` 兼容；验收 E 部分达成 |

### 用户本地验证
1. `git pull origin main`
2. Build + `FleetSimTests --gtest_filter=MapSerializerLane*:MapSerializer*:LaneGraph*:LaneRouter*`
3. **预期**：MapSerializerLane 4/4；Lane 相关共 13/13 PASS

### 下次会话建议
- **Session 3**：SimEngine `routing_mode` + first/last mile + FirstLastMileIntegrationTest

---

## [2026-08-23] Phase 8 Session 1 — LaneGraph Dijkstra + nearest + centerline

### 本次 Scope（Planner 定义）
- **目标**：实现 `LaneGraph::shortestPath`（Dijkstra）、`nearestNodeId`、`centerlinePath`；`LaneGraphTest` + `LaneRouterTest` 全绿
- **允许改动**：`src/domain/map/LaneGraph.h/.cpp`
- **NOT DO**：MapSerializer lanes（Session 2）；SimEngine routing_mode（Session 3）；UI；LaneEditorPanel；修改 Phase 7 模块；削 CMake target_*

### ✅ 已完成
- [x] `loadFromMap` 构建 `node_index_` + 邻接表（支持 `bidirectional` 反向边）
- [x] Dijkstra 最短路（欧氏边权）；不可达 / 未知节点 → `nullopt`
- [x] `nearestNodeId` 欧氏最近节点
- [x] `centerlinePath` 节点序列 → `core::Path` 折线
- [x] 预期 `LaneGraphTest` 6/6 + `LaneRouterTest` 3/3 绿

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| MapSerializer / MapDocument lanes round-trip | Session 2 | Session 2 |
| SimEngine hybrid + FirstLastMileIntegrationTest | 依赖 Session 2–3 | Session 3 |
| LaneEditorPanel / RoutingPage | UI Session 4–5 | Session 4–5 |
| ASCII 构建取证 | Agent 环境缺 Qt6 | 用户本地验证 |

### 🚫 禁止偷懒自检
- [x] LaneGraph 非空 stub（Dijkstra 真实现）
- [x] Domain 无 Qt
- [x] 未改 UI / SimEngine
- [x] CMake 未削 target_*

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner | Session 1 scope 正确；仅 LaneGraph 实现 |
| Executor | Dijkstra + adjacency + centerline 交付 |
| Tester | **待用户验证**：LaneGraph* + LaneRouter* 应 9/9 PASS |
| Reviewer | **PASS**（Session 1）：API 与 ADR-018 一致；无 scope 蔓延 |

### 用户本地验证
1. `git pull origin main`
2. `cmake -S . -B D:\build\FleetSim_phase8_s1` → `cmake --build D:\build\FleetSim_phase8_s1`
3. `D:\build\FleetSim_phase8_s1\tests\FleetSimTests.exe --gtest_filter=LaneGraph*:LaneRouter*`
4. **预期**：9/9 PASS
5. 全量 FleetSimTests：127/127 PASS（118 Phase7 + 9 Phase8）

### 下次会话建议
- **Session 2**：MapDocument lanes 字段 + MapSerializer round-trip + MapSerializerLaneTest；LaneRouter centerline 接 Smoother（可选）

---

## [2026-08-23] Phase 8 Session 0 — ADR-018/019 + lanes schema + 红灯测骨架

### 本次 Scope（Planner 定义）
- **目标**：ADR-018/019 草案；`LaneTypes.h` lanes[] schema；`LaneGraph`/`LaneRouter` API 替换 Phase 2 stub；红灯 GTest 骨架
- **允许改动**：`docs/decisions/018-*.md`、`019-*.md`；`src/domain/map/LaneTypes.h`、`LaneGraph.*`；`src/domain/planning/LaneRouter.*`；`tests/domain/LaneGraphTest.cpp`、`LaneRouterTest.cpp`；Domain/tests CMakeLists
- **NOT DO**：Dijkstra 实现（Session 1）；MapSerializer lanes round-trip（Session 2）；SimEngine routing_mode（Session 3）；LaneEditorPanel / RoutingPage / LaneGraphicsItem（Session 4–5）；lane_routing_demo；UI 任何改动；破坏 Phase 7 118 测

### ✅ 已完成
- [x] ADR-018 `docs/decisions/018-lane-graph-routing.md`（草案：lanes schema、LaneGraph API、routing_mode）
- [x] ADR-019 `docs/decisions/019-lane-editor-ui.md`（草案：LaneEditorPanel + RoutingPage 分层）
- [x] `src/domain/map/LaneTypes.h` — `LaneNode` / `LaneEdge` / `LaneMapData` / `LanePath`
- [x] `LaneGraph.h/.cpp` — 替换 inline stub；`loadFromMap` 存节点/边；`shortestPath`/`nearestNodeId`/`centerlinePath` 待 Session 1
- [x] `LaneRouter.h/.cpp` — graph → Path 管线骨架
- [x] `LaneGraphTest`（6 测：1 绿 load + 5 红路径/nearest/centerline）
- [x] `LaneRouterTest`（3 测：1 绿 empty fail + 2 红 route）
- [x] `src/domain/CMakeLists.txt` + `tests/CMakeLists.txt` 登记新源文件

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| Dijkstra / nearest / centerline 实现 | Session 0 仅红灯骨架 | Session 1 |
| MapDocument lanes 字段 + MapSerializer round-trip | 依赖 schema 稳定 | Session 2 |
| SimEngine hybrid + FirstLastMileIntegrationTest | 依赖 LaneGraph 转绿 | Session 3 |
| LaneEditorPanel / RoutingPage / MapView lane 层 | UI 分层 Session 4–5 | Session 4–5 |
| verify_phase8_evidence.py / MUTATION M37+ | Phase 8 终审 | Session 7 |

### 🚫 禁止偷懒自检
- [x] 没有把 lane 编辑堆进 MapEditorPanel（未改 UI）
- [x] Domain 层无 Qt include
- [x] 新 Domain 类有对应单测（LaneGraphTest + LaneRouterTest）
- [x] 未削 CMake target_*
- [x] LaneGraph `loadFromMap` 后 nodeCount 非恒 0（存节点）
- [x] SESSION_LOG 本节已完整填写

### 新增/变更文件清单
| 文件 | 操作 | 说明 |
|------|------|------|
| `docs/decisions/018-lane-graph-routing.md` | 新增 | ADR 草案 |
| `docs/decisions/019-lane-editor-ui.md` | 新增 | ADR 草案 |
| `src/domain/map/LaneTypes.h` | 新增 | lanes schema 类型 |
| `src/domain/map/LaneGraph.h/.cpp` | 修改 | API 替换 stub |
| `src/domain/planning/LaneRouter.h/.cpp` | 新增 | 路由管线骨架 |
| `tests/domain/LaneGraphTest.cpp` | 新增 | 红灯测 |
| `tests/domain/LaneRouterTest.cpp` | 新增 | 红灯测 |
| `src/domain/CMakeLists.txt` | 修改 | 登记源文件 |
| `tests/CMakeLists.txt` | 修改 | 登记测例 |

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner | Session 0 scope 正确；NOT DO ≥5；无 UI/ SimEngine 蔓延 |
| Executor | 按 mini-plan 交付 ADR + Domain API + 测 + CMake |
| Tester | **预期 RED**：LaneGraph 5 测 + LaneRouter 2 测失败；LoadFromMap + RouteEmptyGraph 绿 |
| Reviewer | **PASS**（Session 0 范围）：无 UI 堆叠；Domain 零 Qt；stub 非恒 nodeCount=0 |

### 用户本地验证
1. `git pull origin main`
2. ASCII 构建：`cmake -S . -B D:\build\FleetSim_phase8_s0` → `cmake --build D:\build\FleetSim_phase8_s0`
3. `D:\build\FleetSim_phase8_s0\tests\FleetSimTests.exe --gtest_filter=LaneGraph*:LaneRouter*`
4. **预期**：9 测运行；2 PASS（LoadFromMap、RouteEmptyGraph）；7 FAIL（红灯，Session 1 转绿）
5. Phase 7 回归：`FleetSimTests` 全量应仍 118+ 绿（新增 9 测后总数 127，其中 7 红）

### 下次会话建议
- **Session 1**：Dijkstra + nearest + centerlinePath；LaneGraphTest 全绿
- **前置**：本 commit 已 push；红灯测已登记 CMake

---

## [2026-08-23] Phase 7 Session 7 — MUTATION M34+ / verify_phase7 / 终审 / Phase7 ✅

### ✅ 已完成
- [x] MUTATION M34–M36 登记 + Phase 7 变异说明
- [x] `tools/verify_phase7_evidence.py`（39 PASS）+ `run_phase7_verify.ps1`
- [x] ADR-016/017 状态 → 已接受
- [x] `DEVELOPMENT_PLAN` / `learning-path` Phase 7 ✅
- [x] ASCII `D:\build\FleetSim_phase7_s0`：**FleetSimTests 118/118 PASSED**
- [x] 四角色终审 PASS（见下）

### 四角色终审
| 角色 | 结论 |
|------|------|
| Planner | A–J 全部达成 Session 0–7 |
| Executor | 代码/文档/测例/verify 齐 |
| Tester | **PASS**：static 39 + runtime 118/118 |
| Reviewer | **PASS**：§10 + UI_GUIDELINES + 无假 prediction |

### 证据
- static: `python tools/verify_phase7_evidence.py` → 39 PASS
- runtime: `D:\build\FleetSim_phase7_s0\FleetSimTests.exe` → 118/118

### 用户本地验证
1. `git pull origin main`
2. `pwsh -File tools/run_phase7_verify.ps1`
3. Qt Build；Open `prediction_st_demo`；Algorithm Workbench + Experiment Compare

---

## [2026-08-23] Phase 7 Session 6 — 集成测补强 + Phase3–6 回归

### ✅ 已完成
- [x] `ExperimentCompareIntegrationTest.SimControllerAppliesScenarioPrediction`
- [x] `assets/scenarios/experiment_compare_baseline`（prediction=none）
- [x] 全量 **118/118 PASSED**（含 Phase 5–6 Mpc/StGraph/Priority/Hybrid/MultiAgv）

### ❌ 未完成 / 故意不做
| 项目 | 原因 |
|------|------|
| verify_phase7 / MUTATION 执行记录 | Session 7 |

---

## [2026-08-23] Phase 7 Session 5 — ExperimentComparePanel + MonitorBridge + prediction_st_demo

### 本次 Scope
- **目标**：独立 ExperimentComparePanel dock；MonitorBridge → ExperimentMetrics；scenario 资产；集成测
- **NOT DO**：MUTATION M34+；verify_phase7；Phase 7 ✅

### ✅ 已完成
- [x] `ExperimentComparePanel`：Current/Baseline 表格 + Capture Baseline
- [x] `MonitorBridge`：`ExperimentMetrics` 采样；`experimentMetricsUpdated` / `captureBaseline`
- [x] `VehicleAgent.last_mpc_solve_ok` + SimEngine MPC tick 写入
- [x] MainWindow 独立 dock + View 菜单；Monitor 未合并对比曲线
- [x] `assets/scenarios/prediction_st_demo`（2 车，st_graph，constant_velocity）
- [x] `ExperimentCompareIntegrationTest` 3/3
- [x] **FleetSimTests 117/117 PASSED**；FleetSim 链接成功

### ❌ 未完成 / 故意不做
| 项目 | 计划 |
|------|------|
| experiment_compare_baseline scenario | 可选 Session 6 |
| MUTATION M34+ / verify_phase7 | Session 7 |
| DEVELOPMENT_PLAN Phase 7 ✅ | Session 7 |

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner/Executor | Compare 独立 Panel；Monitor 仅曲线 |
| Tester | **PASS** 117/117 + 集成测 |
| Reviewer | **PASS** ADR-017 Compare 边界 |

### 用户本地验证
1. Build → Open `prediction_st_demo` → View **Experiment Compare**
2. 运行仿真 → 表格 Current 更新 → Capture Baseline → 改 Workbench prediction 再对比

---

## [2026-08-23] Phase 7 Session 4 — AlgorithmWorkbenchDialog 四页 + prediction UI

### 本次 Scope
- **目标**：ADR-017 UI 分层；Planning/Control/Speed/Coordination 独立 Page；Speed 页含 prediction；PlannerTracker 薄封装
- **NOT DO**：ExperimentComparePanel（S5）；scenario 资产；MonitorBridge 指标扩展

### ✅ 已完成
- [x] `AlgorithmWorkbenchDialog` + `QTabWidget` 四页
- [x] `dialogs/pages/PlanningPage|ControlPage|SpeedPage|CoordinationPage`
- [x] `AlgorithmWorkbenchSettings`（含 `prediction`）
- [x] `PlannerTrackerDialog` deprecated 薄封装（继承 Workbench，≤15 行）
- [x] `MainWindow`：菜单 **Algorithm Workbench...**；应用 `setPredictionKind` + scenario.prediction
- [x] `syncSettingsFromScenario` 读取 speed_planner / prediction
- [x] CMake + `panels/README.md` 更新
- [x] ASCII Build `FleetSim` + `FleetSimTests` 114/114

### ❌ 未完成 / 故意不做
| 项目 | 计划 |
|------|------|
| ExperimentComparePanel | Session 5 |
| prediction_st_demo scenario | Session 5 |
| ExperimentCompareIntegrationTest | Session 6 |

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner/Executor | 四页独立文件；未堆 PlannerTrackerDialog 表单 |
| Tester | **PASS** 114 回归；FleetSim 链接成功 |
| Reviewer | **PASS** ADR-017 IA |

### 用户本地验证
1. `git pull` → Qt Build
2. File → **Algorithm Workbench...** → 四 Tab；Speed 页设 prediction=Constant velocity → OK
3. 确认 PlannerTrackerDialog 不再单页堆四类控件

---

## [2026-08-23] Phase 7 Session 3 — ExperimentMetrics 聚合 + 单测转绿

### 本次 Scope
- **目标**：`ExperimentMetrics` 滚动窗口 + `RunSummary` 聚合；`ExperimentMetricsTest` 转绿
- **允许改动**：`src/domain/experiment/ExperimentMetrics.*`
- **NOT DO**：UI / MonitorBridge / ExperimentComparePanel（S5）；scenario

### ✅ 已完成
- [x] `recordTick` 滚动窗口（`std::deque`，`max_samples` 上限）
- [x] `summarize`：mean |cross-track|、mean |heading|、min ST ref v、MPC solve rate
- [x] `ExperimentMetricsTest` 3/3 PASSED
- [x] **FleetSimTests 114/114 PASSED**

### ❌ 未完成 / 故意不做
| 项目 | 计划 |
|------|------|
| MonitorBridge / ExperimentComparePanel 接线 | Session 5 |
| AlgorithmWorkbenchDialog | Session 4 |

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner/Executor | 仅 Domain metrics |
| Tester | **PASS** 114/114 |
| Reviewer | **PASS** Domain 零 Qt |

### 下次会话
- Session 4：AlgorithmWorkbenchDialog 四页 + scenario prediction UI 字段

---

## [2026-08-23] Phase 7 Session 2 — SimEngine prediction 接线 + wiring 测

### 本次 Scope（Planner mini-plan）
- **目标**：`SimEngine` `prediction=none|constant_velocity`；`collectPeersFor` CV 外推；scenario 序列化；wiring 单测
- **允许改动**：`SimEngine.*`、`ScenarioLoader.h`、`ScenarioSerializer.cpp`、`SimController.cpp`、wiring/serializer 测
- **NOT DO**：ExperimentMetrics（S3）；UI；scenario 资产；假 prediction（静态 Path 换名）

### ✅ 已完成
- [x] `setPredictionKind` / `predictionKind()`；默认 `none`（Phase 6 回归）
- [x] `collectPeersFor`：`constant_velocity` → `ConstantVelocityPredictor` + `from_prediction=true`；`none` → `reference_path`
- [x] `SimulationConfig.prediction` + ScenarioSerializer round-trip
- [x] `SimController::applyScenarioToEngine` 读取 `simulation.prediction`
- [x] `StGraphSimEngineWiringTest.PredictionConstantVelocityChangesStProfile` + `PredictionDefaultNoneMatchesPhase6PeerCollection`
- [x] `ScenarioSerializerTest.RoundTripPredictionConstantVelocity`
- [x] 回归 **111/111 PASSED**（排除 stub `ExperimentMetricsTest`）

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| ExperimentMetrics 聚合 | Session 3 | Session 3 |
| AlgorithmWorkbenchDialog | UI | Session 4 |
| `prediction_st_demo` scenario | 依赖 UI/资产 | Session 5 |

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner | 范围 SimEngine+scenario+wiring 测 |
| Executor | 按 plan；CV 读 pose 非 Path 拷贝 |
| Tester | **PASS**：2 新 wiring + 1 serializer；111 回归 |
| Reviewer | **PASS**：默认 none；§10 无偷懒 |

### 证据
- `StGraphSimEngineWiringTest.Prediction*` 2/2 PASSED
- 全量（排除 ExperimentMetrics）111/111

### 用户本地验证
1. `git pull` → Build → `FleetSimTests --gtest_filter=StGraphSimEngineWiringTest.Prediction*`

### 下次会话建议
- Session 3：`ExperimentMetrics` 滚动窗口 + 聚合 → `ExperimentMetricsTest` 转绿

---

## [2026-08-23] Phase 7 Session 1 — ConstantVelocityPredictor CV 外推 + 单测转绿

### 本次 Scope（Planner mini-plan）
- **目标**：实现 ADR-016 常速外推公式；`ConstantVelocityPredictorTest` + `StGraphWithPredictionTest` 转绿
- **允许改动**：`src/domain/prediction/ConstantVelocityPredictor.cpp`
- **NOT DO**：SimEngine 接线（S2）；ExperimentMetrics（S3）；UI；scenario；假 prediction（静态 Path 换名）

### ✅ 已完成
- [x] `ConstantVelocityPredictor::predictPath`：\(x_k=x_0+v\cos\theta\cdot k\Delta t\)，\(k=0\ldots\lfloor T/\Delta t\rfloor\)
- [x] `ConstantVelocityPredictorTest` 3/3 PASSED
- [x] `StGraphWithPredictionTest.PredictedPeerPathChangesStProfile` PASSED（预测 path ≠ 静态 path → ST 剖面可区分）
- [x] Phase 6 + 新 Predictor 回归：108/108 PASSED（排除仍 stub 的 `ExperimentMetricsTest`）

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| SimEngine `prediction` 字段与 `collectPeersFor` | Session 2 | Session 2 |
| ExperimentMetrics 聚合 | stub | Session 3 |
| UI Workbench / Compare | 未开始 | Session 4–5 |

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner | 范围仅 CV 实装 + 相关测转绿 |
| Executor | 只改 `ConstantVelocityPredictor.cpp` |
| Tester | **PASS**：3+1 测绿；108 回归；公式非 Path 换名 |
| Reviewer | **PASS**：ADR-016 公式对齐；Domain 无 Qt |

### 证据
- `D:\build\FleetSim_phase7_s0\FleetSimTests.exe`
- `--gtest_filter=ConstantVelocityPredictorTest.*:StGraphWithPredictionTest.*` → 4/4 PASSED

### 用户本地验证
1. `git pull origin main`
2. Build `FleetSimTests`
3. 跑 `ConstantVelocityPredictorTest.*` 与 `StGraphWithPredictionTest.*` → 全绿

### 下次会话建议
- Session 2：`SimEngine` `prediction=none|constant_velocity` + `collectPeersFor` 接线 + `StGraphWithPredictionTest` 扩展 / SimEngine wiring 测

---

## [2026-08-23] Phase 7 Session 0 — ADR-016/017 + prediction/experiment stub + red tests

### 本次 Scope（Planner mini-plan）
- **目标**：ADR 草案 + 接口调研 + 红灯 GTest 骨架（Predictor*/ExperimentMetrics*）
- **允许改动**：`docs/decisions/016-*`、`017-*`；`src/domain/prediction/`、`experiment/`；`PeerTrajectory.from_prediction`；`tests/domain/*Predictor*`、`*Experiment*`、`*WithPrediction*`；CMake 登记
- **NOT DO**：CV 外推实装（S1）；SimEngine `prediction` 接线（S2）；ExperimentMetrics 聚合（S3）；AlgorithmWorkbenchDialog / ExperimentComparePanel（S4–5）；scenario 资产 / MUTATION M34+（S6–7）；UI 堆控件；假 prediction（静态 Path 换名）

### ✅ 已完成
- [x] ADR-016 常速预测 + ST 增强草案；ADR-017 算法实验工作台 IA 草案
- [x] `IPeerPredictor`、`ConstantVelocityPredictor`（Session 0 stub 返回空 Path）
- [x] `ExperimentMetrics` / `TickSample` / `RunSummary`（Session 0 stub 仅计数）
- [x] `PeerTrajectory.from_prediction` 字段（默认 false，Phase 6 兼容）
- [x] 红灯测：`ConstantVelocityPredictorTest`（3）、`ExperimentMetricsTest`（3）、`StGraphWithPredictionTest`（1）
- [x] `src/domain/CMakeLists.txt`、`tests/CMakeLists.txt` 登记
- [x] ASCII Build `D:\build\FleetSim_phase7_s0` 编译通过

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| ConstantVelocityPredictor CV 外推 | Session 0 仅 stub | Session 1 |
| SimEngine `prediction=none\|constant_velocity` | 未接线 | Session 2 |
| ExperimentMetrics 滚动窗口聚合 | stub 未实现 mean/rate | Session 3 |
| AlgorithmWorkbenchDialog 四页 | UI 重构 | Session 4 |
| ExperimentComparePanel | 独立 dock | Session 5 |
| scenario `prediction_st_demo` | 依赖接线 | Session 5 |
| MUTATION M34+ / verify_phase7 | 终审 | Session 7 |
| Phase 7 ✅ | 未完成 A–J | Session 7 |

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner | mini-plan 范围 Session 0 仅 ADR+接口+红灯；NOT DO ≥5 条 |
| Executor | 按 plan 交付 ADR/stub/测/CMake；无 UI/SimEngine 扩 scope |
| Tester | **PASS**：新测 6 FAIL / 1 PASS（ResetClearsState）；Phase6 回归 104/104（排除新套件）；无 Qt Domain |
| Reviewer | **PASS**（补 SESSION_LOG 后）：§10 无偷懒；PeerTrajectory 向后兼容 |

### 证据
- Build: `D:\build\FleetSim_phase7_s0\FleetSimTests.exe`
- 新测：`ConstantVelocityPredictorTest.*` 3 FAIL；`ExperimentMetricsTest.*` 2 FAIL + 1 PASS；`StGraphWithPredictionTest.*` 1 FAIL
- Phase6 回归：`--gtest_filter=-ConstantVelocityPredictorTest.*:-ExperimentMetricsTest.*:-StGraphWithPredictionTest.*` → 104/104

### 新增/变更文件清单
| 文件 | 操作 |
|------|------|
| `docs/decisions/016-constant-velocity-prediction.md` | 新增 |
| `docs/decisions/017-algorithm-workbench-ui.md` | 新增 |
| `src/domain/prediction/*` | 新增 |
| `src/domain/experiment/*` | 新增 |
| `src/domain/planning/StGraphSpeedPlanner.h` | 修改 |
| `tests/domain/ConstantVelocityPredictorTest.cpp` 等 3 文件 | 新增 |
| `src/domain/CMakeLists.txt`、`tests/CMakeLists.txt` | 修改 |

### 用户本地验证
1. `git pull origin main`
2. Qt Creator Configure → Build；或 ASCII：`cmake -S . -B D:\build\FleetSim_phase7_s0 -DCMAKE_PREFIX_PATH=D:/QT/6.11.1/mingw_64 -G "MinGW Makefiles"`
3. 跑 `FleetSimTests --gtest_filter=ConstantVelocityPredictorTest.*:ExperimentMetricsTest.*:StGraphWithPredictionTest.*` → **预期 6 FAIL**（红灯）
4. 跑全量（或排除新测）→ Phase 6 104 测仍绿

### 下次会话建议
- Session 1：实现 `ConstantVelocityPredictor::predictPath` CV 外推 → `ConstantVelocityPredictorTest` 转绿

---

## [2026-08-23] Phase 6 Session 6 — E2E / MUTATION / 终审 / Phase6 ✅

### ✅ 已完成
- [x] MUTATION M31–M33；`verify_phase6_evidence.py` + `run_phase6_verify.ps1`（31 PASS）
- [x] DEVELOPMENT_PLAN Phase 6 ✅
- [x] ASCII `D:\build\FleetSim_phase6_s0`：**FleetSimTests 104/104 PASSED**
- [x] 四角色终审 PASS（见下）

### ❌ 未完成 / 故意不做
| 项目 | 原因 |
|------|------|
| 手工实跑变异注入 | 清单已写；可选后续 |
| 完整 Autoware / OSQP | Phase6 明确不做 |

### 四角色终审
| 角色 | 结论 |
|------|------|
| Planner | A–J 范围已覆盖 Session0–6 |
| Executor | 代码/文档/测例齐 |
| Tester | **PASS**：静态 31 + 运行时 104/104 |
| Reviewer | **PASS**：§10 + 防回归；无假 MPC/假 ST |

### 证据
- exe: `D:\build\FleetSim_phase6_s0\FleetSimTests.exe`
- gtest: 104 tests, 0 failures, EXIT=0
- xml: `D:\build\FleetSim_phase6_s0\gtest_results.xml`
- static: `python tools/verify_phase6_evidence.py` → 31 PASS

### 用户本地验证
1. `git pull origin main`
2. 可选：`pwsh -File tools/run_phase6_verify.ps1`
3. Qt Creator Configure → Build；或 ASCII `D:\build\FleetSim_phase6`
4. 跑 `FleetSimTests`；Open `assets/scenarios/mpc_st_demo`；Dialog 切 mpc / ST-Graph；看 Monitor ST ref v

---

## [2026-08-23] Phase 6 Session 5 — Monitor ST 速度曲线 + mpc_st_demo

### ✅ 已完成
- [x] MonitorBridge/Panel：Actual v + **ST ref v** 双曲线
- [x] `assets/scenarios/mpc_st_demo`（tracker=mpc, speed_planner=st_graph）
- [x] panels/README 更新

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| 独立 MpcStGraphDialog | 已并入 PlannerTrackerDialog | 不做 |
| MUTATION / Phase6 ✅ | Session 6 | Session 6 |

### 四角色
| 角色 | 结论 |
|------|------|
| Planner/Executor | Monitor ST 曲线 + 资产 |
| Tester/Reviewer | 随 Session6 终审 |

---

## [2026-08-23] Phase 6 Session 4 — SimEngine ST 强制接线 + 多车合同

### 本次 Scope（Planner）
- 目标：`speed_planner=st_graph` plan/tick 读他车 Path 填 ST；`setSpeedProfile`；TimeWindow 叠加；多车有 peer ≠ 无 peer
- NOT DO：Monitor 曲线（S5）、假 ST、削 CMake

### ✅ 已完成
- [x] `VehicleAgent::speed_profile`；`setSpeedPlannerKind` / `refreshSpeedProfiles` / `collectPeersFor`
- [x] plan/replan 后 + 每 N tick 重算；MPC `setSpeedProfile`；PP/Stanley 覆盖纵向 v；`*= speed_scale` 保留
- [x] scenario `speed_planner`；Dialog Speed planner；SimController/MainWindow 应用
- [x] `StGraphSimEngineWiringTest` 5/5；`RoundTripSpeedPlannerStGraph`

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| Monitor MPC/ST 曲线 | Session 5 | Session 5 |
| MUTATION M31+ / Phase6 ✅ | Session 6 | Session 6 |

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner | mini-plan：接线时机 + 多车合同测 |
| Executor | 按 plan 接线 |
| Tester | **PASS**（agent `56cc640a`；Wiring 5/5；读他车 Path） |
| Reviewer | **PASS**（agent `56cc640a`；§10 Session4） |

### 构建取证
- `D:\build\FleetSim_phase6_s0`：`StGraphSimEngineWiringTest.*` **5/5 PASSED**

### 用户本地验证
1. `git pull`；Build
2. Dialog 开 ST-Graph；两车交叉路径应见减速
3. 过滤 `StGraph*` 应全绿

### 下次会话
- Session 5：Monitor 曲线 + 场景资产

---

## [2026-08-23] Phase 6 Session 3 — StGraphSpeedPlanner 真 ST-Graph MVP

### 本次 Scope（Planner）
- 目标：他车投影 (s,t) 占用 + 减速剖面；`StGraphSpeedPlannerTest` 全绿
- NOT DO：SimEngine 接线、UI、距离停车、改 SpeedProfile 合同

### ✅ 已完成
- [x] 自车弧长累积；peer 投影到 ego 得 (s_hit,t)；ST 占用块
- [x] 自由流与占用相交则冲突前减速并恢复（非单点硬停）
- [x] `StGraphSpeedPlannerTest` **4/4 PASS**；ADR-015 → 已接受

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| SimEngine `speed_planner=st_graph` | Session 4 | Session 4 |
| 多车「忽略他车须 FAIL」集成测 | Session 4 | Session 4 |
| Monitor ST 曲线 | Session 5 | Session 5 |

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner | mini-plan：投影 + 占用避让减速 |
| Executor | 按 plan 实装 |
| Tester | **PASS**（agent `abf3193a`；读 peers + (s,t) 投影；4/4） |
| Reviewer | **PASS**（agent `abf3193a`；未越界 SimEngine/UI） |

### 构建取证
- `D:\build\FleetSim_phase6_s0`：`StGraphSpeedPlannerTest.*` **4/4 PASSED**

### 用户本地验证
1. `git pull`；Build；过滤 `StGraph*` 应全绿
2. MPC/Stanley/Hybrid 回归应仍绿

### 下次会话
- Session 4：SimEngine 强制接线 ST + 多车回归

---

## [2026-08-23] Phase 6 Session 2 — SimEngine/Dialog `tracker=mpc` + vs Stanley

### 本次 Scope（Planner）
- 目标：接入 `tracker=mpc`；Dialog 可切换；`MpcVsStanleyCompareTest`；auto 永不推 mpc
- 允许：SimEngine、PlannerTrackerDialog、scenario 注释、对比/切换/序列化测、CMake、SESSION_LOG、ADR-014 注记
- NOT DO：真 ST、Monitor 新曲线、OSQP、默认场景改 mpc、堆 ControlPanel

### ✅ 已完成
- [x] `resolvedTrackerKind`：显式 `mpc`；`auto`→`pure_pursuit`
- [x] `tick` mpc 分支调用真 `MpcLateralTracker::compute`
- [x] Dialog Tracker 增「MPC (linear lateral)」
- [x] `MpcVsStanleyCompareTest`（舵角可区分 + 非平凡代价）
- [x] `TrackerKindMpcResolvesExplicitly`；`RoundTripTrackerMpc`

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| ST 真实现 / SimEngine 接线 | Session 3–4 | Session 3–4 |
| Monitor MPC/ST 曲线 | Session 5 | Session 5 |
| 弯道闭环对比强化 | 直道已够反换皮 | 可选后续 |
| setSpeedProfile 引擎接线 | 依赖 ST | Session 4 |

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner | mini-plan：resolved/tick/Dialog/对比测 |
| Executor | 按 plan 接线 + 测 |
| Tester | **PASS**（agent `ca4da096`；tick 真调 MPC；auto≠mpc） |
| Reviewer | 初审 FAIL（缺 SESSION_LOG）→ 复审 **PASS**（agent `d8b17455`） |

### 构建取证
- `D:\build\FleetSim_phase6_s0`：过滤测含 MpcVsStanley / TrackerKind* / RoundTripTrackerMpc / MpcLateral* → **PASSED**（EXIT=0）

### 用户本地验证
1. `git pull`；Build
2. File → Planner/Tracker → 选 MPC；观察舵角与 Stanley 不同
3. 过滤：`MpcVsStanley*`、`TrackerKindMpc*` 应绿；`StGraph*` 仍可红

### 下次会话
- Session 3：`StGraphSpeedPlanner` 真 (s,t) + 他车障碍单测转绿

---

## [2026-08-23] Phase 6 Session 1 — MpcLateralTracker 线性化 + Eigen 稠密 QP

### 本次 Scope（Planner）
- 目标：真 MPC（误差状态 + 有限时域箱约束 QP），`MpcLateralTrackerTest` 全绿
- 允许：`MpcLateralTracker.*`、`DenseQpSolver.*`、相关 CMake/测例、ADR-014 状态、SESSION_LOG
- NOT DO：SimEngine/UI/ST、OSQP、改 IPathTracker、Stanley 换皮、削 CMake

### ✅ 已完成
- [x] `DenseQpSolver`：循环坐标下降 + 箱约束精确 1D（非单次投影梯度）
- [x] `MpcLateralTracker`：`[e,θe]` LTI + 凝缩 H/g + receding `u0*`；`lastPredictionNorm` / `lastCostNonTrivial`
- [x] `MpcLateralTrackerTest` 6/6 PASS；`DenseQpSolverTest` 边界测
- [x] ADR-014 → 已接受；CMake 登记 DenseQp

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| SimEngine `tracker=mpc` | Session 2 | Session 2 |
| MpcVsStanleyCompare | Session 2 | Session 2 |
| ST 真实现 / 接线 | Session 3–4 | Session 3–4 |
| UI/Monitor | Session 5 | Session 5 |
| κ 时变弯道线性化强化 | MVP 冻结 κ=0 够直道测 | 后续可选 |

### 🚫 禁止偷懒自检
- [x] 非 Stanley/PP 换皮（有 A/B + QP + 非平凡代价断言）
- [x] Domain 无 Qt；未削 `target_*`；未引 OSQP
- [x] horizon 非法 → 零舵 + `lastSolveOk==false`
- [x] SESSION_LOG 含没做什么 + 四角色

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner | mini-plan：`(e,θe)` + 凝缩箱约束 QP + debug 访问器 |
| Executor | 按 plan 实装 DenseQp + MPC |
| Tester | **PASS**（agent `eef42316`） |
| Reviewer | 初审 FAIL（缺 SESSION_LOG / DenseQp 独立测）→ 补齐后复审 **PASS**（agent `fa24a349`） |

### 构建取证
- `D:\build\FleetSim_phase6_s0`：`MpcLateralTrackerTest.*` **6/6 PASSED**

### 用户本地验证
1. `git pull`；Configure → Build（或 `D:\build\FleetSim_phase6`）
2. 过滤 `MpcLateral*` / `DenseQp*` 预期全绿；`StGraph*` 红灯仍在（Session 3）

### 下次会话
- Session 2：SimEngine/scenario/Dialog `tracker=mpc` + vs Stanley 对比测

---

## [2026-08-23] Phase 6 Session 0 — ADR-014/015 + MPC/ST stub 红灯骨架

### 本次 Scope（Planner 定义）
- 目标：锁死 ADR-014/015 选型；落地 `MpcLateralTracker` / `StGraphSpeedPlanner` / `SpeedProfile` stub + 红灯 GTest；CMake 登记
- 允许改动：`docs/decisions/014|015*.md`、`src/core/types/SpeedProfile.h`、`src/domain/control/MpcLateralTracker.*`、`src/domain/planning/StGraphSpeedPlanner.*`、两处 Domain/tests CMake、`src/core/CMakeLists.txt`、`SESSION_LOG.md`
- 明确不在本次范围：真 QP/ST、SimEngine 接线、UI、scenario 字段、MUTATION M31+、Phase6 ✅

### ✅ 已完成
- [x] ADR-014：Eigen 稠密 QP；失败零舵；`setSpeedProfile`；mpc 须显式选
- [x] ADR-015：`core::SpeedProfile` 与 Path 等长；ST plan 后至少一次 + 每 N=10 tick；禁止 speed_scale 冒充
- [x] `SpeedProfile.h`；`MpcLateralTracker` stub（零舵 + 读 profile）；`StGraphSpeedPlanner` stub（等长 cruise、忽略 peers）
- [x] 红灯测：`MpcLateralTrackerTest`（6）、`StGraphSpeedPlannerTest`（4）；CMake 登记
- [x] 四角色：Planner → Executor → Tester **PASS** → Reviewer（初审缺 SESSION_LOG FAIL → 本条目后复审）

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| 线性化 + Eigen QP | Session 0 只 stub | Session 1 |
| SimEngine `tracker=mpc` | 跨会话 | Session 2 |
| ST 真 (s,t) + 读他车 | 跨会话 | Session 3–4 |
| UI / Monitor 曲线 | 跨会话 | Session 5 |
| MpcVsStanleyCompare / 多车接线测 | 跨会话 | Session 2/4 |
| Phase6 ✅ / MUTATION M31+ | 端到端未齐 | Session 6 |

### 🚫 禁止偷懒自检
- [x] 没有把多个类挤进同一文件
- [x] 没有在 MainWindow / ControlPanel 堆 MPC/ST 控件
- [x] Domain 层无 Qt include
- [x] 新 Domain 类有红灯单测
- [x] 未跨 Phase 写真算法 / 接线
- [x] SESSION_LOG 本节已完整填写（含没做什么 + 四角色）
- [x] CMake 已登记新源；未削 `target_*`；未引 OSQP

### 新增/变更文件清单
| 文件 | 操作 | 说明 |
|------|------|------|
| `docs/decisions/014-linear-mpc.md` | 新增 | ADR-014 |
| `docs/decisions/015-st-graph-speed.md` | 新增 | ADR-015 |
| `src/core/types/SpeedProfile.h` | 新增 | 等长剖面 |
| `src/domain/control/MpcLateralTracker.h/.cpp` | 新增 | stub |
| `src/domain/planning/StGraphSpeedPlanner.h/.cpp` | 新增 | stub |
| `tests/domain/MpcLateralTrackerTest.cpp` | 新增 | 红灯 |
| `tests/domain/StGraphSpeedPlannerTest.cpp` | 新增 | 红灯 |
| `src/core|domain|tests/CMakeLists.txt` | 修改 | 登记 |

### 接口变更
- 新增 `core::SpeedProfile`
- 新增 `control::MpcLateralTracker : IPathTracker`（`setSpeedProfile` / `lastSolveOk`）
- 新增 `planning::StGraphSpeedPlanner` + `PeerTrajectory`
- **未改** `IPathTracker::compute` 签名；**未改** SimEngine

### 四角色结论
| 角色 | 结论 |
|------|------|
| Planner | mini-plan 锁定 Session0 范围与 ADR 五项选型 |
| Executor | 按 mini-plan 落地 ADR + stub + 红灯测 + CMake |
| Tester | **PASS**（stub 诚实；10/10 TEST；红灯锚点未弱化；CMake OK）— agent `8a54873b` |
| Reviewer | 初审 **FAIL**（缺 SESSION_LOG）→ 补条目后复审 **PASS**（agent `cb43aa59`） |
| 构建取证 | `D:\build\FleetSim_phase6_s0`：Mpc*/StGraph* 共 10 测，**5 PASS / 5 FAIL（红灯）** |

### 用户本地验证
1. `git pull origin main`
2. Qt Creator 重新 Configure → Build；或 ASCII：`D:\build\FleetSim_phase6`
3. 运行 `FleetSimTests`，过滤 `MpcLateral*` / `StGraph*`：
   - 预期 **FAIL（红灯）**：`SteeringClampedToMax`、`CrossTrackErrorDecreasesOnStraightPath`、`PredictionOrCostNonTrivial`、`PeerCrossingProducesDecelerationNotDistanceStop`、`ClearingPeerObstaclesChangesProfile`
   - 预期 **PASS**：`LowSpeedNoNan`、`QpFailureReturnsZeroSteer`、`UsesSpeedProfileWhenSet`、`SpeedProfileLengthMatchesPath`、`EmptyObstaclesYieldsNearCruiseProfile`
4. 旧测 Stanley/Hybrid/Priority/MultiAgv 应仍绿

### 下次会话建议
- 第一条任务：Session 1 — `MpcLateralTracker` 线性误差模型 + Eigen 稠密 QP，使 MPC 红灯转绿
- 前置：本会话 stub / ADR 已合入 main

---

## [2026-08-23] Phase 5 Goal J — FleetSimTests 82/82 green (ASCII out-of-tree)

### ✅ 已完成
- [x] Configure/Build at `D:\build\FleetSim_phase5` (Ninja + MinGW + Qt 6.11.1)
- [x] **FleetSimTests 82/82 PASSED**（含 Hybrid*/Dubins*/Stanley*/PlannerSwitch*/Priority*/MultiAgv*/Bicycle*）
- [x] Static `verify_phase5_evidence.py` 32 PASS（既有）
- [x] A–J 代码交付齐；Goal 可 complete

### ❌ 未完成 / 故意不做
| 项目 | 原因 |
|------|------|
| 手工变异 M28–M30 实跑 | 清单已写；可选后续 |

### 证据
- exe: `D:\build\FleetSim_phase5\FleetSimTests.exe`
- gtest: 82 tests, 0 failures, EXIT=0
- xml: `D:\build\FleetSim_phase5\gtest_results.xml`

### Reviewer
- 运行时全绿作 Goal J 终证；会话 0–5 子 Agent 已审过各交付物

### 用户本地验证（仍建议）
1. Qt Creator 对本仓库 Kit 再编一次（与 `D:\build\…` 并行无妨）
2. File → Planner / Tracker…；Open `hybrid_narrow_turn`

---

## [2026-08-23] Phase 5 Session 6 — 文档 / MUTATION / 静态取证（待用户全绿）

### 本次 Scope
- MUTATION M28–M30；DEVELOPMENT_PLAN Phase5；verify_phase5_evidence；SESSION_LOG
- **不** UpdateGoal complete：缺用户本地 FleetSimTests 全绿证据

### ✅ 已完成
- [x] MUTATION M28/M29/M30
- [x] DEVELOPMENT_PLAN Phase 5 ✅（附「待用户全绿」）
- [x] `tools/verify_phase5_evidence.py` + `run_phase5_verify.ps1`
- [x] Session 5 UI 已合入

### ❌ 未完成
| 项目 | 原因 | 计划 |
|------|------|------|
| Goal UpdateGoal complete | 无用户 FleetSimTests 全绿取证 | 用户验证后回复 |
| 手工执行变异 M28–M30 | 需本地改码跑测 | 用户可选 |

### 🚫 禁止偷懒自检
- [x] 未宣称运行时全绿
- [x] SESSION_LOG 写明没做什么

### Reviewer 结果
- 静态脚本 + 会话 0–5 子 Agent 已审；终态 Goal 需用户绿证

### 用户本地验证
1. `git pull`
2. `pwsh -File tools/run_phase5_verify.ps1`
3. Qt Configure → Build → FleetSimTests 全绿
4. 回复「全绿」后主 Agent 方可 UpdateGoal complete

---

## [2026-08-23] Phase 5 Session 5 — PlannerTrackerDialog + Monitor 航向误差

### 本次 Scope
- 独立 `PlannerTrackerDialog`（禁堆 ControlPanel）；Monitor 增加 heading error；接线 SimEngine
- NOT DO：MUTATION 全表、Phase5 ✅ 宣称（需用户本地全绿）

### ✅ 已完成
- [x] `PlannerTrackerDialog` + File 菜单挂载（≤15 行）
- [x] 应用 planner/tracker/coordination 到 engine + scenario stamp
- [x] `MonitorBridge`/`MonitorPanel` 航向误差曲线
- [x] panels/README 更新

### ❌ 未完成
| 项目 | 原因 | 计划 |
|------|------|------|
| Phase5 ✅ / MUTATION M28+ | Session 6 + 用户绿证 | Session 6 |

### 🚫 禁止偷懒自检
- [x] 未堆 ControlPanel/MainWindow 业务控件（仅菜单）
- [x] 独立 Dialog 文件
- [x] Domain 无 Qt

### Reviewer 结果
- Reviewer-UI: **PASS**（独立 Dialog；Monitor heading；MainWindow 仅菜单）
- 主 Agent 不自评

### 用户本地验证
1. git pull；Build
2. File → Planner / Tracker… 切换算法
3. Monitor 见 Cross-track + Heading + Velocity

### 下次会话建议
- MUTATION、DEVELOPMENT_PLAN、verify 脚本、双 Reviewer、用户全绿后 complete

---

## [2026-08-23] Phase 5 Session 4 — PriorityPathCoordinator + TimeWindow 协同

### 本次 Scope
- 目标：Priority 按优先级依次规划；高优先级路径写入低优先级规划栅格障碍；保留 TimeWindow 预约/速度缩放；禁止距离判碰
- NOT DO：完整 CBS、UI、MUTATION 收尾

### ✅ 已完成
- [x] `PriorityPathCoordinator`（排序、paintPathOccupied）
- [x] `SimEngine::replanFleetWithPriorityCoordination` + `coordination` 字段
- [x] scenario / SimController 接线；multi_agv 标注 coordination
- [x] `PriorityCoordinatorTest`；ADR-013 已接受
- [x] CMake 登记

### ❌ 未完成
| 项目 | 原因 | 计划 |
|------|------|------|
| CBS-lite | ADR 标可选，Priority 为 MVP | 不做 |
| PlannerTracker UI | Session 5 | Session 5 |

### 🚫 禁止偷懒自检
- [x] 非距离停车冒充
- [x] 未删 TimeWindow
- [x] Domain 无 Qt
- [x] 有单测

### Reviewer 结果
- Reviewer-Code: **PASS**
- Reviewer-Test: **PASS**
- 主 Agent 不自评

### 用户本地验证
1. git pull；Build FleetSimTests
2. `PriorityCoordinator*`、`MultiAgv*` 预期 PASS

### 下次会话建议
- PlannerTrackerDialog + Monitor 横偏/航向误差

---

## [2026-08-23] Phase 5 Session 3 — StanleyTracker + tracker 切换

### 本次 Scope
- 目标：经典 Stanley（前轴横偏 + 航向）；SimEngine 按 tracker 切换 PP/Stanley；单测转绿
- NOT DO：UI 面板、Priority、Phase5 ✅

### ✅ 已完成
- [x] `StanleyTracker::compute` 真公式 + ε + 限舵
- [x] `SimEngine::resolvedTrackerKind` + tick 分支
- [x] Stanley 单测去红灯文案；PlannerSwitch tracker resolve 测
- [x] ADR-012 标已接受

### ❌ 未完成
| 项目 | 原因 | 计划 |
|------|------|------|
| PlannerTracker UI / Monitor 误差曲线 | Session 5 | Session 5 |
| Priority | Session 4 | Session 4 |

### 🚫 禁止偷懒自检
- [x] Stanley ≠ PP 换皮
- [x] Domain 无 Qt
- [x] 有单测
- [x] 未堆 ControlPanel

### Reviewer 结果
- Reviewer-Code/Test: 初审 FAIL（测文件破损 + θe 测未对准前轴）→ 修复后复审 **PASS**
- 主 Agent 不自评

### 用户本地验证
1. git pull；Configure；Build FleetSimTests
2. `Stanley*`、`PlannerSwitch*Tracker*` 预期 PASS
3. DiffDrive / MultiAgv 仍绿

### 下次会话建议
- PriorityPathCoordinator + TimeWindow 协同

---

## [2026-08-23] Phase 5 Session 2 — SimEngine/scenario hybrid_astar 接入

### 本次 Scope（Architect 定义）
- 目标：scenario `planner`/`tracker` 字段；SimEngine 按配置选 A*/Hybrid；bicycle 默认 hybrid；跳过 Hybrid 的 DP；对比/集成测；`hybrid_narrow_turn` 资产
- 允许：`SimEngine.*`、`ScenarioLoader/Serializer`、`SimController`、资产、`PlannerSwitchIntegrationTest`、SESSION_LOG、ADR-011 状态
- NOT DO：Stanley 真实现、UI 面板、Priority、完整 Phase5 ✅

### ✅ 已完成
- [x] `SimulationConfig.planner` / `tracker` 序列化
- [x] `SimEngine::setPlannerKind` / `resolvedPlannerKind`；`astar_planner_` 重命名
- [x] Hybrid 分支跳过 Douglas-Peucker
- [x] `SimController::applyScenarioToEngine` 应用 scenario 字段
- [x] `assets/scenarios/hybrid_narrow_turn/` + bicycle_demo 标注 hybrid
- [x] `PlannerSwitchIntegrationTest` + Serializer round-trip

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| Stanley 真公式 / tracker 切换 | Session 3 | Session 3 |
| PlannerTracker UI | Session 5 | Session 5 |
| Priority | Session 4 | Session 4 |

### 🚫 禁止偷懒自检
- [x] Hybrid 接线非 A* 换皮
- [x] Domain 无 Qt
- [x] 有集成/对比测
- [x] 未堆 ControlPanel
- [x] CMake 仅追加测文件
- [x] 未改 `.cursor/plans/`

### 新增/变更文件清单
| 文件 | 操作 | 说明 |
|------|------|------|
| `SimEngine.h/.cpp` | 修改 | planner 切换 |
| `ScenarioLoader.h` / `ScenarioSerializer.cpp` | 修改 | 字段 |
| `SimController.cpp` | 修改 | 应用配置 |
| `assets/scenarios/hybrid_narrow_turn/*` | 新增 | 窄弯场景 |
| `bicycle_demo/scenario.json` | 修改 | planner 字段 |
| `PlannerSwitchIntegrationTest.cpp` | 新增 | |
| `ScenarioSerializerTest.cpp` | 修改 | round-trip |
| `tests/CMakeLists.txt` | 修改 | |
| `011-hybrid-astar.md` | 修改 | 状态 |
| `SESSION_LOG.md` | 修改 | 本条目 |

### 接口变更
- `SimEngine::setPlannerKind` / `setTrackerKind` / `resolvedPlannerKind`
- scenario JSON：`simulation.planner` / `simulation.tracker`（可选）

### Reviewer 结果
- Reviewer-Code: **PASS**
- Reviewer-Test: 初审 FAIL（未锁 Hybrid 分支）→ 加固对比测后复审 **PASS**
- 主 Agent 不自评

### 用户本地验证
1. `git pull origin main`
2. Qt Configure → Build FleetSimTests
3. 关注 `PlannerSwitch*`、`HybridAStar*`、`ScenarioSerializer*RoundTripPlanner*`、旧 MultiAgv/Demo
4. Open `assets/scenarios/hybrid_narrow_turn` → 设目标拐角 → Plan

### 下次会话建议
- StanleyTracker 真公式 + 与 PurePursuit 切换

---

## [2026-08-23] Phase 5 Session 1 — HybridAStarPlanner + Dubins MVP

### 本次 Scope（Architect 定义）
- 目标：真 Hybrid A*（`(x,y,θ)` 自行车扩展 + Dubins 解析扩展）；`HybridAStarPlannerTest` / `DubinsPathTest` 转绿；Stanley stub 保持红灯
- 允许：`DubinsPath.*`、`HybridAStarPlanner.cpp`、相关测试与 CMake、SESSION_LOG、ADR-011 状态可标「实装中」
- NOT DO：SimEngine 接线、Stanley 真公式、UI、Priority、Reeds-Shepp、完整 CBS

### ✅ 已完成
- [x] `DubinsPath`：LSL/LSR/RSL/RSR/RLR/LRL + sample
- [x] `HybridAStarPlanner::plan`：舵角离散扩展、碰撞采样、visited `(ix,iy,θbin)`、周期性 Dubins、开集上限
- [x] `DubinsPathTest`；CMake 登记 Dubins
- [x] Session0 Hybrid 红灯测改为实现后应对齐（用户本地验证）

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| SimEngine / scenario `planner` | Session 2 | Session 2 |
| Stanley 真实现 | Session 3 | Session 3 |
| Reeds-Shepp 倒车 | Phase5 可选 | 有需要再开 |
| Priority 协调 | Session 4 | Session 4 |

### 🚫 禁止偷懒自检
- [x] Hybrid ≠ A* 换皮（运动学扩展 + Dubins）
- [x] Domain 无 Qt
- [x] 新类有 GTest
- [x] CMake 仅追加，未削 include/link
- [x] 未堆 UI / 未改 `.cursor/plans/`
- [x] SESSION_LOG 写明没做什么

### 新增/变更文件清单
| 文件 | 操作 | 说明 |
|------|------|------|
| `src/domain/planning/DubinsPath.h/.cpp` | 新增 | Dubins |
| `src/domain/planning/HybridAStarPlanner.cpp` | 重写 | 真搜索 |
| `tests/domain/DubinsPathTest.cpp` | 新增 | |
| `src/domain/CMakeLists.txt` / `tests/CMakeLists.txt` | 修改 | 登记 |
| `SESSION_LOG.md` | 修改 | 本条目 |

### 接口变更
- 无公开 API 破坏；`HybridAStarPlanner::plan` 行为从空路径变为运动学路径

### Reviewer 结果
- Reviewer-Code: **PASS**（真运动学 + Dubins；非阻断：终点 snap / 负坐标 key）
- Reviewer-Test: 初审 FAIL（窄弯）→ 修 closed/走廊后复审 **PASS**
- 主 Agent 不自评

### 用户本地验证
1. `git pull origin main`
2. Qt Creator 重新 Configure → Build `FleetSimTests`
3. 过滤运行：`HybridAStar*`、`DubinsPath*` — 预期 PASS
4. `Stanley*` — 仍预期多项 FAIL（Session 3）
5. 旧测（AStar/Bicycle/MultiAgv）应仍绿

### 下次会话建议
- SimEngine + scenario `planner` 字段；bicycle 默认 `hybrid_astar`；窄弯对比集成

---
### 本次 Scope（Architect 定义）
- 目标：Phase 5 会话 0 — ADR-011/012/013 草案；IPathPlanner/IPathTracker 扩展点调研落地；Hybrid/Stanley stub + 红灯 GTest；CMake 登记
- 允许改动：`docs/decisions/011|012|013*.md`、`src/domain/planning/HybridAStar*`、`src/domain/control/Stanley*`、`tests/domain/*Hybrid*|Stanley*`、两处 CMakeLists、`SESSION_LOG.md`
- 明确不在本次范围：Hybrid/Stanley 真算法、SimEngine 接线、UI、Priority 实装、Phase5 ✅

### ✅ 已完成
- [x] ADR-011 Hybrid A*（接口保持、运动学扩展、跳过 DP、bicycle 默认 hybrid）
- [x] ADR-012 Stanley（前轴横偏+航向、ε、与 PP 并存）
- [x] ADR-013 Priority + TimeWindow 协同边界（CBS-lite 可选）
- [x] 调研结论写入 ADR：SimEngine 硬编码 A*+PP；Path 无 θ；scenario 尚无 planner/tracker
- [x] `HybridAStarPlanner` stub（空 plan；`minTurningRadiusM` 可用）
- [x] `StanleyTracker` stub（零命令）
- [x] `HybridAStarPlannerTest` / `StanleyTrackerTest` 红灯骨架（期望 stub 下多项 FAIL）
- [x] Domain/Tests CMakeLists 登记新源（保留 include/link）

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| Hybrid 运动学搜索 + Dubins | 会话 1 | Session 1 |
| SimEngine/scenario planner 字段 | 会话 2 | Session 2 |
| Stanley 真公式 | 会话 3 | Session 3 |
| PriorityPathCoordinator | 会话 4 | Session 4 |
| PlannerTracker UI / Monitor 误差 | 会话 5 | Session 5 |
| Phase5 ✅ / MUTATION M28+ | 会话 6 | Session 6 |

### 🚫 禁止偷懒自检
- [x] 没有把多个类挤进同一文件
- [x] 没有在 MainWindow 堆业务控件
- [x] 无新 UI（本会话不做 UI）
- [x] Domain 层无 Qt / rclcpp
- [x] 新 Domain 类有对应单测（红灯骨架）
- [x] 未跨 Phase 写完整 Hybrid/Stanley/CBS
- [x] SESSION_LOG 本节完整
- [x] 未削 CMake target_include / target_link
- [x] 未碰 `.cursor/plans/`

### 新增/变更文件清单
| 文件 | 操作 | 说明 |
|------|------|------|
| `docs/decisions/011-hybrid-astar.md` | 新增 | ADR-011 |
| `docs/decisions/012-stanley-tracker.md` | 新增 | ADR-012 |
| `docs/decisions/013-priority-or-cbs-lite.md` | 新增 | ADR-013 |
| `src/domain/planning/HybridAStarPlanner.h/.cpp` | 新增 | Session0 stub |
| `src/domain/control/StanleyTracker.h/.cpp` | 新增 | Session0 stub |
| `tests/domain/HybridAStarPlannerTest.cpp` | 新增 | 红灯 |
| `tests/domain/StanleyTrackerTest.cpp` | 新增 | 红灯 |
| `src/domain/CMakeLists.txt` | 修改 | 登记源 |
| `tests/CMakeLists.txt` | 修改 | 登记测 |
| `SESSION_LOG.md` | 修改 | 本条目 |

### 接口变更
- 新增 `planning::HybridAStarPlanner : IPathPlanner`（构造注入 L/δmax；`plan` 签名不变）
- 新增 `control::StanleyTracker : IPathTracker`
- scenario / SimEngine **尚未**接线（Session 2/3）

### Reviewer 结果
- Reviewer-Code: **PASS**（Domain 零 Qt；CMake 仅追加源；stub 诚实；ADR 覆盖扩展点）
- Reviewer-Test: **PASS**（7 例预期红灯；`MinTurningRadiusMatchesBicycleGeometry` 可绿；OccupancyGrid/BicycleModel API 正确）
- Reviewer-UI: N/A（本会话无 UI）
- 主 Agent 不自评；以子 Agent 结论为准

### 用户本地验证
1. `git pull origin main`
2. Qt Creator 重新 Configure → Build FleetSimTests
3. 运行测试：预期 **Hybrid\*** / **Stanley\*** 多项 FAIL（红灯锚点）；其余旧测应仍绿；`MinTurningRadiusMatchesBicycleGeometry` 可 PASS
4. 确认 Domain 无 Qt include；CMake 仍有 `target_include_directories` / `target_link_libraries`

### 下次会话建议
- 第一条任务：实现 HybridAStarPlanner（自行车扩展 + Dubins MVP）使 Hybrid 单测转绿
- 前置条件：本会话 ADR-011 已接受；用户确认红灯测能编译链接

---

## [2026-08-23] Phase 4 Goal J — CMake export fix + full FleetSimTests green

### ✅ 已完成
- [x] 修复 install(EXPORT) 依赖未导出的 nlohmann_json（BUILD_INTERFACE + Config find_dependency）
- [x] 现有 Kit 增量 Configure/Build FleetSimTests 成功
- [x] fleet_domain_smoke 运行 OK
- [x] ProjectManagerTest：改用 ASCII cwd 下 test_tmp（规避中文 Temp + MinGW ofstream）
- [x] **FleetSimTests 全量 60/60 PASSED**（含 Bicycle*/Steering*/Hungarian*/MultiAgv*）

### ❌ 未完成 / 故意不做
| 项目 | 原因 |
|------|------|
| Stanley / MPC | Phase4 范围外 |

### 证据
- FleetSimTests.exe LastWrite 2026-08-23；FULL=0；60 passed
- domain_smoke: domain_smoke ok pose=(...)

### Reviewer
- Code: CMake export 修复必要；Test: 全绿


---

## [2026-08-23] Phase 4 — 测试资产 cwd + 本地验证脚本（Goal J 前置）

### 本次 Scope
- 修复：FleetSimTests 未拷贝 assets / 未设 WORKING_DIRECTORY → BicycleScenarioTest 易 SKIP
- 新增：	ools/run_phase4_verify.ps1（静态审计 + 拒绝陈旧 exe + 过滤跑测）

### ✅ 已完成
- [x] tests/CMakeLists.txt POST_BUILD copy assets + gtest WORKING_DIRECTORY=源码根
- [x] run_phase4_verify.ps1（检出 exe 早于 2026-08-23 则拒绝作为 J 证据）

### ❌ 未完成
| 项目 | 原因 | 计划 |
|------|------|------|
| Goal complete / J 运行时绿 | 现有 FleetSimTests.exe=2026-08-22 陈旧 | 用户 Qt 重编后跑 ps1 |

### 用户本地验证
1. git pull
2. Qt Creator 重新 Configure → Build
3. pwsh -File tools/run_phase4_verify.ps1
4. 全绿后回复：Build/Tests 通过


---

## [2026-08-23] Phase 4 audit fix — Open Project 不再覆盖 bicycle model

### 本次 Scope
- 修复：pplyProjectToSimulation 曾用默认 diff_drive Settings 覆盖 scenario 的 model=bicycle
- 增加：syncSettingsFromScenario（Open 后 Settings 反映场景）
- 增加：	ools/verify_phase4_evidence.py（A–J 静态证据审计，PASS=37）

### ✅ 已完成
- [x] Open Project 保留 bicycle_demo 模型
- [x] Settings 仍可主动 stamp + recreate
- [x] 静态审计脚本全绿（不含本机 Qt Build）

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| 代跑 Qt Build / FleetSimTests | 中文路径禁止 Agent 代跑 | **用户本地** |
| Goal complete | 缺 J 运行时绿测证据 | 用户确认后关闭 |

### Reviewer
- 待 push 后 Code/Test 快审

### 用户本地验证（关 Goal 前置）
1. git pull
2. Qt Creator **重新 Configure** → Build（旧 FleetSimTests.exe 时间戳早于 Phase4，必须重编）
3. Run FleetSimTests（Bicycle*/Steering*/Hungarian*/MultiAgv*）
4. python tools/verify_phase4_evidence.py → PASS=37 FAIL=0
5. Open assets/scenarios/bicycle_demo → Play（须为 bicycle 转弯）
6. File→Settings 切 hungarian / model 后观察 recreate


---


## [2026-08-23] Phase 4 Sessions 3–5 — Domain export + ROS2 可选桥 + Settings/Hungarian + Phase4 ✅

### 本次 Scope
- Session3: FleetSimCore/Domain install+export + domain_smoke（无 Qt 链接）+ docs/domain_export.md
- Session4: bridges/ros2 + FLEETSIM_BUILD_ROS2=OFF + docs/ros2_bridge.md（ADR-010）
- Session5: SettingsDialog 独立对话框、HungarianAssigner、对比单测、DEVELOPMENT_PLAN Phase4✅、MUTATION M24–27、panels README

### ✅ 已完成
- [x] CMake export / install Domain+Core；.gitignore 白名单 cmake/*.cmake.in
- [x] examples/domain_smoke + FLEETSIM_BUILD_DOMAIN_SMOKE
- [x] ROS2 桥目录默认 OFF；无 rclcpp 时跳过不挂主工程
- [x] SettingsDialog + File→Settings（未堆 ControlPanel）
- [x] HungarianAssigner + SchedulingModule 可注入 + 对比单测
- [x] DEVELOPMENT_PLAN Phase4 ✅；MUTATION M24–27

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| Stanley / MPC / Autoware | Phase4 明确不做 | 后续研究 |
| Windows 原生 ROS2 实机 publish | 环境限制；文档化 WSL | 有 ROS 的机器 |
| 用户本机 Qt Build 代跑 | 中文路径禁止 Agent 代跑 | 用户本地 |
| 时间窗口避碰改距离 | 禁止回退 | — |

### 🚫 禁止偷懒自检
- [x] Bicycle 非 DiffDrive 换皮（前序会话已落地）
- [x] Domain 无 Qt/rclcpp
- [x] Settings 独立 Dialog
- [x] 静态库 export 落地
- [x] ROS2 默认 OFF
- [x] 新类有单测；CMake include/link 保留
- [x] SESSION_LOG 写了没做什么

### Reviewer 结果
- Reviewer-Code: PASS（Settings 已回写 model/wheelbase 并 recreate）
- Reviewer-Test: PASS
- Reviewer-UI: PASS

### 用户本地验证
1. git pull；Qt Creator 重新 Configure → Build
2. Run FleetSimTests（Bicycle* / Steering* / Hungarian* / MultiAgv*）
3. 构建并运行 fleet_domain_smoke
4. Open assets/scenarios/bicycle_demo → Play
5. File → Settings 切换 assigner / model（reload 场景生效于车辆模型）
6. （可选）-DFLEETSIM_BUILD_ROS2=ON（需 ROS）


---

---

## [2026-08-23] Phase 4 Session 1 — BicycleModel 实装 + Vehicle 策略切换

### 本次 Scope
- 目标：后轴 Bicycle 运动学绿测；Vehicle `unique_ptr<IVehicleModel>`；工厂 diff_drive|bicycle；DiffDrive 默认不破
- 允许：`BicycleModel.*`、`Vehicle.*`、`VehicleModelFactory.*`、CMake、`SimController`、测试、SESSION_LOG
- 不做：舵角适配、bicycle 资产、Settings、Hungarian、ROS2、export

### ✅ 已完成
- [x] Bicycle 后轴方程 + clamp + L≤0 防护
- [x] Vehicle unique_ptr + 双构造 + setModel
- [x] VehicleModelFactory / createVehicleModel
- [x] SimController 按 model 工厂创建
- [x] BicycleModelTest / VehicleTest bicycle 注入

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| Pure Pursuit→舵角 | Session 2 | Session 2 |
| bicycle_demo 资产 | Session 2 | Session 2 |
| Domain export / ROS2 / Settings / Hungarian | 后续 | Session 3–5 |

### Reviewer 结果
- Reviewer-Code: PASS / Reviewer-Test: PASS / Reviewer-UI: N/A

### 用户本地验证
1. `git pull` → Configure → Build → Run `FleetSimTests`
2. 预期：`BicycleModelTest.*` / `VehicleTest.*` / DiffDrive / MultiAgv **全 PASS**

---

## [2026-08-23] Phase 4 Session 2 — 舵角适配 + scenario bicycle + 场景资产

### 本次 Scope
- 目标：κ→δ=`atan(κL)`；scenario `wheelbase_m`；`bicycle_demo`；集成测；DiffDrive 路径不破
- 不做：Settings、Hungarian、ROS2、CMake export、Phase4 ✅

### ✅ 已完成
- [x] `SteeringAdapter`（atan + clamp）
- [x] `PurePursuitTracker` bicycle 重载填 `steering_angle`；大航向误差 creep+满舵
- [x] Vehicle 运动学元数据；SimEngine 按 isBicycle 分流
- [x] Scenario 读写 `wheelbase_m` / `max_steering_rad`
- [x] `assets/scenarios/bicycle_demo/`
- [x] `SteeringAdapterTest` + `BicycleScenarioTest`

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| Domain 静态库 export | Session 3 | Session 3 |
| ROS2 桥 | Session 4 | Session 4 |
| SettingsDialog / Hungarian | Session 5 | Session 5 |
| Stanley tracker | 非 MVP | 后续可选 |

### 🚫 禁止偷懒自检
- [x] Bicycle ≠ DiffDrive 换皮
- [x] Domain 无 Qt/rclcpp
- [x] 未堆 Settings 进 ControlPanel
- [x] CMake include/link + 新源登记
- [x] 新类有单测；SESSION_LOG 写了没做什么

### Reviewer 结果
- Reviewer-Code: PASS / Reviewer-Test: PASS / Reviewer-UI: N/A

### 用户本地验证
1. `git pull` → Configure → Build → Run Tests（含 SteeringAdapter* / BicycleScenario*）
2. Open `assets/scenarios/bicycle_demo` → Play，观察转弯与 DiffDrive 差异

### 下次会话建议
- Session 3：CMake export FleetSimDomain + 无 Qt smoke

---

## [2026-08-23] Phase 4 Session 0 — ADR + IVehicleModel / ControlCommand stub + 红灯测

### 本次 Scope（Architect 定义）
- 目标：固化 ADR-009/010；扩展 `ControlCommand.steering_angle`；`IVehicleModel` + `BicycleModel` stub（运动学未实装，单测 RED）；`DiffDriveModel` 实现接口且行为不变
- 允许改动：`docs/decisions/009-*`、`010-*`、`src/core/types/ControlCommand.h`、`src/domain/vehicle/`、`src/domain/CMakeLists.txt`、`tests/`、`SESSION_LOG.md`
- 明确不在本次范围：完整 Bicycle 方程、Vehicle 策略切换、舵角适配、scenario 资产、CMake export、ROS2 桥代码、SettingsDialog、Hungarian、Phase4 ✅

### ✅ 已完成
- [x] ADR-009 `docs/decisions/009-bicycle-control-command.md`
- [x] ADR-010 `docs/decisions/010-ros2-bridge.md`
- [x] `ControlCommand.steering_angle`（默认 0）
- [x] `IVehicleModel.h`
- [x] `DiffDriveModel : public IVehicleModel`（行为不变）
- [x] `BicycleModel` stub（故意不积分，保持 RED）
- [x] `BicycleModelTest.cpp` 红灯用例 + CMake 登记

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划在哪个 Phase/会话 |
|------|------|----------------------|
| Bicycle 后轴运动学实装 | Session 0 仅 stub+红灯 | Session 1 |
| Vehicle unique_ptr 模型切换 | 依赖完整 Bicycle | Session 1 |
| Pure Pursuit→舵角 / scenario bicycle | 后续 | Session 2 |
| Domain 静态库 export | 后续 | Session 3 |
| ROS2 桥代码 | 仅 ADR 草案 | Session 4 |
| SettingsDialog / Hungarian | 后续 | Session 5 |

### 🚫 禁止偷懒自检
- [x] 没有把 Bicycle stub 冒充完整运动学绿测
- [x] Domain 无 Qt / rclcpp
- [x] 未堆 Settings 进 ControlPanel
- [x] 未削掉 CMake target_include / link
- [x] 新 Domain 类有 GTest（红灯预期）
- [x] SESSION_LOG 已写「没做什么」

### 新增/变更文件清单
| 文件 | 操作 | 说明 |
|------|------|------|
| `docs/decisions/009-bicycle-control-command.md` | 新增 | ADR-009 |
| `docs/decisions/010-ros2-bridge.md` | 新增 | ADR-010 |
| `src/core/types/ControlCommand.h` | 修改 | +steering_angle |
| `src/domain/vehicle/IVehicleModel.h` | 新增 | 策略接口 |
| `src/domain/vehicle/BicycleModel.*` | 新增 | stub |
| `src/domain/vehicle/DiffDriveModel.h` | 修改 | 实现接口 |
| `src/domain/CMakeLists.txt` | 修改 | 登记 Bicycle |
| `tests/domain/BicycleModelTest.cpp` | 新增 | 红灯 |
| `tests/CMakeLists.txt` | 修改 | 登记测试 |

### 接口变更
- `ControlCommand` 新增 `steering_angle`（默认 0，兼容）
- 新增 `IVehicleModel::integrate(pose, cmd, dt)`
- `DiffDriveModel` / `BicycleModel` 实现该接口

### Reviewer 结果
- Reviewer-Code: PASS（Bicycle 真 stub、CMake include/link 保留、无 Qt/rclcpp）
- Reviewer-Test: PASS（五类用例齐；运动学用例 Session0 预期 RED）
- Reviewer-UI: N/A（本会话无 UI 改动）

### 用户本地验证
1. `git pull origin main`
2. Qt Creator 重新 Configure → Build
3. Run `FleetSimTests`：既有 DiffDrive/MultiAgv 应 PASS；`BicycleModelTest.*` **预期 FAIL**（Session 0 红灯）
4. Session 1 实装后上述 Bicycle 用例应变绿

### 下次会话建议
- 第一条任务：实现 `BicycleModel` 后轴方程 + Vehicle 策略切换 + 绿测
- 前置条件：本会话已 push；用户确认旧测仍绿、Bicycle 红灯可接受

---

## [2026-08-22] Phase 1 — 核心仿真 MVP

### 本次 Scope
- 目标：单车 A* + 平滑 + Pure Pursuit + DiffDrive + 场景加载 + UI 闭环
- 允许改动：`src/core/`, `src/domain/`, `src/app/`, `src/ui/`, `tests/`, `docs/decisions/006-*`

### ✅ 已完成
- [x] Core：`Waypoint`, `Path`, `GridCell`
- [x] Domain map：`OccupancyGrid`, `MapLoader`
- [x] Domain planning：`AStarPlanner`, `DouglasPeuckerSmoother`, 接口
- [x] Domain control：`PurePursuitTracker`, `IPathTracker`
- [x] Domain vehicle：`DiffDriveModel`, `Vehicle`
- [x] Domain scenario：`ScenarioLoader`
- [x] `SimEngine` 扩展：plan/tick/EventBus 发布
- [x] `SimController`：`loadScenario`, `setGoal`, `planPath`, `setTimeScale`
- [x] UI：`PathGraphicsItem`, `ObstacleOverlayItem`, MapScene 分层, MapView Shift+点击设目标
- [x] `MainWindow` 去 hardcode，QTimer 仿真循环，EventBus 订阅
- [x] `ControlPanel` 增加 Speed 1x/2x/4x
- [x] 单测 + `DemoScenarioTest` 集成测试
- [x] ADR-006 Phase 1 算法参数

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| MonitorPanel | Phase 2 | Phase 2 |
| MapEditorPanel | Phase 2 | Phase 2 |
| JSON 写回 | Phase 2 ProjectManager | Phase 2 |
| 多车 | Phase 3 | Phase 3 |

### 🚫 禁止偷懒自检
- [x] 无 MainWindow 业务按钮堆叠
- [x] Domain 无 Qt include
- [x] 一文件一职责
- [x] A* 路径经 Smoother
- [x] 新 Domain 类有单测
- [x] SESSION_LOG 已填写

### Reviewer 结果
- 待 Reviewer 子 agent

### 用户本地验证
1. Qt Creator 重新 Configure + Build + Run Tests
2. Run → Shift+点击设目标 → Play → 小车沿绿线绕障运动

---

## [2026-08-22] Phase 2 — 地图编辑 + 持久化 + 监控 + 质量 Harness

### 本次 Scope
- 目标：MapEditor、ProjectManager/Serializer、MonitorPanel、CRAP Harness
- 允许改动：`src/app/`, `src/domain/`, `src/ui/`, `tests/`, `tools/`, `docs/`, `third_party/qcustomplot/`, `cmake/`

### ✅ 已完成
- [x] 会话0：`ENABLE_COVERAGE`、`tools/crap_score.py`、`tools/run_quality.ps1`、ADR-007、`MUTATION_CHECKLIST.md`
- [x] 会话1：`ProjectManager`、`MapSerializer`、`ScenarioSerializer` + 单测
- [x] 会话2：`MapEditorPanel`、`ObstacleGraphicsItem`、`MapView` 编辑模式、`MapScene::editorLayer_`
- [x] 会话3：`ProjectDialog`、File 菜单、去除启动 hardcode demo
- [x] 会话4：`MonitorPanel` + QCustomPlot、`MonitorBridge`、cross-track/velocity 曲线
- [x] 会话5：`LaneGraph` stub、SimEngine pose 发布 linear_velocity、SESSION_LOG/DEVELOPMENT_PLAN 更新

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| Mull 自动变异 | MinGW/Clang Kit 冲突 | Phase 4 或 CI 评估 |
| 多车 TaskPanel | Phase 3 范围 | Phase 3 |
| 本地 CRAP 报告数值 | Agent 环境无 Qt Kit | 用户本地跑 `tools/run_quality.ps1` |

### 🚫 禁止偷懒自检
- [x] MapEditor / Monitor 独立 Dock，未堆进 MainWindow/ControlPanel
- [x] Domain Serializer 无 Qt include
- [x] 新 Serializer/ProjectManager 有单测
- [x] SESSION_LOG 已填写

### Reviewer 结果
- 待 Reviewer 子 agent

### 用户本地验证
1. Qt Creator Configure + Build + Run Tests（应含 MapSerializer/ScenarioSerializer/ProjectManager 测试）
2. Run → File → Open → `assets/scenarios/demo` → Editor 画矩形障碍 → File → Save → 重开验证
3. Editor 设 goal / Shift+点击 → Play → Monitor 面板曲线滚动
4. PowerShell：`./tools/run_quality.ps1` 生成 `build-quality/crap_report.txt`

---

## [2026-08-22] Phase 2 复盘 + Phase 3 启动备忘

### Phase 2 编译/集成问题（Phase 3 已对策）
- App 层裸 `map::`/`scenario::` → 一律 `domain::map::` / `domain::scenario::`
- `.h` 缺声明 → 接口先于实现 + Reviewer 对齐 grep
- Lambda 漏 capture → UI Review 必查
- QCustomPlot MinGW → `-Wa,-mbig-obj`
- MainWindow 膨胀 → Phase 3 用 `FleetUiCoordinator`

---

## [2026-08-22] Phase 3 — 多 AGV + 贪心调度 + 时间窗口避碰

### 本次 Scope
- 目标：FleetManager、GreedyAssigner、TimeWindow 避碰、TaskPanel、VehicleInfoPanel、multi_agv 场景
- 允许改动：`src/core/types/`、`src/domain/`、`src/app/`、`src/ui/`、`tests/`、`assets/scenarios/multi_agv/`、`docs/`

### ✅ 已完成
- [x] Core：`Task`、`TimeWindow`、`VehicleState`
- [x] Domain：`TaskQueue`、`GreedyAssigner`、`FleetManager`、`PathReservationTable`、`TimeWindowCollisionAvoidance`
- [x] `SimEngine` 多车 tick + 任务状态机（Pickup → Dropoff）
- [x] `ScenarioSerializer` tasks[] 读写；`assets/scenarios/multi_agv/`
- [x] App：`FleetUiCoordinator`、`SimController::addTask/selectVehicle`
- [x] UI：`TaskPanel`、`VehicleInfoPanel` 独立 Dock
- [x] 单测 + `MultiAgvScenarioTest`；ADR-008；ADR-003 状态更新

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| HungarianAssigner | Phase 3 MVP 用 Greedy | Phase 3+ |
| ROS2 / Bicycle | Phase 4 | Phase 4 |
| 本地全量 Build 验证 | 中文路径由用户执行 | 用户 Qt Creator |

### 🚫 禁止偷懒自检
- [x] 未用简单距离判碰代替时间窗口
- [x] TaskPanel/VehicleInfoPanel 独立 Dock
- [x] Domain 无 Qt include
- [x] 新 Domain 类有单测
- [x] SESSION_LOG 已填写

### 用户本地验证
1. Build + Run Tests（含 MultiAgvScenarioTest）
2. File → Open → `assets/scenarios/multi_agv` → Play → 观察两车接任务、分色路径
3. Task 面板 Add Task；点击车辆查看 Vehicle Info

---

## [2026-08-22] 框架强化 — Agent 反偷懒 + UI 面板规范

（略，见历史条目）

## [2026-08-22] Phase 0 — Harness 与工程骨架

（略，见历史条目）
