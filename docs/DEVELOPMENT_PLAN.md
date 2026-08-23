# FleetSim 开发计划（共识版）

完整决策见各 ADR。阶段概览：

| Phase | 目标 | 预估 |
|-------|------|------|
| **0** | Harness、CMake 模块化、文档、SVG ADR | 2–3 天 |
| **1** | 单车 A* + 平滑 + Pure Pursuit + DiffDrive + MapView | 2 周 |
| **2** | 地图编辑器 + JSON 持久化 + QCustomPlot 监控 + CRAP Harness | 1.5 周 ✅ |
| **3** | 多 AGV + 贪心调度 + 时间窗口避碰 | 3 周 ✅ |
| **4** | Bicycle 模型 + Domain 静态库 + ROS2 桥接 | 3 周 ✅ |
| **5** | Hybrid A* + Stanley 跟踪 + Priority/CBS-lite 多车协调 | 3 周 ✅ |
| **6** | 线性 MPC 横向跟踪 + ST-Graph 速度规划 | 3 周 ✅ |
| **7** | 常速轨迹预测增强 ST + 算法实验工作台 UI 分层 + 对比指标 | 3 周 ✅ |

> Phase 5 完整 Goal 提示词：`docs/PHASE5_GOAL_PROMPT.md`  
> Phase 6 完整 Goal 提示词：`docs/PHASE6_GOAL_PROMPT.md`  
> Phase 7 完整 Goal 提示词：`docs/PHASE7_GOAL_PROMPT.md`（复制 §0 给 Goal Agent）。  
> Phase 6 验收：ASCII 外置 Build `D:\build\FleetSim_phase6*` + `FleetSimTests` 全绿（用户本地 Qt 再证）。  
> Phase 7 验收：ASCII 外置 Build `D:\build\FleetSim_phase7*` + `FleetSimTests` 全绿 + `tools/verify_phase7_evidence.py`（会话 7 交付）。

## 禁止偷懒清单（Reviewer 必查）

### 会话交付
1. 禁止 SESSION_LOG 只写「完成了功能」— 必须按 `docs/AGENT_SESSION_TEMPLATE.md` 填写
2. 禁止「没做什么」留空 — 必须列明原因和计划 Phase
3. 禁止回复用户时不写「做了什么/没做什么」

### 代码结构
4. 禁止把所有 UI 堆进 MainWindow
5. 禁止 Domain 层 include Qt
6. 禁止多类挤在同一文件
7. 禁止不写单测（Core/Domain 新类）
8. 禁止跳过 Reviewer

### UI 专项
9. 禁止用 QToolBar 承载业务控件 — 用 `src/ui/panels/`
10. 禁止 centralWidget 放表单/曲线 — 只能是 MapView
11. 禁止把 Monitor + Control + Task 合并成一个 Panel
12. 禁止新增 UI 不更新 CMakeLists.txt

### 功能质量
13. 禁止硬编码地图在 C++ 里
14. 禁止跨 Phase 实现功能
15. 禁止 A* 折线不经 Smoother 直接跟踪
16. 禁止 Phase 3 用简单距离判碰糊弄

## Agent 会话 Prompt 模板

```
读 AGENTS.md + SESSION_LOG.md + docs/AGENT_SESSION_TEMPLATE.md
→ Architect mini-plan（含 NOT DO）
→ Implementer
→ Reviewer（含 UI_GUIDELINES）
→ Scribe 更新 SESSION_LOG
→ 回复用户四段式摘要
```

## UI 面板 Phase 规划

见 `docs/UI_GUIDELINES.md` 与 `src/ui/panels/README.md`。
