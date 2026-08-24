# Phase 11 Goal 模式执行提示词（完整版）

> **用途**：新开对话后，整段复制「§0 一键粘贴 Prompt」给 Agent（Goal 模式）。  
> **仓库**：https://github.com/gubai131210-ui/FleetSim.git  
> **本地路径**：`d:\QT_CppPrograms\FleetSim\FleetSim`  
> **前置**：Phase 0–10 ✅（OSM 子集 + BT XML/运动 Recovery + 多车 BT + CBS-lite + UI 四件套；FleetSimTests **196/196** 绿；`verify_phase10_evidence.py` **102 PASS**）。  
> **本阶段主题**：**教学版 EECBS（有界次优 + bypass + 矩形对称约束）** + **2D 激光射线投射感知层（mark/clear）** + **多模态运动学预测（CTRA + LaneKeep / IMM-lite，非神经网络）** + **交通灯/停车线 Regulatory 教学子集** + **非线性运动学 Bicycle NMPC（Domain 自研，零 CasADi）**；与 Phase 5–10 的 Hybrid/线性 MPC/ST/CV 预测/LaneGraph/BT/CBS-lite/OSM **并存、可切换、默认不破回归**。  
> **禁止**：编辑 `.cursor/plans/`；完整 Jiaoyang EECBS 工业实现 / 完整 Lanelet2 C++ 库 / CasADi·IPOPT / 神经网络预测 / 真机激光驱动 / Groot2·完整 BT.CPP；Agent 不代跑中文 Temp 破坏性操作；中文路径 Qt Build 由用户验证（Agent 可在 **ASCII** 路径如 `D:\build\FleetSim_phase11_*` 外置构建取证）。

---

## §0 一键粘贴 Prompt（Goal 模式直接用）

```
你是 FleetSim Phase 11 的执行 Agent（Goal 模式）。
仓库：d:\QT_CppPrograms\FleetSim\FleetSim
远程：https://github.com/gubai131210-ui/FleetSim.git
权威说明：docs/PHASE11_GOAL_PROMPT.md（必须先完整阅读再写代码）

【强制阅读顺序 — 未读完禁止写代码】
1. docs/PHASE11_GOAL_PROMPT.md（全文，尤其 §1–§13、§4 算法清单、§5 UI 分层硬合同）
2. docs/DEVELOPMENT_PLAN.md、docs/learning-path.md（确认 Phase 10 ✅、Phase 11 目标）
3. docs/AGENT_SESSION_TEMPLATE.md、SESSION_LOG.md（Phase 10 Session 7 终审）
4. docs/decisions/013 / 016 / 022–025、001-architecture.md、002-map-format.md
5. docs/UI_GUIDELINES.md + src/ui/panels/README.md + docs/MUTATION_CHECKLIST.md
6. 代码优先：
   domain/collision/CbsLiteCoordinator.*（扩展基座，禁止换皮冒充 EECBS）
   domain/prediction/ConstantVelocityPredictor.*
   domain/control/MpcLateralTracker.* / DenseQpSolver.*
   domain/map/OccupancyGrid.* / OsmLaneletImporter.*
   SimEngine.cpp（tick / planPath / prediction / coordination / tracker）
   AlgorithmWorkbenchDialog + pages/*（Coordination/Control/Speed/Behavior* — 勿再堆）
   MapEditorPanel / LaneEditorPanel / OsmImportPanel / BehaviorTreePanel / MultiAgentBehaviorPanel
   tools/verify_phase10_evidence.py（仿写 verify_phase11，目标 ≥70 PASS）
   tests：CbsLiteCoordinatorTest、ConstantVelocityPredictorTest、MpcLateralTrackerTest、
          OsmLaneletDemoScenarioTest、CbsLiteDemoScenarioTest

【Phase 11 完成定义（全部达成才可结束 Goal）】
A. LidarRayCaster + ObstacleLayer（Domain，零 Qt，零真机驱动）：
   - 从车体位姿发射 2D 扇形射线；命中静态障碍或动态他车几何 → range 命中
   - mark：命中格标 occupied；clear：沿射线 Bresenham/等价射线清除 free（对齐 Nav2 ObstacleLayer 思想）
   - 输出可合并进规划用 OccupancyGrid（或独立 perception layer 再 merge）；禁止「假扫描恒空」
   - 单测：墙前射线有命中；空旷无命中；clear 后曾标格可变 free
B. MultiModalPredictor（非 NN）：
   - 模式至少：ConstantVelocity（复用/包装既有）、CTRA（常转率+常加速度）、LaneKeep（沿最近车道中心线外推）
   - IMM-lite：按模式权重输出 ≥2 条模态轨迹，或融合主轨迹+备选模态列表（ADR 写死）
   - ST-Graph / 规划可消费多模态占用；默认 prediction=constant_velocity 保 Phase 7 回归
   - 禁止只改名 CV 却声称多模态；禁止引入 PyTorch/ONNX
C. EecbsCoordinator（教学有界，真实现）：
   - 在 CbsLite 之上：高层节点选择近似 EES（OPEN / FOCAL / CLEANUP 三表思想，ADR 写清简化）
   - bypass：若改路径可消冲突则不分支约束
   - rectangle symmetry：检测矩形对称冲突并一次加入 barrier 类约束（教学子集；不做 corridor/target 全套）
   - coordination=eecbs 可切换；默认仍 priority；cbs_lite 必须仍可运行
   - 禁止 EECBS = CbsLite 换名；禁止忽略 suboptimality 因子 w
D. Regulatory / TrafficLight 教学子集：
   - OSM 或场景 JSON 解析 stop_line + traffic_light（relation subtype 或 scenario 字段）
   - 运行时灯态机：red/yellow/green（可 scripted 时序）；红灯时在 stop_line 前强制停车/禁止越过
   - 单测：红灯车停在线前；绿灯可通过
E. NonlinearKinematicMpc（Domain 自研）：
   - 非线性运动学自行车模型前向预测（欧拉或 RK2/RK4）；迭代线性化 + 既有 DenseQpSolver，或教学 shooting
   - tracker=nonlinear_mpc 可切换；默认/auto 不强制启用（保线性 MPC 回归）
   - 禁止引入 CasADi/IPOPT；禁止空壳恒返回 0 舵角
F. SimEngine / scenario 接线：
   - 字段：perception、lidar_*、prediction=multimodal|constant_velocity|none、coordination 含 eecbs、
     eecbs_w / eecbs_*、tracker 含 nonlinear_mpc、traffic_light / regulatory 相关
   - 字段写了必须被读取；Phase 7–10 默认行为不破
G. UI 信息架构（硬合同 — Goal Agent 很懒很笨时尤其要遵守，禁止堆页）：
   - 新建 PerceptionPanel（独立 dock）：仅扫描开关摘要、命中点数、layer 可见性；禁止塞进 MapEditor/Monitor
   - Workbench 新建 PerceptionPage（新 Tab）：仅 perception on/off、range、ray_count、mark/clear 选项
   - Workbench 新建 MultiModalPredictionPage（新 Tab）：仅 prediction 模式、horizon、模态权重；禁止堆进 SpeedPage
   - Workbench 新建 RegulatoryPage（新 Tab）：仅交通灯/停车线绑定与时序脚本；禁止堆进 MapImportPage/RoutingPage
   - Workbench 新建 NonlinearControlPage（新 Tab）：仅 nonlinear_mpc 参数；禁止在 ControlPage（线性 MPC/Stanley）继续堆
   - CoordinationPage：仅增加 coordination=eecbs 与 w/depth；禁止把感知/预测/交通灯塞进 CoordinationPage
   - MapView 可用独立 GraphicsItem 画射线/灯态（只读叠加）；表单控件禁止堆到 MapView
   - MainWindow 仅挂载；单 Panel/Page .cpp ≤300 行，超出必须拆 SubWidget
H. scenario：assets/scenarios/perception_lidar_demo/ + eecbs_demo/ + traffic_light_demo/ + multimodal_prediction_demo/（可合并部分，但至少 3 个可加载 demo）
I. CMake 登记；Domain 零 Qt/rclcpp；保留 target_include_directories / target_link_libraries
J. 文档：ADR-026…030；DEVELOPMENT_PLAN Phase11✅；SESSION_LOG；MUTATION M47+
K. 测试：各算法 Domain GTest + 集成 demo 测；Phase 7–10 回归；
   ASCII Build + FleetSimTests 全绿 + verify_phase11_evidence.py（≥70 PASS）才可 complete
L. 每会话 commit + push；四角色互相监督 PASS；回复四段式

【本阶段必须实现的算法清单 — 禁止只做 UI / 禁止 stub 冒充】
1. 2D Lidar 射线投射（扇形 beams vs 障碍几何）
2. ObstacleLayer mark（命中格占用）
3. ObstacleLayer clear（射线清障）
4. 感知层合并进规划栅格
5. CTRA 轨迹预测
6. LaneKeep / 车道中心线外推模态
7. IMM-lite 多模态组合（≥2 模态可测区分）
8. EECBS 高层 EES 风格节点选择（w 有界次优）
9. Bypass conflicts
10. Rectangle symmetry barrier 约束
11. 交通灯状态机 + stop_line 停车语义
12. 非线性运动学 Bicycle NMPC（迭代线性化或 shooting + QP）
以上 12 项全部有 Domain 实现 + GTest；缺一不可标 Phase11 ✅

【架构硬约束】
UI → App → Domain → Core
Domain/Core 禁止 #include <Qt*> 与 rclcpp
App 层一律 domain:: 前缀命名空间
接口先于实现：先 .h 后 .cpp
新 UI = 新 Panel/Dialog + 独立 Page；MainWindow 仅挂载
禁止编辑 .cursor/plans/
中文路径：ProjectManager 测试用仓库内 ASCII test_tmp

【四角色子 Agent 团队 — 每会话强制】
1. Planner（计划）：mini-plan — 允许改动 / NOT DO（≥10 条） / 文件清单 / 测试清单 /
   UI 分页验收（必须点名新建哪些 Page/Panel；禁止污染清单）
2. Executor（执行）：只按 mini-plan 写代码与 CMake；禁止扩 scope；禁止「先做 UI 后补算法」
3. Tester（测试）：写/跑/补 GTest（ASCII D:\build\FleetSim_phase11_*）；空壳算法 → FAIL；UI 堆控件 → FAIL
4. Reviewer（检查）：对照 §11 + UI_GUIDELINES + Phase7–10 防回归；输出 PASS/FAIL
流程：Planner → Executor → Tester → Reviewer。主 Agent 禁止自评 PASS。SESSION_LOG 须引用四角色结论。
每会话结束应用独立子 Agent（或同等流程）再检查一次是否达到本会话需求。

【建议 9 会话，可连续但禁止跳验收】
0: ADR-026…030 草案 + 红灯测骨架（Lidar* / MultiModal* / Eecbs* / Regulatory* / NonlinearMpc* 预期 FAIL）
1: LidarRayCaster + ObstacleLayer mark/clear + 合并规划栅格 + LidarPerceptionTest 绿
2: CTRA + LaneKeep + IMM-lite MultiModalPredictor + MultiModalPredictorTest 绿；ST 可消费多模态
3: EecbsCoordinator：EES 选择 + w + 与 CbsLite 可切换 + EecbsCoordinatorTest 基础绿
4: Bypass + Rectangle barrier + 冲突场景测绿（证明非 CbsLite 换皮）
5: TrafficLight/StopLine regulatory + TrafficLightTest + SimEngine 停车语义
6: NonlinearKinematicMpc + NonlinearMpcTest；tracker 切换；线性 MPC 回归不破
7: UI — PerceptionPanel + PerceptionPage + MultiModalPredictionPage + RegulatoryPage + NonlinearControlPage
   （严禁堆页；负向审计）
8: demo 场景 + Phase7–10 回归 + MUTATION M47+ + verify_phase11 ≥70 + 四角色终审 + Phase11 ✅

【禁止偷懒 — 执行 Agent 必查（Reviewer 逐条打勾）】
1. 禁止 Lidar 恒返回空扫描 / 恒不 mark 却标 Phase11 ✅
2. 禁止 clear 与 mark 同一空函数
3. 禁止 MultiModal = ConstantVelocity 换名（模态数恒 1 且轨迹位码相同）
4. 禁止 EECBS = CbsLite 换名或只改 coordination 字符串
5. 禁止 bypass / rectangle 空实现却声称已做对称推理
6. 禁止交通灯只画 UI 不改车辆停车行为
7. 禁止 Nonlinear MPC 恒输出 0 控制量或直接调用线性 MpcLateralTracker 换皮
8. 禁止把感知/预测/交通灯/NMPC 控件堆进同一个 Workbench 页或同一个 Panel
9. 禁止在 ControlPage / SpeedPage / CoordinationPage / MapImportPage / BehaviorPage 继续无限加控件
10. 禁止 Domain 写 Qt / rclcpp
11. 禁止新 Domain 类无 GTest
12. 禁止削 CMake target_*
13. 禁止破坏 Phase10 CBS-lite / BT / OSM / Phase7 CV 预测 / Phase6 线性 MPC 回归
14. 禁止引入 CasADi、IPOPT、完整 Lanelet2、神经网络、真机激光驱动
15. 禁止 SESSION_LOG 缺四角色 / 缺「没做什么」
16. 禁止未更新 CMakeLists 就加源文件
17. 禁止 Phase11 ✅ 但 12 项算法清单有任何一项缺失
18. 禁止 MapView 上堆表单按钮当「感知 UI」

从会话 0 开始立刻执行。中间不要换模型。
```

---

## §1 Phase 11 目标与验收标准

### 1.1 为何是 Phase 11（与学习路径对齐）

| 已完成（Phase 10） | Phase 11 要补的缺口 | 自动驾驶对应 |
|------------------|-------------------|--------------|
| CBS-lite（深度有界约束树） | **EECBS 教学版**：EES 风格选点 + bypass + 矩形对称 | EECBS / ECBS / CBSH-RTC |
| 静态 map.json / OSM 车道 | **2D Lidar 射线感知层** mark/clear | Nav2 costmap ObstacleLayer |
| 仅 ConstantVelocity 预测 | **多模态**：CTRA + LaneKeep + IMM-lite | 物理/机动多模态预测（非 NN） |
| OSM 可行驶 lanelet | **Traffic light + stop_line** 教学监管 | Autoware Lanelet2 regulatory |
| 线性横向 MPC | **非线性运动学 NMPC** | NMPC / CasADi 栈的教学替代 |
| Workbench 8 Tab | **Perception / Prediction / Regulatory / NonlinearControl 新页** + Perception dock | 感知–预测–决策–控制分层 UI |

Phase 10 解决「地图从哪来、工业 BT XML、运动恢复、多车约束树入门」；Phase 11 解决「**看见动态世界**、**多假设预测**、**更强 MAPF**、**路口灯控**、**非线性跟踪**」。

### 1.2 验收清单（全部勾选 = Phase 11 完成）

- [ ] `LidarRayCaster` + `ObstacleLayer`：扫描非空可断言；mark/clear 可测；合并栅格影响 `planPath`
- [ ] `MultiModalPredictor`：≥2 模态轨迹可区分；`prediction=multimodal` 可切换；CV 默认回归
- [ ] `EecbsCoordinator`：`coordination=eecbs`；w 有界；bypass 或 rectangle 至少一处可观测减少分支/改变路径；`cbs_lite`/`priority` 仍可用
- [ ] TrafficLight + StopLine：红灯停车、绿灯通行单测
- [ ] `NonlinearKinematicMpc`：曲率较大路径上跟踪误差优于「零控制」或可与线性 MPC 对比非平凡；`tracker=nonlinear_mpc` 可切换
- [ ] **PerceptionPanel**、**PerceptionPage**、**MultiModalPredictionPage**、**RegulatoryPage**、**NonlinearControlPage** 均存在且未污染既有页
- [ ] ≥3 个 Phase 11 demo 场景可加载；Phase 7–10 demo 仍可加载
- [ ] ADR-026…030；DEVELOPMENT_PLAN Phase 11 ✅；MUTATION M47+；`verify_phase11_evidence.py` ≥70 PASS
- [ ] GTest 全绿（≥196 + 新增测）

### 1.3 明确不做（Phase 11 范围外 → Phase 12+）

| 项目 | 原因 |
|------|------|
| 完整 EECBS 工业实现（全部 symmetry + 在线学习 ĥ 全套） | 教学做 EES 三表思想 + bypass + rectangle；corridor/target 可 Phase 12 |
| CasADi / IPOPT / ACADOS | 依赖爆炸；自研迭代线性化 + DenseQpSolver |
| 神经网络预测 / 占用网络 | 明确另阶段 |
| 真机激光 / ROS2 LaserScan 驱动 | 仿真生成扫描即可；ROS2 桥已有可后续接 |
| 完整 Lanelet2 库 / GeographicLib | 继续自研教学子集 |
| Groot2 / 完整 BehaviorTree.CPP | 保持零第三方 BT 依赖 |
| 3D VoxelLayer / 点云 | Phase 11 仅 2D 射线 |

---

## §2 当前框架结构（必须保持）

```
UI (Qt) → App (SimController / ProjectManager / MonitorBridge / FleetUiCoordinator)
        → Domain (
              SimEngine + planning/map/scheduling/collision/prediction/experiment/behavior/control
              + perception/LidarRayCaster + ObstacleLayer     ← Phase 11
              + prediction/CtraPredictor + LaneKeepPredictor + MultiModalPredictor ← Phase 11
              + collision/EecbsCoordinator                    ← Phase 11
              + map/RegulatoryElements / TrafficLightRuntime ← Phase 11
              + control/NonlinearKinematicMpc                ← Phase 11
           )
        → Core (Pose / Path / SpeedProfile / EventBus / SimClock)

CMake:
FleetSimCore → FleetSimDomain → FleetSimApp → FleetSimUI → FleetSim
FleetSimTests → App + Domain + Core + GTest
```

**Phase 11 增量（示意）：**

```
domain/perception/LidarTypes.h
domain/perception/LidarRayCaster.*
domain/perception/ObstacleLayer.*
domain/prediction/CtraPredictor.*
domain/prediction/LaneKeepPredictor.*
domain/prediction/MultiModalPredictor.*
domain/collision/EecbsCoordinator.*
domain/map/RegulatoryTypes.h
domain/map/TrafficLightRuntime.*          # 或 map/regulatory/*
domain/control/NonlinearKinematicMpc.*
扩展 OsmLaneletImporter / Scenario*       # stop_line / traffic_light 字段
扩展 SimEngine.*                          # perception tick、eecbs、multimodal、nmpc
ui/panels/PerceptionPanel.*
ui/dialogs/pages/PerceptionPage.*         # 新 Tab
ui/dialogs/pages/MultiModalPredictionPage.*
ui/dialogs/pages/RegulatoryPage.*
ui/dialogs/pages/NonlinearControlPage.*
ui/graphics/LidarScanGraphicsItem.*       # 可选只读叠加
assets/scenarios/perception_lidar_demo/
assets/scenarios/eecbs_demo/
assets/scenarios/traffic_light_demo/
assets/scenarios/multimodal_prediction_demo/  # 可与 lidar demo 合并，但字段须可测
```

**数据流（示意）：**

```
静态 Map + 动态他车 ──LidarRayCaster──► Scan
                                      │
                         ObstacleLayer mark/clear
                                      │
                                      ▼
                            Planning OccupancyGrid ──► A*/Hybrid/CBS/EECBS
动态他车状态 ──MultiModalPredictor──► 多模态轨迹 ──► ST-Graph / 占用
Regulatory(stop_line, light) ──TrafficLightRuntime──► 停车约束 / BT 条件（可选）
参考路径 ──NonlinearKinematicMpc──► δ, a（或等价控制）──► Bicycle/DiffDrive
```

---

## §3 优先阅读的程序（按优先级）

### P0 — 动手前必读

| 文件 | 为什么 |
|------|--------|
| `domain/collision/CbsLiteCoordinator.*` | EECBS 扩展基座；对照禁止换皮 |
| `domain/prediction/ConstantVelocityPredictor.*` | 多模态必须可区分于 CV |
| `domain/control/MpcLateralTracker.*` / `DenseQpSolver.*` | NMPC 线性化求解复用 |
| `domain/map/OccupancyGrid.*` | mark/clear / inflate 接口 |
| `domain/map/OsmLaneletImporter.*` / `LaneGraph.*` | regulatory 扩展与 LaneKeep |
| `domain/SimEngine.cpp` | tick / prediction / coordination / tracker 注入点 |
| `ui/dialogs/pages/ControlPage.*` / `SpeedPage.*` / `CoordinationPage.*` | **勿再堆** |
| `ui/panels/MapEditorPanel.*` / `MonitorPanel.*` | **勿塞感知表单** |
| `tools/verify_phase10_evidence.py` | 仿写 verify_phase11 |

### P1 — 场景与 App

| 文件 | 为什么 |
|------|--------|
| `app/SimController.cpp` | applyScenario；新字段 |
| `ui/MainWindow.cpp` | 仅挂载新 dock |
| `scenario/ScenarioLoader.h` / `ScenarioSerializer.cpp` | 新字段 |
| `assets/scenarios/cbs_lite_demo/` | CBS 回归对照 |
| `assets/scenarios/prediction_st_demo/` | CV 预测回归 |
| `assets/scenarios/mpc_st_demo/` | 线性 MPC 回归 |

### P2 — 防回归

| ID | 对策 |
|----|------|
| P10-CBS | CbsLiteCoordinatorTest / cbs_lite_demo 仍绿 |
| P10-OSM-BT | osm_lanelet_demo / Bt* / MultiBt* 仍绿 |
| P9-BT-JSON | bt_navigation_demo 仍绿 |
| P8-Lane | lane_routing_demo / LaneGraph 仍绿 |
| P7-Pred | prediction_st_demo；默认 CV 行为 |
| P6-MPC | MpcLateralTrackerTest；tracker=mpc 仍可用 |
| P5-Priority | coordination=priority 默认可用 |

---

## §4 网络调研摘要（执行 Agent 必读 — 算法必须按此实现）

> 下列结论来自公开论文/官方文档摘要；FleetSim 实现的是**教学子集**，不是论文全文移植。

### 4.1 EECBS（AAAI 2021）与对称推理

**来源**：Li, Ruml, Koenig — *EECBS: A Bounded-Suboptimal Search for Multi-Agent Path Finding*（AAAI 2021）；参考实现 [Jiaoyang-Li/EECBS](https://github.com/Jiaoyang-Li/EECBS)；对称推理见 Li et al. *Pairwise Symmetry Reasoning for MAPF*（AIJ / ICAPS）。

**工业要点**：

| 概念 | 含义 |
|------|------|
| ECBS | 用 focal search 加速 CBS，保证解代价 ≤ w · OPT |
| EECBS | 高层用 **Explicit Estimation Search (EES)**：维护 OPEN / FOCAL / CLEANUP；用在线学习的 inadmissible 估计 ĥ 选点 |
| Bypass | 冲突时尝试改一条路径消冲突，成功则**不**分裂约束树 |
| Rectangle symmetry | 4-邻接网格上两车最短路在矩形区域内必然相撞；用 **barrier constraints** 一次分支消除指数组合 |
| Corridor / Target | 窄道对向 / 目标点阻挡对称 — **Phase 11 不做完整版** |

**FleetSim Phase 11 `EecbsCoordinator` 最低真实现**：

1. 保留 CbsLite 的冲突检测 + 约束树 + 底层时空重规划。  
2. 引入次优因子 `eecbs_w`（w≥1）；节点选择体现 EES 三表思想的**可测简化**（ADR 写清：例如 CLEANUP 按 lb、FOCAL 按 inadmissible 估计、OPEN 按 f̂）。  
3. **Bypass**：若对冲突 agent 重规划得到无冲突解且代价可接受，则采用 bypass，不计一次约束分裂。  
4. **Rectangle**：在栅格上检测简单矩形对称冲突，加入 barrier 约束集合（禁止在矩形对边于特定时间片占格）。  
5. `coordination=eecbs`；`cbs_lite` / `priority` / `none` 仍可用。  
6. 深度/时间上限保留；失败可回退 cbs_lite 或 priority（ADR 写清，须可测）。

禁止：只改枚举字符串；与 CbsLite 输出路径位码在构造冲突场景下永远相同且无 bypass/barrier 计数。

### 4.2 Nav2 Costmap ObstacleLayer（2D 射线）

**来源**：[Nav2 Obstacle Layer](https://docs.nav2.org/configuration/packages/costmap-plugins/obstacle.html)、[Costmap 2D](https://docs.nav2.org/configuration/packages/configuring-costmaps.html)。

**工业要点**：

- `marking`：激光命中 → 障碍格  
- `clearing`：沿射线 **raytrace** 清除 free（到命中点之前）  
- `obstacle_max_range` / `raytrace_max_range` 可不同  
- 与 InflationLayer 分层；FleetSim 可复用既有 `inflate`

**FleetSim 映射**：

| Nav2 | FleetSim |
|------|----------|
| LaserScan | `LidarScan`（角度、ranges，仿真生成） |
| ObstacleLayer mark | `ObstacleLayer::markHits` |
| ObstacleLayer clear | `ObstacleLayer::clearAlongRays` |
| 合并 master costmap | `mergeInto(OccupancyGrid&)` 或 SimEngine 维护 `perception_grid_` |
| 真机驱动 | **不做**；由地图障碍 + 他车 AABB/圆近似生成命中 |

### 4.3 多模态预测（非神经网络）

**来源**：IMM + CTRA 文献（如 IMM-CTRA 车辆状态估计、SAE/运动-机动融合预测综述）；教学上区分 **physics-based** 与 **maneuver-based**。

| 模态 | 模型 | 适用 |
|------|------|------|
| CV | 常速直线（Phase 7 已有） | 短时 |
| CTRA | 常转率 + 常加速度 | 转弯/加减速 |
| LaneKeep | 沿 LaneGraph 中心线前进 | 结构化道路较长时域 |

**IMM-lite（FleetSim）**：

- 不强制完整 Kalman IMM；允许：固定或启发式权重 → 输出 `vector<PredictedTrajectory>`（每模态一条）+ 可选融合主轨迹  
- ST-Graph 应对**多模态占用取并集或加权**（ADR 选一种并单测锁死）  
- `prediction=none|constant_velocity|multimodal`

禁止：神经网络；禁止「多模态」但只返回 1 条与 CV 完全相同的轨迹且测试无法区分。

### 4.4 Lanelet2 / Autoware Regulatory（交通灯）

**来源**：[Autoware lanelet2_format_extension — TrafficLights](https://tier4.github.io/autoware.iv/tree/main/map/lanelet2_extension/docs/lanelet2_format_extension/)。

**工业要点**：`regulatory_element` + `subtype=traffic_light`；`ref_line`=停车线；`refers`=灯具 LineString。

**FleetSim 教学子集**：

- 支持 scenario JSON 显式 `traffic_lights[]` / `stop_lines[]`（优先，易测），**可选** OSM relation 扩展  
- 运行时：`TrafficLightRuntime` 按 `schedule` 或周期切换 red/yellow/green  
- 语义：红灯 → 自车不得越过关联 stop_line（速度规划置 0 或 goal 夹在线前）  
- **不做**：light_bulbs 识别、行人过街全语义、detection area

### 4.5 非线性运动学 NMPC（零 CasADi）

**来源**：NMPC + bicycle 文献普遍用 CasADi/IPOPT；FleetSim **禁止**引入。教学替代：

1. 离散非线性运动学自行车：`(x,y,θ,v)` + 控制 `(a, δ)` 或 `(v, δ)`  
2. 在参考路径上做 **迭代线性化（iLQR / SQP-like）**：每迭代线性化动力学 → QP（复用 `DenseQpSolver`）→ 更新控制序列  
3. 或 multiple shooting 短时域（N≤15）手工高斯-牛顿  

验收：在带曲率路径上，`nonlinear_mpc` 的横偏显著小于「零舵角开环」；与线性 `mpc` 可并存切换。

### 4.6 UI 分层启示（防 Goal Agent 堆控件 — **本章最重要**）

| 层次 | 用户心智 | FleetSim 落点 | 禁止 |
|------|---------|--------------|------|
| 静态障碍编辑 | 画矩形 | MapEditorPanel | 塞 Lidar 参数 |
| HD Map 导入 | .osm | OsmImportPanel / MapImportPage | 塞交通灯时序表单 |
| **感知** | 激光看见什么 | **PerceptionPanel + PerceptionPage** | 塞进 Monitor/MapEditor |
| 速度规划 | ST | SpeedPage | 塞多模态权重 |
| **多模态预测** | 他车假设 | **MultiModalPredictionPage** | 塞进 SpeedPage/ControlPage |
| 线性跟踪 | Stanley/MPC | ControlPage | 继续堆 NMPC 大表单 |
| **非线性跟踪** | NMPC 参数 | **NonlinearControlPage** | 与 ControlPage 混堆 |
| 协调 | priority/cbs/eecbs | CoordinationPage（仅增 eecbs） | 塞感知/灯控 |
| **监管/灯控** | 灯态与停车线 | **RegulatoryPage** + 可选灯态 overlay | 塞进 Routing/MapImport |
| 实验对比 | 指标 CSV | ExperimentComparePanel | 加感知表单 |

**Reviewer 量化红线**：

- `MapEditorPanel.cpp` / `LaneEditorPanel.cpp` / `OsmImportPanel.cpp` 出现 lidar ray 参数表单 → **FAIL**  
- `SpeedPage.cpp` 出现 `multimodal|ctra|imm` → **FAIL**  
- `ControlPage.cpp` 出现 `nonlinear_mpc|ilqr|horizon_nmpc` → **FAIL**  
- `CoordinationPage.cpp` 出现 `lidar|traffic_light|prediction` → **FAIL**  
- `MapImportPage.cpp` / `RoutingPage.cpp` 出现交通灯时序编辑 → **FAIL**  
- 任一新 Panel/Page `.cpp` >300 行未拆分 → **FAIL**  
- MainWindow 新增业务逻辑 >15 行/功能 → **FAIL**

---

## §5 设计决策建议（先写 ADR 再写码）

### ADR-026 — 2D Lidar 感知层

```json
"simulation": {
  "perception": "none",
  "lidar_range_m": 8.0,
  "lidar_ray_count": 36,
  "lidar_fov_deg": 180.0
}
```

- `perception`: `none`（默认）| `lidar`  
- 扫描在 `SimEngine::tick` 中更新；规划可选使用 `planningGrid()` = static ⋃ perception  

### ADR-027 — 多模态预测

```json
"simulation": {
  "prediction": "constant_velocity",
  "prediction_horizon_s": 2.0,
  "multimodal_weights": { "cv": 0.3, "ctra": 0.3, "lane_keep": 0.4 }
}
```

- `prediction`: `none` | `constant_velocity` | `multimodal`  
- 输出结构含 `modes[]`；ST 占用策略写死  

### ADR-028 — EECBS 教学版

```json
"simulation": {
  "coordination": "priority",
  "eecbs_w": 1.5,
  "eecbs_max_depth": 20,
  "eecbs_time_limit_ms": 200,
  "eecbs_enable_bypass": true,
  "eecbs_enable_rectangle": true
}
```

- `coordination`: `none` | `priority` | `cbs_lite` | `eecbs`  
- 与 CbsLite **并存**；禁止删除 CbsLite  

### ADR-029 — TrafficLight / StopLine

```json
"regulatory": {
  "stop_lines": [{ "id": "sl0", "x1": 8, "y1": 4, "x2": 8, "y2": 6 }],
  "traffic_lights": [{
    "id": "tl0",
    "stop_line_id": "sl0",
    "schedule": [
      { "state": "red", "duration_s": 5 },
      { "state": "green", "duration_s": 5 }
    ]
  }]
}
```

### ADR-030 — Nonlinear Kinematic MPC + UI IA

- `tracker`: 增加 `nonlinear_mpc`；auto 规则不强制选它  
- UI：PerceptionPanel / PerceptionPage / MultiModalPredictionPage / RegulatoryPage / NonlinearControlPage 职责表（§4.6）  

---

## §6 建议新增/变更文件清单

### Domain

| 文件 | 职责 |
|------|------|
| `perception/LidarRayCaster.*` | 仿真扫描 |
| `perception/ObstacleLayer.*` | mark/clear/merge |
| `prediction/CtraPredictor.*` | CTRA |
| `prediction/LaneKeepPredictor.*` | 车道外推 |
| `prediction/MultiModalPredictor.*` | IMM-lite 组合 |
| `collision/EecbsCoordinator.*` | EECBS 教学 |
| `map/RegulatoryTypes.h` + `TrafficLightRuntime.*` | 灯与停车线 |
| `control/NonlinearKinematicMpc.*` | 非线性 NMPC |
| 扩展 `SimEngine.*` / `scenario/*` / 可选 `OsmLaneletImporter` | 字段与 tick |

### UI

| 文件 | 职责 |
|------|------|
| `panels/PerceptionPanel.*` | 感知 dock |
| `dialogs/pages/PerceptionPage.*` | Workbench 新 Tab |
| `dialogs/pages/MultiModalPredictionPage.*` | 新 Tab |
| `dialogs/pages/RegulatoryPage.*` | 新 Tab |
| `dialogs/pages/NonlinearControlPage.*` | 新 Tab |
| 扩展 `CoordinationPage.*` | 仅 eecbs |
| 扩展 `AlgorithmWorkbenchDialog.*` | 注册新 Tab |
| 可选 `graphics/LidarScanGraphicsItem.*` | 射线叠加 |

### 测试 / 文档 / 工具 / 资产

```
tests/domain/LidarPerceptionTest.cpp
tests/domain/MultiModalPredictorTest.cpp
tests/domain/EecbsCoordinatorTest.cpp
tests/domain/TrafficLightRuntimeTest.cpp
tests/domain/NonlinearMpcTest.cpp
tests/integration/PerceptionLidarDemoScenarioTest.cpp
tests/integration/EecbsDemoScenarioTest.cpp
tests/integration/TrafficLightDemoScenarioTest.cpp
docs/decisions/026-lidar-obstacle-layer.md
docs/decisions/027-multimodal-prediction.md
docs/decisions/028-eecbs-teaching.md
docs/decisions/029-traffic-light-regulatory.md
docs/decisions/030-nonlinear-mpc-ui-ia.md
tools/verify_phase11_evidence.py
tools/run_phase11_verify.ps1
assets/scenarios/perception_lidar_demo/
assets/scenarios/eecbs_demo/
assets/scenarios/traffic_light_demo/
```

---

## §7 建议 9 会话任务拆解

| Session | 目标 | 关键验收 | NOT DO |
|---------|------|----------|--------|
| 0 | ADR-026…030 + 红灯测 | 测例编译但 FAIL | 不写假 PASS |
| 1 | Lidar + ObstacleLayer | LidarPerceptionTest 绿；影响规划可测 | 不引 ROS 激光；不改 UI |
| 2 | CTRA + LaneKeep + IMM-lite | MultiModalPredictorTest；与 CV 可区分 | 不上 NN |
| 3 | EECBS 核心 + w | EecbsCoordinatorTest；可切换 | 不删 CbsLite |
| 4 | Bypass + Rectangle | 冲突场景断言 bypass/barrier 计数 | 不做 corridor 全套 |
| 5 | TrafficLight + StopLine | 红停绿行 | 不做视觉识灯 |
| 6 | NonlinearKinematicMpc | NonlinearMpcTest；线性 MPC 回归 | 不引 CasADi |
| 7 | UI 五件套分层 | 负向审计全过 | 单页堆控件 |
| 8 | demo + verify≥70 + M47+ + Phase11 ✅ | 全绿 | 跳回归 |

---

## §8 测试要求（Tester 强制）

| 测试 | 最低断言 |
|------|----------|
| `LidarPerceptionTest` | 墙前 ranges 有限命中；空地无命中或 max_range；mark 增占用；clear 减占用 |
| `MultiModalPredictorTest` | multimodal 返回 ≥2 modes；CTRA 转弯与 CV 终点偏离 ≥ε；LaneKeep 沿 lane |
| `EecbsCoordinatorTest` | 对撞场景 success；`w` 影响或可记录；bypass_count 或 rectangle_count >0（构造场景）；cbs_lite 对照路径可不同 |
| `TrafficLightRuntimeTest` | 红灯时越过 stop_line 被拒绝或速度为 0；绿灯可规划通过 |
| `NonlinearMpcTest` | 控制序列非全零；跟踪误差有上界；切换 tracker 不崩 |
| Demo 集成测 | 各 demo 可 loadScenario；关键字段正确；短时 tick 不崩 |
| 回归 | Phase10 Cbs*/Osm*/Bt*；Phase9 Behavior*；Phase8 Lane*；Phase7 Experiment*/CV*；Phase6 Mpc* |

**假实现检测**：

- Lidar 恒空 / 恒 max_range 无几何关系 → FAIL  
- MultiModal modes.size()==1 且与 CV 点列相同 → FAIL  
- EECBS 与 CbsLite 在 rectangle 场景约束计数皆 0 且路径相同 → FAIL  
- 交通灯 UI 有、车辆红灯仍全速过线 → FAIL  
- NonlinearMpc 直接 `return MpcLateralTracker::...` → FAIL  

---

## §9 CMake / 构建

- Domain 新源必须进 `src/domain/CMakeLists.txt`  
- UI 进 `src/ui/CMakeLists.txt`  
- Tests 进 `tests/CMakeLists.txt`  
- ASCII 外置：`D:\build\FleetSim_phase11_s0`  
- 用户中文路径 Qt：Agent **不**代跑；交付验证步骤  

---

## §10 文档与变异

| 交付 | 说明 |
|------|------|
| ADR-026…030 | Session 0 草案 → Session 8 已接受 |
| DEVELOPMENT_PLAN | Phase **11** ✅ |
| learning-path.md | 更新 Phase 11 行 |
| MUTATION M47–M52 | 见 §19 |
| verify_phase11_evidence.py | ≥70 静态检查 |
| SESSION_LOG | 每会话四角色 |

---

## §11 Reviewer 检查单（每会话）

### 算法真实性
- [ ] 12 项算法清单均有实现文件与 TEST  
- [ ] Lidar mark/clear 可区分  
- [ ] MultiModal ≥2 模态可区分 CV  
- [ ] EECBS 有 w +（bypass 或 rectangle）可观测  
- [ ] 红灯停车语义  
- [ ] NMPC 非平凡控制  

### UI
- [ ] Perception ≠ MapEditor ≠ Monitor  
- [ ] MultiModalPredictionPage ≠ SpeedPage  
- [ ] NonlinearControlPage ≠ ControlPage  
- [ ] RegulatoryPage ≠ MapImportPage  
- [ ] CoordinationPage 仅协调参数  
- [ ] 无单页控件堆叠  

### 回归 / 工程
- [ ] Domain 零 Qt  
- [ ] CMake 完整  
- [ ] Phase 7–10 测绿  
- [ ] SESSION_LOG 完整  

---

## §12 用户本地验证清单（每会话末尾交给用户）

1. `git pull origin main`  
2. ASCII：`cmake` + Build `FleetSimTests` + 运行  
3. `python tools/verify_phase11_evidence.py`（Session 8 起；中间会话可先跑子集）  
4. 打开 `perception_lidar_demo` → MapView 可见射线/动态占格（若已做 overlay）  
5. `eecbs_demo` → 多车无长时间卡死；可与 `cbs_lite_demo` 对比  
6. `traffic_light_demo` → 红停绿行  
7. 回归：`cbs_lite_demo`、`bt_navigation_demo`、`lane_routing_demo`、`prediction_st_demo`、`mpc_st_demo`  

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
| EECBS 组合爆炸 | w + depth + time_limit；失败回退 |
| 射线性能 | ray_count 默认 ≤72；单测小地图 |
| LaneKeep 无车道 | 无 lane 时降级 CV/CTRA 并测 |
| NMPC 不收敛 | 迭代上限 + 回退上一控制；单测短 horizon |
| UI Agent 堆控件 | §4.6 红线 + verify 负向检查 |
| 中文路径 Build | 用户本地；Agent ASCII |

---

## §15 Goal 模式启动短指令（可选第二段）

```
执行 docs/PHASE11_GOAL_PROMPT.md。
Goal：完成 Phase 11（Lidar 感知层 + 多模态预测 + EECBS 教学版 + 交通灯监管 + 非线性 NMPC + UI 分层）。
强制四角色子 Agent；12 项算法全部真实现 + GTest；verify_phase11 ≥70；每会话 push。
从会话 0 开始。禁止堆 UI。禁止 stub。禁止 CasADi / 神经网络。
```

---

## §16 Planner mini-plan 模板（每会话必填）

```markdown
**Phase**: 11
**本会话目标**: （一句话）
**允许改动**: （目录/文件）
**NOT DO**: （≥10 条，含禁止堆 UI / 假 Lidar / CV 冒充多模态 / CbsLite 冒充 EECBS / CasADi / 破坏 Phase10）
**预计新增/修改文件**:
**计划测试**（精确 TEST 名）:
**UI 分页验收**:
  - 必须新建/修改: （点名 Panel/Page）
  - 禁止污染: MapEditorPanel / LaneEditorPanel / OsmImportPanel / ControlPage / SpeedPage /
    CoordinationPage / MapImportPage / BehaviorPage / MonitorPanel / ExperimentComparePanel
**验收标准**:
**四角色**: Planner / Executor / Tester / Reviewer 结论占位
```

---

## §17 Phase 12 展望（勿在 Phase 11 实现）

- Corridor / Target 全套对称推理；更完整 EES 在线学习 ĥ  
- 3D Voxel / 点云感知  
- 学习型/多模态神经网络预测  
- CasADi/ACADOS 级 NMPC 或动态轮胎模型  
- 完整 Lanelet2 regulatory / 过街 / right_of_way  
- Groot2 / 完整 BT.CPP 插件生态  

---

## §18 verify_phase11_evidence.py 检查项（Session 8 必须实现）

静态脚本至少覆盖（仿 Phase 10，目标 **≥70 PASS**）：

| 分组 | 检查项 |
|------|--------|
| A Perception | LidarRayCaster.*、ObstacleLayer.*、LidarPerceptionTest |
| B Multimodal | CTRA/LaneKeep/MultiModalPredictor.*、MultiModalPredictorTest |
| C EECBS | EecbsCoordinator.*、EecbsCoordinatorTest、coordination eecbs、bypass/rectangle 关键字 |
| D Regulatory | TrafficLightRuntime.*、TrafficLight*Test、demo 资产 |
| E NMPC | NonlinearKinematicMpc.*、NonlinearMpcTest |
| F UI IA | PerceptionPanel、PerceptionPage、MultiModalPredictionPage、RegulatoryPage、NonlinearControlPage |
| G 负向 | MapEditor/SpeedPage/ControlPage/CoordinationPage/MapImport 无对应污染 |
| H demo | perception / eecbs / traffic_light demo |
| I CMake | 新源登记；Domain 无 Qt |
| J ADR | 026–030、M47+、Phase 11 marked |
| K 回归 | Phase10 verify 关键资产仍在；cbs_lite_demo / bt_navigation_demo / prediction_st_demo |
| L 工具 | verify_phase11_evidence.py、run_phase11_verify.ps1 |

---

## §19 MUTATION M47–M52（Session 8 登记）

| ID | 注入点 | 预期 FAIL 测例 |
|----|--------|----------------|
| M47 | `LidarRayCaster` — 恒返回 max_range / 空 hit | `LidarPerceptionTest.*` |
| M48 | `ObstacleLayer::clearAlongRays` — 空操作 | `LidarPerceptionTest.Clear*` |
| M49 | `MultiModalPredictor` — 只返回 CV 单模态 | `MultiModalPredictorTest.*` |
| M50 | `EecbsCoordinator` — 禁用 bypass 且 rectangle 恒 false，行为同 CbsLite 换皮 | `EecbsCoordinatorTest.*` |
| M51 | `TrafficLightRuntime` — 红灯仍放行 | `TrafficLightRuntimeTest.*` |
| M52 | `NonlinearKinematicMpc` — 控制量恒 0 | `NonlinearMpcTest.*` |

---

## §20 算法实现对照表（Reviewer 终审用）

| # | 算法 | 实现类/文件 | 单测 | 完成 |
|---|------|------------|------|------|
| 1 | 2D Lidar 射线投射 | LidarRayCaster | LidarPerceptionTest | ☐ |
| 2 | ObstacleLayer mark | ObstacleLayer | 同上 | ☐ |
| 3 | ObstacleLayer clear | ObstacleLayer | 同上 | ☐ |
| 4 | 感知合并规划栅格 | ObstacleLayer / SimEngine | LidarPerceptionTest / 集成 | ☐ |
| 5 | CTRA 预测 | CtraPredictor | MultiModalPredictorTest | ☐ |
| 6 | LaneKeep 模态 | LaneKeepPredictor | 同上 | ☐ |
| 7 | IMM-lite 多模态 | MultiModalPredictor | 同上 | ☐ |
| 8 | EECBS EES 选点 + w | EecbsCoordinator | EecbsCoordinatorTest | ☐ |
| 9 | Bypass conflicts | EecbsCoordinator | 同上 | ☐ |
| 10 | Rectangle barrier | EecbsCoordinator | 同上 | ☐ |
| 11 | 交通灯 + stop_line | TrafficLightRuntime | TrafficLightRuntimeTest | ☐ |
| 12 | 非线性运动学 NMPC | NonlinearKinematicMpc | NonlinearMpcTest | ☐ |

**全部勾选 + verify ≥70 PASS + FleetSimTests 全绿 = 才允许 UpdateGoal complete。**

---

## §21 调研参考链接（Agent 可再查阅，禁止用「网上有」代替实现）

| 主题 | 链接 |
|------|------|
| EECBS AAAI 2021 | https://ojs.aaai.org/index.php/AAAI/article/view/17466 |
| EECBS 参考代码 | https://github.com/Jiaoyang-Li/EECBS |
| MAPF 对称推理 | https://doi.org/10.48550/arxiv.2103.07116 |
| Nav2 Obstacle Layer | https://docs.nav2.org/configuration/packages/costmap-plugins/obstacle.html |
| Nav2 Costmap 2D | https://docs.nav2.org/configuration/packages/configuring-costmaps.html |
| Autoware Traffic Light 扩展 | https://tier4.github.io/autoware.iv/tree/main/map/lanelet2_extension/docs/lanelet2_format_extension/ |
| 轨迹预测综述（物理/机动） | https://pmc.ncbi.nlm.nih.gov/articles/PMC12390385/ |

---

*文档版本：Phase 11 · 2026-08-24 · 前置 Phase 10 Session 7（196/196 绿，verify_phase10 102 PASS）*  
*调研来源：EECBS（AAAI 2021）、CBSH 对称推理、Nav2 costmap ObstacleLayer、Autoware Lanelet2 traffic_light、IMM/CTRA 运动学多模态预测、NMPC bicycle（教学自研替代 CasADi）*
