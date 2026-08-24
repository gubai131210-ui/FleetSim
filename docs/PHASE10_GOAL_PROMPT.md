# Phase 10 Goal 模式执行提示词（完整版）

> **用途**：新开对话后，整段复制「§0 一键粘贴 Prompt」给 Agent（Goal 模式）。  
> **仓库**：https://github.com/gubai131210-ui/FleetSim.git  
> **本地路径**：`d:\QT_CppPrograms\FleetSim\FleetSim`  
> **前置**：Phase 0–9 ✅（Behavior Tree MVP + CSV + BehaviorTreePanel/BehaviorPage；FleetSimTests **171/171** 绿；`verify_phase9_evidence.py` **85 PASS**）。  
> **本阶段主题**：**OSM/Lanelet2 子集 HD Map 导入** + **BT Nav2 扩展（XML + Spin/BackUp/Clear + RoundRobin/ReactiveFallback）** + **多车独立 BT 黑板隔离** + **CBS-lite 真冲突消解**；与 Phase 5–9 的 Hybrid/MPC/ST/Prediction/LaneGraph/BT JSON **并存、可切换、默认不破回归**。  
> **禁止**：编辑 `.cursor/plans/`；完整 Lanelet2 C++ 库 / 完整 BehaviorTree.CPP / 完整 Autoware / 完整 EECBS 最优大规模；神经网络感知；激光点云真机驱动；Agent 不代跑中文 Temp 破坏性操作；中文路径 Qt Build 由用户验证（Agent 可在 **ASCII** 路径如 `D:\build\FleetSim_phase10_*` 外置构建取证）。

---

## §0 一键粘贴 Prompt（Goal 模式直接用）

```
你是 FleetSim Phase 10 的执行 Agent（Goal 模式）。
仓库：d:\QT_CppPrograms\FleetSim\FleetSim
远程：https://github.com/gubai131210-ui/FleetSim.git
权威说明：docs/PHASE10_GOAL_PROMPT.md（必须先完整阅读再写代码）

【强制阅读顺序 — 未读完禁止写代码】
1. docs/PHASE10_GOAL_PROMPT.md（全文，尤其 §1–§13、§4 算法清单、§5 UI 分层）
2. docs/DEVELOPMENT_PLAN.md、docs/learning-path.md（确认 Phase 9 ✅、Phase 10 目标）
3. docs/AGENT_SESSION_TEMPLATE.md、SESSION_LOG.md（Phase 9 Session 6 终审）
4. docs/decisions/018–021、013-priority-or-cbs-lite.md、002-map-format.md、001-architecture.md
5. docs/UI_GUIDELINES.md + src/ui/panels/README.md + docs/MUTATION_CHECKLIST.md
6. 代码优先：
   domain/map/LaneGraph.* / MapSerializer.* / LaneTypes.h
   domain/behavior/*（BtNavigator、BtTreeLoader、BtControlNodes、BtFleetActionNodes）
   domain/collision/PriorityPathCoordinator.*
   SimEngine.cpp（tick / planPath / behavior_mode / coordination）
   AlgorithmWorkbenchDialog + pages/*（RoutingPage / BehaviorPage / CoordinationPage — 勿乱堆）
   LaneEditorPanel / MapEditorPanel / BehaviorTreePanel
   tools/verify_phase9_evidence.py（仿写 verify_phase10）
   tests/integration/BtNavigationDemoScenarioTest.cpp、LaneRoutingDemoScenarioTest.cpp

【Phase 10 完成定义（全部达成才可结束 Goal）】
A. OsmLaneletImporter（Domain，零 Qt，零 Lanelet2 库依赖）：
   - 解析教学子集 OSM XML：node / way / relation(type=lanelet)
   - 支持 local_x/local_y（或 x/y）本地坐标；禁止假装解析却恒返回空图
   - 从 left/right bound 生成 centerline → 写入既有 LaneMapData / LaneGraph
   - 后继拓扑：共享边界端点 node id 推断 successor（非几何模糊匹配）
   - 单测：合法 OSM → node/edge 非空；坏文件失败；导入后 shortestPath 可用
B. BtXmlLoader（Domain）：Nav2 / BT.CPP 教学子集 XML → 内存树（与 JSON loader 并存）
   - 至少支持：Sequence、Fallback、Recovery、Rate（或 RateController）、RoundRobin、ReactiveFallback
   - Action/Condition 映射到既有 FleetSim 叶节点名（PlanPath、FollowUntilGoal、WaitRecovery、Spin、BackUp、ClearInflation…）
   - 禁止引入 BehaviorTree.CPP 依赖；禁止 XML 解析空壳恒 SUCCESS
C. 运动 Recovery 叶节点（须接真运动学，非空 sleep）：
   - BtSpinRecovery：原地旋转目标角度（DiffDrive / Bicycle 可用 yaw 积分）
   - BtBackUpRecovery：沿车体后方直线倒退指定距离
   - BtClearInflation：教学版 ClearCostmap — 临时清除/重置 inflation 层或标记 replan 前清障碍缓存（ADR 写清语义）
D. 控制节点扩展：
   - BtRoundRobinNode：子节点轮转，任一 SUCCESS 则 SUCCESS；全 FAILURE 才 FAILURE
   - BtReactiveFallbackNode：高优先级条件可打断 RUNNING 子树（对齐 BT.CPP ReactiveFallback 思想）
E. 多车独立 BT：
   - 每启用 BT 的 agent 独立 BtNavigator 实例 + 独立 Blackboard（禁止多车共用同一 blackboard 写 agent_id）
   - scenario 可指定默认树；可选 per-vehicle behavior_tree_path（ADR 定义）
F. CbsLiteCoordinator（真实现，非 stub）：
   - 高层：冲突检测（点/边或时空格）→ 约束树分支
   - 底层：带约束的单车重规划（复用 A* / 既有 planner）
   - 深度/节点数上限（bounded）；coordination=cbs_lite 可切换；默认仍 priority 保回归
   - 禁止「只改距离判碰」或 Priority 换皮冒充 CBS
G. SimEngine / scenario 接线：
   - map 可从 .osm 导入再进 LaneGraph；simulation 字段：map_source、osm_path、bt_format(json|xml)、coordination 含 cbs_lite
   - 字段写了必须被读取；legacy/bt/priority 默认行为不破 Phase 8–9
H. UI 信息架构（硬合同 — 防堆控件，Goal Agent 很懒很笨时尤其要遵守）：
   - 新建 OsmImportPanel（独立 dock）：仅 OSM 路径、预览摘要、导入按钮；禁止塞进 MapEditorPanel / LaneEditorPanel
   - Workbench 新建第 7 页 MapImportPage：仅 map_source / osm 相对路径 / 导入选项（与 RoutingPage、BehaviorPage 分离）
   - Workbench 新建第 8 页 BehaviorXmlPage：仅 bt_format、xml/json 树路径、RoundRobin/Recovery 教学参数；禁止把 Spin/BackUp 运动参数堆进 RoutingPage
   - 扩展既有 CoordinationPage：仅增加 coordination=cbs_lite 与 depth/suboptimality 绑定；禁止新建「把 CBS+BT+OSM 全塞一起」的超级页
   - 新建 MultiAgentBehaviorPanel（独立 dock）：每车 active 节点 / status（只读）；禁止塞进 BehaviorTreePanel 把单车+多车混成一团无分页
   - MainWindow 仅挂载；单 Panel/Page .cpp ≤300 行，超出必须拆 SubWidget
I. scenario：assets/scenarios/osm_lanelet_demo/ + assets/scenarios/cbs_lite_demo/ + 至少 1 份 Nav2 风格 .xml 行为树资产
J. CMake 登记；Domain 零 Qt/rclcpp；保留 target_include_directories / target_link_libraries
K. 文档：ADR-022（OSM 子集导入）、ADR-023（BT XML + 运动 Recovery）、ADR-024（CBS-lite）、ADR-025（多车 BT + UI IA）；DEVELOPMENT_PLAN Phase10✅；SESSION_LOG；MUTATION M43+
L. 测试：OsmLaneletImporterTest、BtXmlLoaderTest、BtMotionRecoveryTest、CbsLiteCoordinatorTest、MultiBtNavigationTest、集成 demo 测；Phase 7–9 回归；
   ASCII Build + FleetSimTests 全绿 + verify_phase10_evidence.py（≥60 PASS）才可 complete
M. 每会话 commit + push；四角色互相监督 PASS；回复四段式

【本阶段必须实现的算法清单 — 禁止只做 UI / 禁止 stub 冒充】
1. OSM XML 子集解析（node/way/relation）
2. Lanelet centerline 生成（left/right 中点或等价）
3. 共享 node id 后继/邻接推断
4. BT XML 子集加载器
5. RoundRobin 控制节点
6. ReactiveFallback 控制节点
7. Spin 运动 recovery
8. BackUp 运动 recovery
9. ClearInflation（教学 ClearCostmap）
10. 每车独立 BT + Blackboard 隔离
11. CBS-lite（约束树 + 底层重规划 + 深度上限）
以上 11 项全部有 Domain 实现 + GTest；缺一不可标 Phase10 ✅

【架构硬约束】
UI → App → Domain → Core
Domain/Core 禁止 #include <Qt*> 与 rclcpp
App 层一律 domain:: 前缀命名空间
接口先于实现：先 .h 后 .cpp
新 UI = 新 Panel/Dialog + 独立 Page；MainWindow 仅挂载
禁止编辑 .cursor/plans/
中文路径：ProjectManager 测试用仓库内 ASCII test_tmp

【四角色子 Agent 团队 — 每会话强制】
1. Planner（计划）：mini-plan — 允许改动 / NOT DO / 文件清单 / 测试清单 / UI 分页验收（必须点名新建哪些 Page/Panel）
2. Executor（执行）：只按 mini-plan 写代码与 CMake；禁止扩 scope；禁止「先做 UI 后补算法」
3. Tester（测试）：写/跑/补 GTest（ASCII D:\build\FleetSim_phase10_*）；空壳算法 → FAIL；UI 堆控件 → FAIL
4. Reviewer（检查）：对照 §11 + UI_GUIDELINES + Phase2–9 防回归；输出 PASS/FAIL
流程：Planner → Executor → Tester → Reviewer。主 Agent 禁止自评 PASS。SESSION_LOG 须引用四角色结论。

【建议 8 会话，可连续但禁止跳验收】
0: ADR-022…025 草案 + 红灯测骨架（Osm* / BtXml* / CbsLite* 预期 FAIL）
1: OsmLaneletImporter + centerline + topology → LaneGraph + OsmLaneletImporterTest 绿
2: BtXmlLoader + RoundRobin + ReactiveFallback + BtXmlLoaderTest 绿
3: Spin / BackUp / ClearInflation 叶节点 + BtMotionRecoveryTest 绿
4: Multi-agent BT 隔离 + SimEngine 接线 + MultiBtNavigationTest
5: CbsLiteCoordinator 真实现 + CbsLiteCoordinatorTest + coordination 切换
6: UI — OsmImportPanel + MapImportPage + BehaviorXmlPage + MultiAgentBehaviorPanel（严禁堆页）
7: demo 场景 + Phase7–9 回归 + MUTATION M43+ + verify_phase10 + 四角色终审 + Phase10 ✅

【禁止偷懒 — 执行 Agent 必查（Reviewer 逐条打勾）】
1. 禁止 Osm 导入器返回恒空 LaneMap 却标 Phase10 ✅
2. 禁止 BtXmlLoader 忽略子节点顺序 / 所有节点 tick 恒 SUCCESS
3. 禁止 Spin/BackUp 只用 WaitRecovery 换皮（无姿态变化）
4. 禁止 CBS-lite = Priority 换名或只做距离减速
5. 禁止多车共用一个 Blackboard 导致 agent_id 互相覆盖
6. 禁止把 OSM 导入控件堆进 MapEditorPanel 或 LaneEditorPanel
7. 禁止把 XML/BT/CBS/OSM 控件堆进同一个 Workbench 页或同一个 Panel
8. 禁止在 BehaviorPage（Tab 6）继续无限加控件 — XML 相关必须 BehaviorXmlPage（Tab 8）
9. 禁止 Domain 写 Qt / rclcpp
10. 禁止新 Domain 类无 GTest
11. 禁止削 CMake target_*
12. 禁止破坏 Phase9 BT JSON / Phase8 hybrid / Phase7 prediction 回归
13. 禁止引入完整 Lanelet2 / BehaviorTree.CPP / Autoware / 神经网络感知
14. 禁止 SESSION_LOG 缺四角色 / 缺「没做什么」
15. 禁止未更新 CMakeLists 就加源文件
16. 禁止 Phase10 ✅ 但 11 项算法清单有任何一项缺失
17. 禁止 ClearInflation 空函数却声称已实现 ClearCostmap
18. 禁止 ReactiveFallback / RoundRobin 与 Sequence 同一实现却改名

从会话 0 开始立刻执行。中间不要换模型。
```

---

## §1 Phase 10 目标与验收标准

### 1.1 为何是 Phase 10（与学习路径对齐）

| 已完成（Phase 9） | Phase 10 要补的缺口 | 自动驾驶对应 |
|------------------|-------------------|--------------|
| 手工 `lanes[]` JSON | **OSM/Lanelet2 子集导入** → 同一 LaneGraph | Autoware Lanelet2 / HD Map |
| BT 仅 JSON schema | **BT XML 子集**（Nav2 风格） | BehaviorTree.CPP / Nav2 BT XML |
| Recovery 仅 WaitRecovery | **Spin / BackUp / ClearInflation** | Nav2 Behavior Server |
| 单车 / 共享导航树 | **每车独立 BT + 黑板隔离** | 多机器人编排 |
| Priority 协调 | **CBS-lite 约束树**（bounded） | CBS / ECBS / EECBS（教学有界） |
| Workbench 6 页 | **MapImport + BehaviorXml 新页** + OsmImport / MultiAgent BT dock | 地图/行为/协调分层 UI |

Phase 9 解决「**何时** plan/follow/replan」；Phase 10 解决「**地图从哪来**、**树如何用工业 XML 描述**、**失败时如何运动恢复**、**多车如何约束消解**」。

### 1.2 验收清单（全部勾选 = Phase 10 完成）

- [ ] `OsmLaneletImporter`：`.osm` → `LaneMapData`；centerline + successor；单测非空图
- [ ] `BtXmlLoader`：至少加载一份 Nav2 风格 XML 树并 tick；与 JSON 树可切换
- [ ] `RoundRobin` + `ReactiveFallback` 单测覆盖语义（与 Sequence/Fallback 可区分）
- [ ] `BtSpinRecovery` / `BtBackUpRecovery`：仿真后 pose.yaw 或位置发生可断言变化
- [ ] `BtClearInflation`：有可测副作用（inflation 清除或 cost 变化或 replan 成功路径）
- [ ] 多车 BT：≥2 agent 独立 blackboard；互不覆盖 `agent_id`
- [ ] `CbsLiteCoordinator`：人为制造冲突时产生约束并重规划；`coordination=cbs_lite` 可切换
- [ ] **OsmImportPanel**、**MapImportPage**、**BehaviorXmlPage**、**MultiAgentBehaviorPanel** 均存在且未污染 MapEditor/LaneEditor/Routing/Behavior(Tab6)
- [ ] demo：`osm_lanelet_demo`、`cbs_lite_demo`；Phase 8–9 demo 仍可加载
- [ ] ADR-022…025；DEVELOPMENT_PLAN Phase 10 ✅；MUTATION M43+；`verify_phase10_evidence.py` PASS
- [ ] GTest 全绿（≥171 + 新增测，预期 ~190+）

### 1.3 明确不做（Phase 10 范围外 → Phase 11+）

| 项目 | 原因 |
|------|------|
| 完整 Lanelet2 C++ 库 / GeographicLib 全投影栈 | 依赖爆炸；本阶段自研 OSM 子集解析 |
| 完整 BehaviorTree.CPP / Groot2 | 保持零第三方 BT 依赖；自研 XML 子集 |
| 完整 EECBS（EES + 全部 symmetry reasoning） | 教学用 depth-bounded CBS-lite；EECBS 可 Phase 11 |
| 激光雷达/相机真感知 / 神经网络预测 | 明确另阶段 |
| 交通灯 RegulatoryElement 全语义 / 停车位 Area | OSM 子集只做可行驶 lanelet |
| 非线性大规模 MPC / 完整 Autoware Universe | 超出教学仿真范围 |

---

## §2 当前框架结构（必须保持）

```
UI (Qt) → App (SimController / ProjectManager / MonitorBridge / FleetUiCoordinator)
        → Domain (
              SimEngine + planning/map/scheduling/collision/prediction/experiment/behavior
              + map/OsmLaneletImporter   ← Phase 10
              + behavior/BtXmlLoader + motion recovery + RoundRobin/ReactiveFallback ← Phase 10
              + collision/CbsLiteCoordinator ← Phase 10
           )
        → Core (Pose / Path / SpeedProfile / EventBus / SimClock)

CMake:
FleetSimCore → FleetSimDomain → FleetSimApp → FleetSimUI → FleetSim
FleetSimTests → App + Domain + Core + GTest
```

**Phase 10 增量（示意）：**

```
domain/map/OsmLaneletImporter.*     OSM XML → LaneMapData
domain/map/OsmTypes.h               解析中间结构（可选）
domain/behavior/BtXmlLoader.*       XML → BtNode 树
domain/behavior/BtRoundRobin.*      或并入 BtControlNodes
domain/behavior/BtReactiveFallback  或并入 BtControlNodes
domain/behavior/BtMotionRecoveryNodes.*  Spin / BackUp / ClearInflation
domain/behavior/MultiBtNavigator.*  每 agent 导航器注册表
domain/collision/CbsLiteCoordinator.*
扩展 SimEngine.*                    osm 加载、xml 树、cbs_lite、多车 BT tick
扩展 scenario/*                     map_source、osm_path、bt_format、cbs 参数
ui/panels/OsmImportPanel.*
ui/panels/MultiAgentBehaviorPanel.*
ui/dialogs/pages/MapImportPage.*    Workbench Tab 7
ui/dialogs/pages/BehaviorXmlPage.*  Workbench Tab 8
assets/maps/*.osm（教学小地图）
assets/behavior_trees/*.xml
assets/scenarios/osm_lanelet_demo/
assets/scenarios/cbs_lite_demo/
```

**数据流（示意）：**

```
.osm ──OsmLaneletImporter──► LaneMapData ──► LaneGraph ──► LaneRouter / hybrid planPath
                                                      │
BT .xml ──BtXmlLoader──► BtNavigator(per agent) ──tick──► Spin/BackUp/Clear/Plan/Follow
                                                      │
CbsLiteCoordinator ◄── conflicts(agent paths) ──► constrain + replan (bounded depth)
```

---

## §3 优先阅读的程序（按优先级）

### P0 — 动手前必读

| 文件 | 为什么 |
|------|--------|
| `src/domain/map/LaneGraph.*` / `LaneTypes.h` | OSM 导入的目标数据结构 |
| `src/domain/map/MapSerializer.*` | lanes 序列化；导入后应能 round-trip 或另存 |
| `src/domain/behavior/BtTreeLoader.*` | JSON 加载先例；XML 应对齐节点工厂 |
| `src/domain/behavior/BtControlNodes.*` | Recovery/Sequence/Fallback — 扩展 RoundRobin/ReactiveFallback |
| `src/domain/behavior/BtFleetActionNodes.*` | 叶节点风格；Spin/BackUp 仿此接线 |
| `src/domain/collision/PriorityPathCoordinator.*` | CBS-lite 对照；禁止换皮 |
| `src/domain/SimEngine.cpp` | tick / planPath / behavior_mode / coordination 注入点 |
| `src/ui/dialogs/pages/BehaviorPage.*` | **勿再堆** — XML 去 Tab 8 |
| `src/ui/dialogs/pages/RoutingPage.*` / `CoordinationPage.*` | 分层先例 |
| `src/ui/panels/LaneEditorPanel.*` / `MapEditorPanel.*` | **勿塞 OSM 导入** |
| `tools/verify_phase9_evidence.py` | 仿写 verify_phase10 |

### P1 — 场景与 App

| 文件 | 为什么 |
|------|--------|
| `src/app/SimController.cpp` | applyScenario；解析 osm/xml 路径 |
| `src/ui/MainWindow.cpp` | 仅挂载新 dock |
| `src/domain/scenario/ScenarioLoader.h` / `ScenarioSerializer.cpp` | 新字段 |
| `assets/scenarios/bt_navigation_demo/` | Phase 9 回归 |
| `assets/scenarios/lane_routing_demo/` | Phase 8 回归 |

### P2 — 防回归

| ID | 对策 |
|----|------|
| P9-BT-JSON | BehaviorTreeTest / BtNavigation* / navigate_replan_recovery.json 仍绿 |
| P9-UI | BehaviorPage + BehaviorTreePanel 独立；CSV 仍仅 ComparePanel |
| P8-Lane | LaneGraph / hybrid / LaneEditor 仍绿 |
| P7-Pred | prediction_st_demo / ExperimentCompare 仍绿 |
| P5-Priority | `coordination=priority` 默认仍可用 |
| P4-TEMP | test_tmp ASCII |

---

## §4 网络调研摘要（执行 Agent 必读 — 算法必须按此实现）

### 4.1 Lanelet2 / OSM：教学子集（禁止引入完整 Lanelet2 库）

工业 Autoware 使用扩展 Lanelet2 OSM（`.osm`）：`node` / `way` / `relation`，lanelet 为 `relation` + `tag type=lanelet`，成员 role=`left`/`right`（可选 `centerline`）。本地仿真常用 `local_x`/`local_y`（及 `ele`）代替经纬度。

**连通性关键事实**：Lanelet2 **不**显式存储 successor 表；后继由 **边界端点共享同一 OSM node id** 推断；换道邻接由 **共享 boundary way** 推断。禁止用「距离很近就算连通」糊弄。

**FleetSim Phase 10 选择**：

| Lanelet2 / Autoware | FleetSim MVP |
|---------------------|--------------|
| lanelet2 库 + 投影 | 自研 `OsmLaneletImporter`（tinyxml2 / pugixml / 手写 XML，任选其一，Domain 内） |
| left/right ways | 中点插值生成 centerline → `LaneNode`/`LaneEdge` |
| successor 隐式 | 端点 node id 匹配建边 |
| regulatory_element | **不做** |
| 完整地理投影 | **不做**；仅 local_x/y 或平面 x/y |

参考：
- [Lanelet2 maps / OSM mapping](https://github.com/fzi-forschungszentrum-informatik/Lanelet2/blob/master/lanelet2_maps/README.md)
- [Autoware Lanelet2 format extension](https://tier4.github.io/autoware.iv/tree/main/map/lanelet2_extension/docs/lanelet2_format_extension/)
- [Converting Lanelet2 to routing graphs](https://www.vehiclemapping.org/lane-geometry-extraction-road-network-processing/road-network-graph-construction/converting-lanelet2-maps-to-routing-graphs/)

### 4.2 Nav2 BT XML + Behavior Server（Spin / BackUp / Clear / RoundRobin）

Nav2 默认导航树使用 `RecoveryNode` + `PipelineSequence` + `RateController` + 系统级 `RoundRobin(Clear, Spin, Wait, BackUp)`。  
Spin：原地旋转；BackUp：沿 heading 反向平移；ClearEntireCostmap：清局部/全局 costmap。

**FleetSim Phase 10 映射**：

| Nav2 | FleetSim |
|------|----------|
| BT XML `<root><BehaviorTree>…` | `BtXmlLoader`（子集） |
| RoundRobin | `BtRoundRobinNode` |
| ReactiveFallback | `BtReactiveFallbackNode` |
| Spin | `BtSpinRecovery`（积分 yaw） |
| BackUp | `BtBackUpRecovery`（负向线速度积分位移） |
| ClearEntireCostmap | `BtClearInflation`（清 inflation 或临时可通行标记，ADR 写死） |
| Wait | 已有 `BtWaitRecovery` |
| PipelineSequence | 可用 Sequence 近似，或实现「已成功子节点在 RUNNING 时重 tick」的 PipelineSequence（推荐实现，避免与 Sequence 不可区分） |

参考：
- [Nav2 BT XML nodes](https://docs.nav2.org/configuration/packages/configuring-bt-xml.html)
- [Nav2 specific nodes](https://docs.nav2.org/behavior_trees/overview/nav2_specific_nodes.html)
- [BackUp plugin](https://docs.nav2.org/configuration/packages/bt-plugins/actions/BackUp.html)
- [BehaviorTree.CPP XML format](https://www.behaviortree.dev/docs/learn-the-basics/xml_format/)

### 4.3 ReactiveFallback vs Fallback（必须可测区分）

- **Fallback**：子 RUNNING 时继续 tick **同一**子节点。  
- **ReactiveFallback**：每 tick 从第一个子节点重新评估，条件 SUCCESS 可 **打断** 后续异步子节点。  

单测必须证明：条件从 FAILURE→SUCCESS 时，正在 RUNNING 的动作被中止（或不再继续），与普通 Fallback 行为不同。禁止两个类同一实现改名。

### 4.4 CBS-lite（教学有界；非完整 EECBS）

**CBS**（Conflict-Based Search，Sharon et al. 2015）：高层约束树处理冲突；底层单智能体寻路。  
**ECBS/EECBS**：有界次优加速（focal / EES）；完整 EECBS + symmetry reasoning 超出本阶段。

**FleetSim Phase 10 `CbsLiteCoordinator` 最低真实现**：

1. 检测多智能体路径的点冲突或边冲突（时空格或 discretized time）。  
2. 生成约束（agent_i 禁止在时刻 t 占格 c）。  
3. 对冲突 agent 在约束下重跑底层 A*（或现有 planner）。  
4. 限制高层扩展深度 / 节点数；超时返回失败或回退 priority。  
5. `coordination=cbs_lite` 时走此路径；默认 `priority` 不变。

参考：
- Sharon et al., CBS, Artificial Intelligence 2015  
- [EECBS (AAAI 2021)](https://ojs.aaai.org/index.php/AAAI/article/view/17466) — **阅读思想，不要求完整移植**  
- [Jiaoyang-Li/EECBS](https://github.com/Jiaoyang-Li/EECBS) — 对照概念用

### 4.5 UI 分层启示（防 Goal Agent 堆控件 — **本章最重要**）

| 层次 | 用户心智 | FleetSim 落点 | 禁止 |
|------|---------|--------------|------|
| 障碍几何 | 矩形/多边形障碍 | MapEditorPanel | 塞 OSM / BT / CBS |
| 手工车道 | 节点边 CRUD | LaneEditorPanel | 塞 OSM 导入表单 |
| **HD Map 导入** | 从 .osm 生成车道 | **OsmImportPanel** + **MapImportPage** | 塞进 Map/Lane 编辑器 |
| 路由策略 | freespace/lane/hybrid | RoutingPage | 塞 XML/CBS |
| 行为模式（JSON 时代） | legacy/bt + 基本 hz | BehaviorPage（Tab 6） | 继续堆 XML/Spin 参数 |
| **行为 XML / 运动恢复参数** | bt_format、xml 路径、spin/backup 距 | **BehaviorXmlPage（Tab 8）** | 塞进 BehaviorPage/Routing |
| **多车 BT 监控** | 每车 active 节点 | **MultiAgentBehaviorPanel** | 与单车 BehaviorTreePanel 无结构混堆 |
| 协调策略 | priority / cbs_lite | **CoordinationPage**（扩展） | 新建超级「算法全家桶」页 |
| 实验对比 | 指标 + CSV | ExperimentComparePanel | 加 OSM/BT |

**Reviewer 量化红线**：

- `MapEditorPanel.cpp` / `LaneEditorPanel.cpp` 出现 `osm` / `OsmImport` / `.osm` 文件选择 → **FAIL**  
- `BehaviorPage.cpp` 出现 `bt_format` / `RoundRobin` / `spin_rad` / `backup_dist` → **FAIL**（应在 BehaviorXmlPage）  
- `RoutingPage.cpp` 出现 `cbs_lite` / `osm_path` → **FAIL**  
- `CoordinationPage.cpp` 出现 `behavior_tree` / `osm` → **FAIL**  
- 任一新 Panel/Page `.cpp` >300 行未拆分 → **FAIL**  
- MainWindow 新增业务逻辑 >15 行/功能 → **FAIL**

---

## §5 设计决策建议（先写 ADR 再写码）

### ADR-022 — OSM/Lanelet2 教学子集导入

```json
"simulation": {
  "map_source": "json",
  "osm_path": "map.osm"
}
```

- `map_source`: `json`（默认，Phase 8 行为）| `osm`  
- 导入结果写入内存 `LaneMapData`，可选另存为 `lanes` JSON  
- 坐标系：优先 `local_x`/`local_y`，否则 `x`/`y` 属性  

### ADR-023 — BT XML + 运动 Recovery

- `bt_format`: `json` | `xml`（默认 `json` 保 Phase 9）  
- XML 根：`<root main_tree_to_execute="…">` + `<BehaviorTree ID="…">`  
- Spin：`spin_rad`（默认 ±π/2）；BackUp：`backup_dist_m`（默认 0.3）、`backup_speed_mps`  
- ClearInflation：清除 inflation 半径影响或重置栅格 inflation 层（实现选一种，测例锁死）  

### ADR-024 — CBS-lite

- `coordination`: `none` | `priority` | `cbs_lite`（默认 `priority`）  
- `cbs_max_depth`、`cbs_time_limit_ms`  
- 与 TimeWindow/Reservation：**协同或替代边界写清**；禁止静默删掉 Phase 3 避碰却无测  

### ADR-025 — 多车 BT + UI IA

- 每 agent：`BtNavigator` + `BtBlackboard`  
- UI：OsmImportPanel / MapImportPage / BehaviorXmlPage / MultiAgentBehaviorPanel 职责表（§4.5）  

---

## §6 建议新增/变更文件清单

### Domain

| 文件 | 职责 |
|------|------|
| `map/OsmLaneletImporter.*` | OSM → LaneMapData |
| `behavior/BtXmlLoader.*` | XML → 树 |
| `behavior/BtControlNodes.*` | + RoundRobin、ReactiveFallback（± PipelineSequence） |
| `behavior/BtMotionRecoveryNodes.*` | Spin、BackUp、ClearInflation |
| `behavior/MultiBtNavigator.*` | agent_id → navigator |
| `collision/CbsLiteCoordinator.*` | CBS-lite |
| 扩展 `SimEngine.*` / `scenario/*` | 字段与 tick 分支 |

### UI

| 文件 | 职责 |
|------|------|
| `panels/OsmImportPanel.*` | OSM 导入 dock |
| `panels/MultiAgentBehaviorPanel.*` | 多车 BT 只读监控 |
| `dialogs/pages/MapImportPage.*` | Workbench Tab 7 |
| `dialogs/pages/BehaviorXmlPage.*` | Workbench Tab 8 |
| 扩展 `CoordinationPage.*` | cbs_lite 选项 |
| 扩展 `AlgorithmWorkbenchDialog.*` | Tab 7–8 |

### 测试 / 文档 / 工具 / 资产

```
tests/domain/OsmLaneletImporterTest.cpp
tests/domain/BtXmlLoaderTest.cpp
tests/domain/BtMotionRecoveryTest.cpp
tests/domain/CbsLiteCoordinatorTest.cpp
tests/integration/MultiBtNavigationTest.cpp
tests/integration/OsmLaneletDemoScenarioTest.cpp
tests/integration/CbsLiteDemoScenarioTest.cpp
docs/decisions/022-osm-lanelet-import.md
docs/decisions/023-bt-xml-motion-recovery.md
docs/decisions/024-cbs-lite.md
docs/decisions/025-multi-bt-ui-ia.md
tools/verify_phase10_evidence.py
tools/run_phase10_verify.ps1
assets/maps/teaching_lanelet_subset.osm
assets/behavior_trees/navigate_spin_backup_recovery.xml
assets/scenarios/osm_lanelet_demo/
assets/scenarios/cbs_lite_demo/
```

---

## §7 建议 8 会话任务拆解

| Session | 目标 | 关键验收 | NOT DO |
|---------|------|----------|--------|
| 0 | ADR-022…025 + 红灯测 | 测例编译但 FAIL | 不写假 PASS 实现 |
| 1 | Osm 导入 → LaneGraph | OsmLaneletImporterTest 绿；shortestPath | 不引 Lanelet2 库；不改 UI |
| 2 | BtXmlLoader + RoundRobin + ReactiveFallback | BtXmlLoaderTest；语义可区分 | 不引 BT.CPP |
| 3 | Spin/BackUp/ClearInflation | pose/栅格断言变化 | Wait 换皮 |
| 4 | Multi BT | MultiBtNavigationTest | 共享 blackboard |
| 5 | CbsLite | CbsLiteCoordinatorTest；冲突可消 | Priority 换名 |
| 6 | UI 四件套 | 负向审计：MapEditor/LaneEditor/BehaviorPage 无新污染 | 单页堆控件 |
| 7 | demo + verify + M43+ + Phase10 ✅ | verify ≥60；全绿 | 跳回归 |

---

## §8 测试要求（Tester 强制）

| 测试 | 最低断言 |
|------|----------|
| `OsmLaneletImporterTest` | 合法 OSM → nodes≥2、edges≥1；坏 XML 失败；导入后 `LaneGraph::shortestPath` 有值 |
| `BtXmlLoaderTest` | 加载 `navigate_spin_backup_recovery.xml`；未知标签失败；RoundRobin 轮转顺序可测 |
| `BtMotionRecoveryTest` | Spin 后 `|Δyaw|≥ε`；BackUp 后位移沿后方 ≥ε；Clear 后 inflation 占用减少或 plan 成功 |
| `CbsLiteCoordinatorTest` | 两车对撞场景：cbs_lite 产生不同路径或约束次数>0；depth=0 立即失败/回退 |
| `MultiBtNavigationTest` | 两车 blackboard `agent_id` 不同且 tick 互不覆盖 |
| `OsmLaneletDemoScenarioTest` | 场景可加载；lane_graph/hybrid 可规划 |
| `CbsLiteDemoScenarioTest` | coordination=cbs_lite 可跑通 |
| 回归 | Phase9 Behavior* / BtNavigation*；Phase8 Lane*；Phase7 ExperimentCompare* |

**假实现检测**：

- Osm 恒空图 → FAIL  
- Spin 后 yaw 不变 → FAIL  
- CBS 与 Priority 输出路径位码相同且无约束节点 → FAIL（Tester 构造冲突场景）  
- ReactiveFallback 与 Fallback 测例结果完全相同且实现指针相同 → FAIL  

---

## §9 CMake / 构建

- Domain 新增源必须进 `src/domain/CMakeLists.txt`  
- UI 进 `src/ui/CMakeLists.txt`  
- Tests 进 `tests/CMakeLists.txt`  
- ASCII 外置：`D:\build\FleetSim_phase10_s0`  
- 用户中文路径 Qt：Agent **不**代跑；交付验证步骤  

---

## §10 文档与变异

| 交付 | 说明 |
|------|------|
| ADR-022…025 | Session 0 草案 → Session 7 已接受 |
| DEVELOPMENT_PLAN | Phase **10** ✅ |
| learning-path.md | 更新 Phase 10 行 |
| MUTATION M43–M46 | 见 §19 |
| verify_phase10_evidence.py | ≥60 静态检查 |
| SESSION_LOG | 每会话四角色 |

---

## §11 Reviewer 检查单（每会话）

### 算法真实性
- [ ] 11 项算法清单均有实现文件与 TEST  
- [ ] Osm 后继用 node id，非距离糊弄  
- [ ] Spin/BackUp 改变 pose  
- [ ] CBS 有约束树或等价可观测冲突消解  

### UI
- [ ] OsmImport ≠ MapEditor ≠ LaneEditor  
- [ ] BehaviorXmlPage ≠ BehaviorPage  
- [ ] MultiAgentBehaviorPanel ≠ BehaviorTreePanel 职责混淆  
- [ ] CoordinationPage 仅协调参数  
- [ ] 无单页控件堆叠  

### 回归 / 工程
- [ ] Domain 零 Qt  
- [ ] CMake 完整  
- [ ] Phase 7–9 测绿  
- [ ] SESSION_LOG 完整  

---

## §12 用户本地验证清单（每会话末尾交给用户）

1. `git pull origin main`  
2. ASCII：`cmake` + Build `FleetSimTests` + 运行  
3. `python tools/verify_phase10_evidence.py`（Session 7 起）  
4. 打开 `osm_lanelet_demo` → 确认 lane 显示与规划  
5. 打开 XML 行为树场景 → BehaviorTree / MultiAgent 面板有状态；触发 Spin/BackUp 可见车动  
6. `cbs_lite_demo` → 两车无长时间卡死对撞  
7. 回归：`bt_navigation_demo`、`lane_routing_demo`、`prediction_st_demo`  

---

## §13 回复用户四段式（强制）

1. **做了什么**  
2. **没做什么**（含原因）  
3. **四角色结论**（Planner / Executor / Tester / Reviewer）  
4. **请你本地验证**（步骤列表）  

---

## §14 风险与缓释

| 风险 | 缓释 |
|------|------|
| OSM 方言过多 | 只支持教学子集；资产仓库内自备 `.osm` |
| XML 标签爆炸 | 白名单节点名；未知标签加载失败 |
| CBS 组合爆炸 | max_depth + time_limit；失败回退 priority |
| UI Agent 堆控件 | §4.5 红线 + verify 负向检查 |
| 中文路径 Build | 用户本地；Agent ASCII |

---

## §15 Goal 模式启动短指令（可选第二段）

```
执行 docs/PHASE10_GOAL_PROMPT.md。
Goal：完成 Phase 10（OSM 子集导入 + BT XML/运动 Recovery + 多车 BT + CBS-lite + UI 分层）。
强制四角色子 Agent；11 项算法全部真实现 + GTest；verify_phase10 全绿；每会话 push。
从会话 0 开始。禁止堆 UI。禁止 stub。
```

---

## §16 Planner mini-plan 模板（每会话必填）

```markdown
**Phase**: 10
**本会话目标**: （一句话）
**允许改动**: （目录/文件）
**NOT DO**: （≥8 条，含禁止堆 UI / 假 Osm / Wait 冒充 Spin / Priority 冒充 CBS / 破坏 Phase9）
**预计新增/修改文件**:
**计划测试**（精确 TEST 名）:
**UI 分页验收**:
  - 必须新建/修改: （点名 Panel/Page）
  - 禁止污染: MapEditorPanel / LaneEditorPanel / BehaviorPage / RoutingPage / MonitorPanel
**验收标准**:
**四角色**: Planner / Executor / Tester / Reviewer 结论占位
```

---

## §17 Phase 11 展望（勿在 Phase 10 实现）

- 完整 EECBS（EES + bypass/symmetry）  
- 激光射线投射占用感知 / 动态障碍感知层  
- 多模态轨迹预测（非 CV）  
- Groot2 / 完整 BT.CPP 插件生态  
- Lanelet2 regulatory / 交通灯全语义  
- 非线性 MPC / Autoware 规划栈对接  

---

## §18 verify_phase10_evidence.py 检查项（Session 7 必须实现）

静态脚本至少覆盖（仿 Phase 9，目标 **≥60 PASS**）：

| 分组 | 检查项 |
|------|--------|
| A Osm | OsmLaneletImporter.*、OsmLaneletImporterTest、teaching `.osm` 资产 |
| B BT XML | BtXmlLoader.*、RoundRobin/ReactiveFallback、`.xml` 资产 |
| C Motion | Spin/BackUp/ClearInflation 源与 BtMotionRecoveryTest |
| D Multi BT | MultiBtNavigator 或等价；MultiBtNavigationTest |
| E CBS | CbsLiteCoordinator.*、CbsLiteCoordinatorTest、coordination cbs_lite |
| F UI IA | OsmImportPanel、MapImportPage、BehaviorXmlPage、MultiAgentBehaviorPanel |
| G 负向 | MapEditor/LaneEditor 无 osm 导入；BehaviorPage 无 spin/xml format；RoutingPage 无 cbs |
| H demo | osm_lanelet_demo、cbs_lite_demo |
| I CMake | 新源登记；Domain 无 Qt |
| J ADR | 022–025、M43+、Phase 10 marked |
| K 回归 | Phase9 verify 关键资产仍在；bt_navigation_demo / lane_routing_demo |
| L 工具 | verify_phase10_evidence.py、run_phase10_verify.ps1 |

---

## §19 MUTATION M43–M46（Session 7 登记）

| ID | 注入点 | 预期 FAIL 测例 |
|----|--------|----------------|
| M43 | `OsmLaneletImporter` — 丢弃所有 relation 或 centerline 恒空 | `OsmLaneletImporterTest.*` |
| M44 | `BtSpinRecovery` — 不修改 yaw | `BtMotionRecoveryTest.Spin*` |
| M45 | `BtRoundRobin` — 恒 tick 第一个 child | `BtXmlLoaderTest` / RoundRobin 专用测 |
| M46 | `CbsLiteCoordinator` — 忽略冲突恒返回原路径 | `CbsLiteCoordinatorTest.*` |

---

## §20 算法实现对照表（Reviewer 终审用）

| # | 算法 | 实现类/文件 | 单测 | 完成 |
|---|------|------------|------|------|
| 1 | OSM XML 子集解析 | OsmLaneletImporter | OsmLaneletImporterTest | ☐ |
| 2 | Centerline 生成 | OsmLaneletImporter | 同上 | ☐ |
| 3 | 共享 node 后继推断 | OsmLaneletImporter | 同上 | ☐ |
| 4 | BT XML 加载 | BtXmlLoader | BtXmlLoaderTest | ☐ |
| 5 | RoundRobin | BtRoundRobinNode | BtXmlLoaderTest / BehaviorTreeTest | ☐ |
| 6 | ReactiveFallback | BtReactiveFallbackNode | BehaviorTreeTest | ☐ |
| 7 | Spin recovery | BtSpinRecovery | BtMotionRecoveryTest | ☐ |
| 8 | BackUp recovery | BtBackUpRecovery | BtMotionRecoveryTest | ☐ |
| 9 | ClearInflation | BtClearInflation | BtMotionRecoveryTest | ☐ |
| 10 | 多车 BT 隔离 | MultiBtNavigator | MultiBtNavigationTest | ☐ |
| 11 | CBS-lite | CbsLiteCoordinator | CbsLiteCoordinatorTest | ☐ |

**全部勾选 + verify PASS + FleetSimTests 全绿 = 才允许 UpdateGoal complete。**

---

*文档版本：Phase 10 · 2026-08-24 · 前置 Phase 9 Session 6（171/171 绿，verify_phase9 85 PASS）*  
*调研来源：Lanelet2 OSM 映射、Autoware lanelet2_extension、Nav2 BT XML / Behavior Server、BehaviorTree.CPP XML / ReactiveFallback、CBS/EECBS（AAAI 2021）*
