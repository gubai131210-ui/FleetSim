# Phase 9 Goal 模式执行提示词（完整版）

> **用途**：新开对话后，整段复制「§0 一键粘贴 Prompt」给 Agent（Goal 模式）。  
> **仓库**：https://github.com/gubai131210-ui/FleetSim.git  
> **本地路径**：`d:\QT_CppPrograms\FleetSim\FleetSim`  
> **前置**：Phase 0–8 ✅（LaneGraph + hybrid first/last mile + LaneEditorPanel + RoutingPage；FleetSimTests **139/139** 绿；`verify_phase8_evidence.py` **55 PASS**）。  
> **本阶段主题**：**轻量 Behavior Tree 教学 MVP**（plan / follow / replan / recovery / yield）+ **ExperimentMetrics CSV 导出** + **行为层 UI 再分层**；与 Phase 5–8 的 planner/tracker/ST/prediction/routing **并存、可切换、默认 legacy 不破回归**。  
> **禁止**：编辑 `.cursor/plans/`；完整 Nav2 / BehaviorTree.CPP 依赖栈；完整 Lanelet2/OSM 导入；神经网络感知；Agent 不代跑中文 Temp 破坏性操作；中文路径 Qt Build 由用户验证（Agent 可在 **ASCII** 路径如 `D:\build\FleetSim_phase9_*` 外置构建取证）。

---

## §0 一键粘贴 Prompt（Goal 模式直接用）

```
你是 FleetSim Phase 9 的执行 Agent（Goal 模式）。
仓库：d:\QT_CppPrograms\FleetSim\FleetSim
远程：https://github.com/gubai131210-ui/FleetSim.git
权威说明：docs/PHASE9_GOAL_PROMPT.md（必须先完整阅读再写代码）

【强制阅读顺序 — 未读完禁止写代码】
1. docs/PHASE9_GOAL_PROMPT.md（全文，尤其 §1–§13）
2. docs/DEVELOPMENT_PLAN.md、docs/learning-path.md（确认 Phase 8 ✅、Phase 9 目标）
3. docs/AGENT_SESSION_TEMPLATE.md、SESSION_LOG.md（Phase 8 Session 8 终审）
4. docs/decisions/018-lane-graph-routing.md、019-lane-editor-ui.md、017-algorithm-workbench-ui.md、001-architecture.md
5. docs/UI_GUIDELINES.md + src/ui/panels/README.md + docs/MUTATION_CHECKLIST.md
6. 代码优先：
   SimEngine.cpp（tick / planPath / handleAgentGoalReached / needs_replan）
   FleetManager.cpp（AgentPhase 状态机 — BT 将部分替代/包装）
   domain/planning/* + domain/map/LaneGraph.*
   AlgorithmWorkbenchDialog + pages/*（尤其 RoutingPage — 勿在此堆 BT）
   ExperimentComparePanel / ExperimentMetrics.*
   tests/integration/LaneRoutingDemoScenarioTest.cpp（Phase 8 回归锚点）
   tools/verify_phase8_evidence.py（仿写 verify_phase9）

【Phase 9 完成定义（全部达成才可结束 Goal）】
A. Behavior Tree 核心（Domain，零 Qt）：
   - NodeStatus：SUCCESS / FAILURE / RUNNING
   - 控制节点：Sequence、Fallback、Recovery（两子节点 + number_of_retries，语义对齐 Nav2 RecoveryNode）
   - 装饰节点（MVP 至少一个）：RateTick 或 RepeatUntilSuccess（用于周期性 replan）
   - Blackboard：键值存储（agent_id、goal、path_valid、replan_requested、recovery_count…）
   - 禁止空壳 tick() 恒 SUCCESS 冒充 BT 引擎
B. FleetSim 叶节点（Domain Action/Condition，须接 SimEngine 真逻辑）：
   - BtPlanPath：调 planPathForAgent / planPath；写 blackboard path_valid
   - BtFollowUntilGoal：检查 goal_reached / 路径非空（不重复实现 tracker，仍由 SimEngine::tick 跟踪）
   - BtReplanIfTimer：按 replan_hz 或 needs_replan 触发重规划
   - BtWaitRecovery：仿真等待 N tick（教学版 recovery，非 Spin/BackUp 运动学）
   - BtIsGoalUpdated：条件节点，goal 变化则 SUCCESS
   - BtYieldIfBlocked（可选）：检测 reservation 冲突时 RUNNING/FAILURE，触发 replan
C. BtNavigator / BtEngine：
   - 从 JSON 加载树（MVP 用 JSON，非完整 XML；schema 写死 ADR-020）
   - 每 SimEngine tick 对启用 BT 的 agent tick 一次（或子 tick 与 sim dt 对齐）
   - simulation.behavior_mode: legacy | bt（默认 legacy，保 Phase 8 行为）
   - legacy 模式：现有 AgentPhase + needs_replan 逻辑不变
D. SimEngine / scenario 接线：
   - scenario.simulation.behavior_mode、behavior_tree_path（相对场景目录）
   - scenario.simulation.replan_hz（默认 1.0，对齐 Nav2 RateController 思想）
   - SimController / MainWindow 应用字段；禁止字段写了却不读
E. ExperimentMetrics CSV 导出（Domain + UI 最小触点）：
   - ExperimentMetrics::exportCsv(path) 或等价；列含 tick、cross_track、heading_error、st_ref_v、mpc_ok
   - **仅** ExperimentComparePanel 增加 Export CSV 按钮（≤20 行）；禁止堆进 Workbench/ControlPanel/Monitor
   - 单测：导出后文件可读、行数与 sample_count 一致
F. UI 信息架构（硬合同 — 防堆控件）：
   - 新建 BehaviorTreePanel（独立 dock）：当前 active 节点名、BT 状态、blackboard 摘要（只读）
   - AlgorithmWorkbench 新建第 6 页 BehaviorPage：仅 behavior_mode、tree 文件、replan_hz、recovery 开关
   - 禁止把 BT 配置堆进 RoutingPage / PlanningPage / SpeedPage / LaneEditorPanel / TaskPanel
   - MainWindow 挂载 BehaviorTreePanel dock ≤15 行
G. scenario：assets/scenarios/bt_navigation_demo/
   - 含 behavior_mode=bt、behavior_tree JSON、可演示 plan 失败 → recovery → replan 成功
   - Phase 8 lane_routing_demo、Phase 7 prediction_st_demo 仍须可加载回归
H. CMake 登记；Domain 零 Qt/rclcpp；保留 target_include_directories / target_link_libraries
I. 文档：ADR-020（BT MVP + JSON schema）、ADR-021（Behavior UI IA）；DEVELOPMENT_PLAN Phase9✅；SESSION_LOG；MUTATION M40+
J. 测试：BehaviorTreeTest、ExperimentMetricsExportTest、BtNavigationIntegrationTest、Phase8/7 回归；
    ASCII Build + FleetSimTests 全绿 + verify_phase9_evidence.py 才可 complete
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
3. Tester（测试）：写/跑/补 GTest（ASCII D:\build\FleetSim_phase9_*）；假 BT（tick 恒 SUCCESS）→ FAIL；UI 堆控件 → FAIL
4. Reviewer（检查）：对照 §11 + UI_GUIDELINES + Phase2–8 防回归；输出 PASS/FAIL
流程：Planner → Executor → Tester → Reviewer。主 Agent 禁止自评 PASS。SESSION_LOG 须引用四角色结论。

【建议 7 会话，可连续但禁止跳验收】
0: ADR-020/021 草案 + BT 类型/接口 + 红灯测骨架（BehaviorTreeTest 预期 FAIL）
1: BT 控制节点 + Blackboard + BehaviorTreeTest 控制流转绿
2: FleetSim 叶节点 + BtNavigator JSON 加载 + 单元测
3: SimEngine behavior_mode 接线 + BtNavigationIntegrationTest
4: UI — BehaviorTreePanel + BehaviorPage（Workbench 第 6 页）
5: bt_navigation_demo + ExperimentMetrics CSV + Phase7/8 回归
6: MUTATION M40+ + verify_phase9 + 四角色终审 + push + Phase9 ✅

【禁止偷懒 — 执行 Agent 必查】
1. 禁止 BT 引擎为空壳（所有节点 tick 恒 SUCCESS）却标 Phase9 ✅
2. 禁止 behavior_mode 写了但 SimEngine 仍只走硬编码 AgentPhase 且不读 tree
3. 禁止把 BT 监控/配置堆进 RoutingPage、PlanningPage、LaneEditorPanel、MonitorPanel
4. 禁止在一个 Panel 里同时放「BT 树编辑 + planner + routing + lane + 实验对比 + CSV 全字段配置」
5. 禁止用 if-else 500 行冒充 Behavior Tree（须可 JSON 加载、可单测节点）
6. 禁止 Domain 写 Qt / rclcpp
7. 禁止新 Domain 类无 GTest
8. 禁止削 CMake target_*
9. 禁止破坏 Phase8 hybrid/lane、Phase7 prediction/ST/Workbench/Compare 回归
10. 禁止引入完整 BehaviorTree.CPP / Nav2 插件系统 / Lanelet2 / 感知
11. 禁止 SESSION_LOG 缺四角色 / 缺「没做什么」
12. 禁止未更新 CMakeLists 就加源文件
13. 禁止 Phase9 ✅ 但未新建 BehaviorTreePanel 与 BehaviorPage
14. 禁止 CSV 导出按钮出现在 MainWindow toolbar 或 ControlPanel
15. 禁止 Recovery 节点语义与 Nav2 相反（先主行为、失败才 recovery 子行为）

从会话 0 开始立刻执行。中间不要换模型。
```

---

## §1 Phase 9 目标与验收标准

### 1.1 为何是 Phase 9（与学习路径对齐）

| 已完成（Phase 8） | Phase 9 要补的缺口 | 自动驾驶对应 |
|------------------|-------------------|--------------|
| 路由层：freespace / lane_graph / hybrid | **决策编排层**：何时 plan、follow、replan、recovery | Nav2 Behavior Tree |
| SimEngine 内硬编码 `AgentPhase` + `needs_replan` | **可配置行为树**驱动导航子流程 | `ComputePathToPose` + `FollowPath` + Recovery |
| ExperimentMetrics 内存聚合 | **CSV 导出**供离线分析 | 仿真日志 / rosbag 分析思想 |
| Workbench 5 页（含 Routing） | **第 6 页 Behavior** + **独立 BT 监控 dock** | Nav2 BT 监控 / 插件选择 UI 分离 |

Phase 8 解决了「**走哪条路**」；Phase 9 解决「**何时规划、何时跟踪、失败怎么办**」——工业栈中 Planner/Controller 之上的 **Behavior Tree 编排层**（教学简化版）。

### 1.2 验收清单（全部勾选 = Phase 9 完成）

- [ ] `BtEngine` / `BtNavigator`：JSON 加载树；tick 返回 NodeStatus；Blackboard 跨节点共享
- [ ] 控制节点 Sequence / Fallback / Recovery 单测覆盖
- [ ] 叶节点 `BtPlanPath` 真调 `SimEngine` 规划；失败触发 Recovery 子树
- [ ] `simulation.behavior_mode=bt` 时启用 BT；**默认 legacy** 与 Phase 8 行为一致
- [ ] **BehaviorTreePanel** 独立 dock；**BehaviorPage** 为 Workbench 第 6 页
- [ ] RoutingPage / LaneEditorPanel / ExperimentComparePanel **未** 混入 BT 表单（Reviewer 行数/关键词审计）
- [ ] `ExperimentMetrics::exportCsv` + ComparePanel Export 按钮；单测验证 CSV 内容
- [ ] `bt_navigation_demo` 可加载、可演示 replan/recovery
- [ ] ADR-020、ADR-021；DEVELOPMENT_PLAN Phase 9 ✅；MUTATION M40+；`verify_phase9_evidence.py` PASS
- [ ] GTest 全绿（≥139 + 新增测，预期 ~150+）

### 1.3 明确不做（Phase 9 范围外）

| 项目 | 原因 |
|------|------|
| 完整 BehaviorTree.CPP / Nav2 bt_navigator 移植 | 依赖与插件生态爆炸；本阶段自研教学 MVP |
| BT XML 完整兼容 Nav2 全部节点 | MVP 用 JSON schema；XML 可 Phase 10 |
| Spin / BackUp / ClearCostmap 真运动 recovery | 用 WaitRecovery + replan 教学；运动 recovery 另阶段 |
| 完整 Lanelet2 / OSM 导入 | Phase 8 已界定；Phase 10+ |
| 神经网络 / 多模态预测 / 激光感知 | 明确另阶段 |
| 多车每车独立 BT 森林（可选 stretch） | MVP 先单车；多车 legacy 调度保持 |

---

## §2 当前框架结构（必须保持）

```
UI (Qt) → App (SimController / ProjectManager / MonitorBridge / FleetUiCoordinator)
        → Domain (
              SimEngine + planning/map/scheduling/collision/prediction/experiment
              + behavior/   ← Phase 9 新增
           )
        → Core (Pose / Path / SpeedProfile / EventBus / SimClock)

CMake:
FleetSimCore → FleetSimDomain → FleetSimApp → FleetSimUI → FleetSim
FleetSimTests → App + Domain + Core + GTest
```

**Phase 9 增量（示意）：**

```
domain/behavior/BtTypes.h                 NodeStatus, BtNodeId
domain/behavior/BtBlackboard.h            键值 + 类型擦除（或 variant map）
domain/behavior/BtNode.h                  抽象 tick()
domain/behavior/BtControlNodes.*          Sequence, Fallback, Recovery
domain/behavior/BtDecoratorNodes.*        RateTick（replan 频率）
domain/behavior/BtFleetActionNodes.*      PlanPath, FollowUntilGoal, WaitRecovery
domain/behavior/BtFleetConditionNodes.*   IsGoalUpdated, IsPathValid
domain/behavior/BtTreeLoader.*            JSON → 内存树
domain/behavior/BtNavigator.*             持有树 + blackboard + tick 入口
扩展 SimEngine.*                          behavior_mode_, bt_navigator_, tick 分支
扩展 scenario/*                           behavior_mode, behavior_tree_path, replan_hz
扩展 experiment/ExperimentMetrics.*       exportCsv
ui/panels/BehaviorTreePanel.*             BT 状态监控 dock
ui/dialogs/pages/BehaviorPage.*           Workbench 第 6 页
assets/behavior_trees/navigate_replan_recovery.json
assets/scenarios/bt_navigation_demo/
```

**数据流（BT 模式 + 既有规划栈，示意）：**

```
User Goal / Task
  → [BT Root: RecoveryNode]
       ├─ [Navigation: Sequence]
       │    ├─ RateTick(replan_hz) → BtPlanPath → SimEngine::planPathForAgent
       │    └─ BtFollowUntilGoal → SimEngine::tick 内 tracker 跟踪
       └─ [Recovery: Sequence]
            └─ BtWaitRecovery → 再 tick Navigation
  → reference_path → ST (optional) → Tracker (MPC/Stanley/PP)
  → routing_mode (freespace|lane_graph|hybrid) 仍在 planPath 内生效 — BT 不替代路由算法
```

**关键设计原则**：BT **编排** plan/follow/replan/recovery 的**时序**；**不**重写 A*、LaneGraph、MPC、ST。

---

## §3 优先阅读的程序（按优先级）

### P0 — 动手前必读

| 文件 | 为什么 |
|------|--------|
| `src/domain/SimEngine.cpp` | tick、planPath、handleAgentGoalReached、needs_replan — **BT 注入点** |
| `src/domain/SimEngine.h` | 扩展 behavior_mode / BtNavigator 成员 |
| `src/domain/vehicle/FleetManager.*` | AgentPhase 状态机；legacy 路径须保留 |
| `src/domain/vehicle/FleetManager.h` | VehicleAgent：goal、needs_replan、phase、reference_path |
| `src/domain/planning/LaneRouter.*` / `LaneGraph.*` | Phase 8 路由 — BT plan 节点会间接调用 |
| `src/domain/experiment/ExperimentMetrics.*` | CSV 导出入口 |
| `src/ui/panels/ExperimentComparePanel.*` | Export 按钮唯一合法位置 |
| `src/ui/dialogs/AlgorithmWorkbenchDialog.*` | 加 BehaviorPage（第 6 Tab） |
| `src/ui/dialogs/pages/RoutingPage.*` | **勿在此堆 BT** — 对照 Phase 8 先例 |
| `src/ui/panels/LaneEditorPanel.*` | Phase 8 UI 分层先例 |
| `tests/integration/LaneRoutingDemoScenarioTest.cpp` | Phase 8 回归锚点 |
| `tools/verify_phase8_evidence.py` | 仿写 verify_phase9 |

### P1 — 场景与 App 接线

| 文件 | 为什么 |
|------|--------|
| `src/app/SimController.cpp` | applyScenarioToEngine |
| `src/ui/MainWindow.cpp` | dock 挂载、Workbench 菜单 |
| `src/domain/scenario/ScenarioLoader.h` / `ScenarioSerializer.cpp` | 新 simulation 字段 |
| `assets/scenarios/lane_routing_demo/` | Phase 8 hybrid 回归 |
| `assets/scenarios/prediction_st_demo/` | Phase 7 回归 |
| `docs/decisions/017-algorithm-workbench-ui.md` | Workbench 分页 IA 先例 |

### P2 — 防回归（继续生效）

| ID | 对策 |
|----|------|
| P8-Lane/Hybrid | LaneGraphTest、FirstLastMile*、LaneRoutingDemo* 仍绿 |
| P8-UI | LaneEditorPanel + RoutingPage 独立；MapEditor 未膨胀 |
| P7-Pred/ST | ConstantVelocity/StGraphWithPrediction/ExperimentCompare 仍绿 |
| P7-UI | Workbench 5 页 + Compare 独立 |
| P6-MPC/ST | M31–M33 仍有效 |
| P4-TEMP | test_tmp ASCII |

---

## §4 网络调研摘要（执行 Agent 必读）

### 4.1 Nav2 Behavior Tree：Navigation + Recovery 双 subtree

Nav2 默认树 `navigate_to_pose_w_replanning_and_recovery.xml` 的核心结构（教学简化）：

1. **顶层 RecoveryNode**（`number_of_retries=6`）：左子树 = 导航；右子树 = 系统级 recovery。  
2. **Navigation subtree（PipelineSequence）**：  
   - `RateController hz=1.0` 装饰 **ComputePathToPose**（周期性重规划，避免 100Hz 刷爆 planner）  
   - **FollowPath**（跟踪当前 path）  
   - 各自包一层 **RecoveryNode**：主行为失败 → 上下文 recovery（Nav2 清 costmap；FleetSim 教学版用 Wait + replan）  
3. **Recovery subtree**：`RoundRobin` 轮换 Spin / Wait / BackUp 等；任一 SUCCESS 后回到 Navigation。  
4. **条件节点**：`GoalUpdated` — 目标变化则中断当前子树、强制 replan（ReactiveFallback 模式）。

**FleetSim Phase 9 选择**：

| Nav2 概念 | FleetSim MVP |
|-----------|--------------|
| RecoveryNode | `BtRecoveryNode`（两子节点 + retries） |
| PipelineSequence | `BtSequenceNode` + RateTick 装饰 plan |
| ComputePathToPose | `BtPlanPath` → `SimEngine::planPathForAgent` |
| FollowPath | `BtFollowUntilGoal`（依赖现有 tracker tick） |
| RateController 1Hz | `replan_hz` scenario 字段 + `BtRateDecorator` |
| ClearCostmap / Spin / BackUp | **不做**；用 `BtWaitRecovery` + 重试 plan 教学 |
| GoalUpdated | `BtIsGoalUpdated` 条件节点 |

参考：[Nav2 BT Walkthrough](https://docs.nav2.org/behavior_trees/overview/detailed_behavior_tree_walkthrough.html) · [RecoveryNode](https://docs.nav2.org/configuration/packages/bt-plugins/controls/RecoveryNode.html) · [Nav2 Specific Nodes](https://docs.nav2.org/behavior_trees/overview/nav2_specific_nodes.html)

### 4.2 BehaviorTree.CPP：概念对照（不引入依赖）

| BT.CPP 概念 | FleetSim Phase 9 |
|-------------|------------------|
| TreeNode::tick() | `BtNode::tick(BtBlackboard&)` |
| NodeStatus SUCCESS/FAILURE/RUNNING | `BtTypes.h` 同名枚举 |
| Blackboard + Ports | `BtBlackboard` 键值；MVP 可不用 Port 重映射 |
| SyncActionNode | FleetSim Action 叶节点 |
| ConditionNode | FleetSim Condition 叶节点 |
| XML createTreeFromFile | **JSON** `BtTreeLoader`（降低 MVP 复杂度） |
| BehaviorTreeFactory 插件注册 | 编译期注册节点类型 map（string → factory fn） |

参考：[BehaviorTree.CPP Main Concepts](https://www.behaviortree.dev/docs/learn-the-basics/main_concepts) · [Blackboard and Ports](https://www.behaviortree.dev/docs/tutorial-basics/tutorial_02_basic_ports)

**为何不自研 if-else**：Goal Agent 会写 `if (fail) replan()` 糊弄；强制 JSON 树 + 单测控制流节点可审计。

### 4.3 行为树 vs 有限状态机（FleetSim 现状）

当前 `FleetManager` 使用 `AgentPhase::Idle | ToPickup | ToDropoff` + `handleAgentGoalReached` — 这是 **任务级 FSM**，应 **保留** 在 `behavior_mode=legacy`。

Phase 9 的 BT 负责 **导航子流程**（plan → follow → replan → recovery），与任务调度（Greedy/Hungarian、Pickup/Dropoff）**正交**：

- legacy：SimEngine 见 `needs_replan` 直接 plan  
- bt：SimEngine tick 前/中调用 `BtNavigator::tick()`，由树决定何时 `needs_replan=true` 或直接调 plan

### 4.4 ExperimentMetrics CSV：仿真实验可追溯性

工业仿真/Common 做法：时间序列指标导出 CSV，便于 Python/MATLAB 离线绘图。FleetSim Phase 7 已有 `TickSample` 滚动窗口；Phase 9 补 **exportCsv**，UI 仅在 **ExperimentComparePanel** 暴露，与 BT 监控分离。

### 4.5 UI 分层启示（防 Goal Agent 堆控件 — **本章最重要**）

| 层次 | 用户心智 | FleetSim 落点 | 禁止 |
|------|---------|--------------|------|
| 环境几何 | 障碍物、边界 | MapEditorPanel | 与 lane/BT 混放 |
| 拓扑车道 | 节点、边 | LaneEditorPanel | 与 BT 混放 |
| 路由策略 | graph/freespace/hybrid | RoutingPage（Tab 5） | 与 BT 混放 |
| **行为编排** | 何时 plan/replan/recovery | **BehaviorPage（Tab 6）** + **BehaviorTreePanel** | 塞进 Routing/Planning |
| 算法参数 | planner/tracker/ST/pred | Workbench Tab 1–4 | 与 BT 树结构混放 |
| 运行监控 | 曲线、误差 | MonitorPanel | 加 BT 表单 |
| 实验对比 | 聚合指标、**CSV 导出** | ExperimentComparePanel | 加 routing/BT 控件 |

**Reviewer 量化红线**：

- `RoutingPage.cpp` 出现 `behavior_mode` / `RecoveryNode` / `replan_hz` → **FAIL**  
- `BehaviorPage.cpp` 出现 `routing_mode` / `lane_snap` / planner 下拉 → **FAIL**（应读 Workbench 其它页）  
- `BehaviorTreePanel.cpp` >300 行且未拆 `BtStatusWidget` → **FAIL**  
- MainWindow 新增 BT 业务逻辑 >15 行 → **FAIL**

---

## §5 设计决策建议（先写 ADR 再写码）

### ADR-020 — Behavior Tree MVP + JSON Schema

1. **simulation 字段**：

```json
"simulation": {
  "behavior_mode": "legacy",
  "behavior_tree_path": "navigate_replan_recovery.json",
  "replan_hz": 1.0,
  "recovery_wait_ticks": 20
}
```

- `behavior_mode` 默认 `legacy`；`bt` 启用 BtNavigator。  
- `behavior_tree_path` 相对场景目录或 `assets/behavior_trees/`。

2. **JSON 树 schema（MVP 写死）**：

```json
{
  "version": 1,
  "root": {
    "type": "Recovery",
    "name": "NavigateRecovery",
    "retries": 3,
    "children": [
      {
        "type": "Sequence",
        "name": "NavigateWithReplanning",
        "children": [
          {
            "type": "Rate",
            "hz": 1.0,
            "child": { "type": "Action", "name": "PlanPath" }
          },
          { "type": "Action", "name": "FollowUntilGoal" }
        ]
      },
      {
        "type": "Sequence",
        "name": "RecoveryWait",
        "children": [
          { "type": "Action", "name": "WaitRecovery" },
          { "type": "Action", "name": "PlanPath" }
        ]
      }
    ]
  }
}
```

3. **Recovery 语义**（对齐 Nav2）：  
   - 仅当第一个 child 返回 FAILURE 时 tick 第二个 child；  
   - 第二个 child SUCCESS 后重试第一个 child；  
   - 超过 `retries` 返回 FAILURE。

4. **禁止**：`BtNavigator::tick()` 空实现或忽略 child 顺序。

### ADR-021 — Behavior UI 信息架构

1. **BehaviorTreePanel**（独立 dock）：  
   - 显示：当前树名、active 节点路径、上次 NodeStatus、replan/recovery 计数  
   - **只读监控**；不在此编辑 JSON 树（编辑走场景文件 + 未来专用编辑器）  
2. **BehaviorPage**（Workbench Tab 6）：  
   - `behavior_mode` 下拉：legacy / bt  
   - `behavior_tree_path` 文本或文件 picker  
   - `replan_hz`、`recovery_wait_ticks`  
   - **禁止**：planner、routing_mode、prediction、lane 控件  
3. **ExperimentComparePanel**：仅增 Export CSV 按钮 + 文件对话框  
4. **行数红线**：单 Panel/Page `.cpp` ≤300 行；超出拆 SubWidget。

---

## §6 建议新增/变更文件清单

### Domain / Behavior

| 文件 | 职责 |
|------|------|
| `behavior/BtTypes.h` | NodeStatus、节点类型枚举 |
| `behavior/BtBlackboard.h` | 共享状态 |
| `behavior/BtNode.h` | 抽象接口 |
| `behavior/BtControlNodes.*` | Sequence、Fallback、Recovery |
| `behavior/BtDecoratorNodes.*` | Rate |
| `behavior/BtFleetActionNodes.*` | PlanPath、FollowUntilGoal、WaitRecovery |
| `behavior/BtFleetConditionNodes.*` | IsGoalUpdated、IsPathValid |
| `behavior/BtTreeLoader.*` | JSON 解析 |
| `behavior/BtNavigator.*` | 树实例 + tick |
| 扩展 `SimEngine.*` | behavior_mode 分支 |
| 扩展 `experiment/ExperimentMetrics.*` | exportCsv |

### UI

| 文件 | 职责 |
|------|------|
| `ui/panels/BehaviorTreePanel.*` | BT 监控 dock |
| `ui/dialogs/pages/BehaviorPage.*` | Workbench 第 6 页 |
| 扩展 `AlgorithmWorkbenchDialog.*` | Tab 6 + settings 字段 |
| 扩展 `ExperimentComparePanel.*` | Export CSV |

### 测试 / 文档 / 工具 / 资产

```
tests/domain/BehaviorTreeTest.cpp
tests/domain/ExperimentMetricsExportTest.cpp
tests/integration/BtNavigationIntegrationTest.cpp
docs/decisions/020-behavior-tree-mvp.md
docs/decisions/021-behavior-ui-ia.md
tools/verify_phase9_evidence.py
tools/run_phase9_verify.ps1
assets/behavior_trees/navigate_replan_recovery.json
assets/scenarios/bt_navigation_demo/
```

---

## §7 如何继续开发（Goal 节奏 + 四角色）

1. `git pull`；确认 Phase 8：`python tools/verify_phase8_evidence.py`；FleetSimTests **139** 绿  
2. 会话 0：ADR-020/021 + BT 接口 + 红灯测  
3. 会话 1–2：控制节点 + 叶节点 + JSON 加载  
4. 会话 3：SimEngine 接线 + 集成测  
5. 会话 4：UI 分层（BehaviorTreePanel + BehaviorPage）  
6. 会话 5–6：demo + CSV + 回归 + MUTATION + 终审  

### 四角色监督协议

| 角色 | 输入 | 输出 | 否决权 |
|------|------|------|--------|
| **Planner** | Phase9 目标 + 本会话 scope | mini-plan + **UI 分页清单** + NOT DO ≥5 | 范围蔓延 |
| **Executor** | mini-plan | 代码 + CMake | 越 scope → Reviewer FAIL |
| **Tester** | 代码 + 测例 | 红绿 + 假 BT/UI 堆叠判定 | 无测/假 BT/堆 UI → FAIL |
| **Reviewer** | diff + §11 + UI_GUIDELINES | PASS/FAIL | FAIL 阻 complete |

**子 Agent 调用要求（主 Agent 每会话必须）**：

1. 启动 **Planner** 子 Agent：产出 mini-plan（含 UI 分页表）  
2. Executor 实施（主 Agent 或子 Agent）  
3. 启动 **Tester** 子 Agent：跑 GTest + grep UI 违规关键词  
4. 启动 **Reviewer** 子 Agent：对照 §11 禁止偷懒清单  
5. **禁止**主 Agent 在未跑 Tester/Reviewer 前自评 PASS  

**UI 专项 Tester 检查**：

- `RoutingPage.cpp` 新增 `behavior` / `recovery` / `Bt` 关键词 → **FAIL**  
- Workbench 无第 6 页 `BehaviorPage` 却标 Phase 9 UI 完成 → **FAIL**  
- `ControlPanel` / `MonitorPanel` 出现 Export CSV 或 BT 表单 → **FAIL**  
- `BehaviorPage.cpp` 行数 >300 或未独立文件 → **FAIL**

---

## §8 Phase 2–8 防回归（继续生效）

| ID | 对策 |
|----|------|
| P8-Lane/Hybrid | Lane* / FirstLast* / LaneRoutingDemo* 仍绿 |
| P8-UI | LaneEditor + RoutingPage 独立 |
| P7-Pred/ST | prediction/CV/StGraph/ExperimentCompare 仍绿 |
| P7-UI | Workbench 原 5 页职责不变 |
| P6-MPC/ST | Mpc*/StGraph* 仍绿 |
| P5-Hybrid/Priority | 仍绿 |
| P9-BT | legacy 默认；显式 bt 才启用树 |

---

## §9 关键参数默认值

| 参数 | 默认 | 说明 |
|------|------|------|
| simulation.behavior_mode | legacy | 保 Phase 8 行为 |
| behavior_tree_path | 空 | bt 模式必填 |
| replan_hz | 1.0 | 对齐 Nav2 RateController 思想 |
| recovery_wait_ticks | 20 | WaitRecovery 仿真 tick 数 |
| routing_mode | freespace | Phase 8 不变；BT 不修改 |
| Recovery retries | 3 | 教学 |

---

## §10 测试策略（最低标准）

| 层级 | 要求 |
|------|------|
| 单元 BT 控制流 | Sequence 全 SUCCESS；Fallback 第一个 SUCCESS；Recovery 主失败→recovery 成功→主成功 |
| 单元 BT 叶节点 | PlanPath mock SimEngine 接口或 fixture engine |
| 单元 CSV | exportCsv 行数、表头、数值字段 |
| 集成 BtNavigation | behavior_mode=bt 场景 load → plan 成功；模拟 plan 失败 → recovery → 再 plan |
| 回归 Phase 8 | LaneRoutingDemo*、FirstLastMile* 全绿 |
| 回归 Phase 7 | prediction_st_demo 相关测绿 |
| 突变 M40 | Recovery 语义反写（先 recovery 后主行为）→ BehaviorTreeTest FAIL |
| 突变 M41 | behavior_mode=bt 但 tick 不调 planPath → BtNavigationIntegrationTest FAIL |
| 突变 M42 | exportCsv 写空文件 → ExperimentMetricsExportTest FAIL |

**预期新增测试数量**：约 12–18 个 TEST（3 个文件），总套件 **~150–155**。

---

## §11 禁止偷懒清单（Reviewer 打印打勾）

1. 禁止 BT 空壳（tick 恒 SUCCESS）冒充完成  
2. 禁止 behavior_mode 字段不读  
3. 禁止用 SimEngine 巨型 if-else 替代 JSON 树  
4. 禁止 BT 配置堆 RoutingPage / PlanningPage / LaneEditorPanel  
5. 禁止 BT 监控堆 MonitorPanel / ControlPanel  
6. 禁止 CSV 导出堆 Workbench / MainWindow toolbar  
7. 禁止同一 Panel 混合「行为树 + 路由 + 车道 + 算法 + 实验」  
8. 禁止 Domain 含 Qt / rclcpp  
9. 禁止新 Domain 类无 GTest  
10. 禁止削 CMake `target_*`  
11. 禁止破坏 Phase8 hybrid/lane、Phase7 prediction/ST  
12. 禁止引入 BehaviorTree.CPP / Nav2 全栈 / Lanelet2  
13. 禁止 SESSION_LOG 缺四角色 / 缺「没做什么」  
14. 禁止未登记 CMake 就加源  
15. 禁止跳过四角色子 Agent  
16. 禁止 Phase9 ✅ 但无 BehaviorTreePanel + BehaviorPage  
17. 禁止 Recovery 节点 child 顺序与 Nav2 语义相反  
18. 禁止 legacy 模式行为相对 Phase 8 回退（默认必须 legacy）

---

## §12 用户本地验证清单（每次交付必须附上）

1. `git pull origin main`  
2. 可选：`pwsh -File tools/run_phase9_verify.ps1`  
3. Qt Creator Configure → Build；或 ASCII：  
   `cmake -S . -B D:\build\FleetSim_phase9 -DCMAKE_PREFIX_PATH=D:/QT/6.11.1/mingw_64 -G "MinGW Makefiles"`  
4. 运行 `FleetSimTests`（关注 Behavior* / Bt* / ExperimentMetricsExport* / Phase8* / Phase7*）  
5. Open `assets/scenarios/bt_navigation_demo` → behavior_mode=bt → Play → 观察 BehaviorTreePanel active 节点变化  
6. Open `assets/scenarios/lane_routing_demo` → hybrid 仍可用（Phase 8 回归）  
7. Open `assets/scenarios/prediction_st_demo` → Phase 7 回归  
8. Experiment Compare → Export CSV → 用 Excel/文本编辑器打开验证列  
9. Algorithm Workbench → **Behavior 页** 切 legacy/bt；**Routing 页** 不应出现 BT 控件  

---

## §13 风险与缓解

| 风险 | 缓解 |
|------|------|
| Agent 用 if-else 冒充 BT | 强制 JSON 加载 + BehaviorTreeTest 控制流 |
| Agent 把 BT 堆进 RoutingPage | ADR-021 + grep 审计 |
| BT tick 与 SimEngine tick 双重 plan | Blackboard 标志 + 单测断言 plan 调用次数 |
| legacy 回归破坏 | behavior_mode 默认 legacy + LaneRoutingDemo 必跑 |
| UI 文件超 300 行 | 强制拆 BtStatusWidget |
| 引入 BT.CPP 依赖 | ADR 明确禁止；Reviewer 查 CMake |

---

## §14 参考链接

1. [Nav2 Detailed Behavior Tree Walkthrough](https://docs.nav2.org/behavior_trees/overview/detailed_behavior_tree_walkthrough.html)  
2. [Nav2 RecoveryNode](https://docs.nav2.org/configuration/packages/bt-plugins/controls/RecoveryNode.html)  
3. [Nav2 Behavior Tree Overview / Specific Nodes](https://docs.nav2.org/behavior_trees/overview/nav2_specific_nodes.html)  
4. [BehaviorTree.CPP — Main Concepts](https://www.behaviortree.dev/docs/learn-the-basics/main_concepts)  
5. [BehaviorTree.CPP — Blackboard and Ports](https://www.behaviortree.dev/docs/tutorial-basics/tutorial_02_basic_ports)  
6. Phase 8：`docs/PHASE8_GOAL_PROMPT.md`、`docs/decisions/018-*.md`、`019-*.md`  
7. Phase 7：`docs/PHASE7_GOAL_PROMPT.md`、`docs/decisions/016-*.md`、`017-*.md`

---

## §15 新对话开场白（短版）

> 执行 `docs/PHASE9_GOAL_PROMPT.md`。Goal：完成 Phase 9（Behavior Tree MVP + ExperimentMetrics CSV + BehaviorTreePanel + Workbench BehaviorPage + bt_navigation_demo）。强制四角色子 Agent；GTest + verify_phase9 全绿；每会话 push。从会话 0 开始。

---

## §16 Planner mini-plan 模板（每会话必填）

```markdown
**Phase**: 9
**本会话目标**: （一句话）
**允许改动**: （目录/文件）
**NOT DO**: （≥5 条，含禁止堆 UI / 假 BT / if-else 冒充树 / 破坏 Phase8 legacy）
**预计新增/修改文件**:
**计划测试**（精确 TEST 名）:
**UI 分页验收**:
  - 必须新建/修改: BehaviorTreePanel / BehaviorPage
  - 禁止改动含 BT 表单: RoutingPage / LaneEditorPanel / MonitorPanel / ControlPanel
  - ExperimentComparePanel 仅允许 Export CSV 按钮
**验收标准**:
**四角色**: Planner / Executor / Tester / Reviewer 结论占位
```

---

## §17 Phase 10 展望（勿在 Phase 9 实现）

> **已落地为完整 Goal 提示词**：`docs/PHASE10_GOAL_PROMPT.md`（复制 §0 执行）。

- BT **XML** 导入（Nav2 子集兼容）  
- **Lanelet2/OSM** 轻量导入插件  
- 多车 **每 agent 独立 BT** + 黑板隔离  
- Spin / BackUp **运动 recovery**（需车辆运动学）  
- CBS-lite 真冲突消解  
- 感知 / 多模态预测 — 明确 Phase 11+  

---

## §18 verify_phase9_evidence.py 检查项（Session 6 必须实现）

静态脚本至少覆盖（仿 Phase 8，目标 **≥50 PASS**）：

| 分组 | 检查项 |
|------|--------|
| A BT Domain | BtNavigator.*、BtControlNodes.*、BehaviorTreeTest 存在 |
| B SimEngine | behavior_mode 分支、BtNavigator 成员 |
| C Scenario | behavior_mode / behavior_tree_path 序列化 |
| D CSV | exportCsv、ExperimentMetricsExportTest |
| E UI IA | BehaviorTreePanel、BehaviorPage、Workbench Tab 6 |
| F 负向 | RoutingPage 无 behavior_mode；BehaviorPage 无 routing_mode |
| G demo | bt_navigation_demo 场景 + JSON 树资产 |
| H CMake | behavior/ 源登记、Domain 无 Qt |
| I ADR | 020、021、M40–M42、Phase 9 marked |
| J 回归 | Phase8 verify 关键项仍 PASS |
| K 工具 | verify_phase9_evidence.py、run_phase9_verify.ps1 |

---

## §19 MUTATION M40–M42（Session 6 登记）

| ID | 注入点 | 预期 FAIL 测例 |
|----|--------|----------------|
| M40 | `BtRecoveryNode` — 先 tick recovery 再 tick 主行为 | `BehaviorTreeTest.Recovery*` |
| M41 | `SimEngine` — behavior_mode=bt 但 BtNavigator 不调用 plan | `BtNavigationIntegrationTest.*` |
| M42 | `ExperimentMetrics::exportCsv` — 写空或缺列 | `ExperimentMetricsExportTest.*` |

---

*文档版本：Phase 9 草案 · 2026-08-23 · 前置 Phase 8 Session 8（139/139 绿）*
