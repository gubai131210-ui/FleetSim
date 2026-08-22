# FleetSim 开发计划（共识版）

完整决策见各 ADR。阶段概览：

| Phase | 目标 | 预估 |
|-------|------|------|
| **0** | Harness、CMake 模块化、文档、SVG ADR | 2–3 天 |
| **1** | 单车 A* + 平滑 + Pure Pursuit + DiffDrive + MapView | 2 周 |
| **2** | 地图编辑器 + JSON 持久化 + QCustomPlot 监控 | 1.5 周 |
| **3** | 多 AGV + 贪心调度 + 时间窗口避碰 | 3 周 |
| **4** | Bicycle 模型 + Domain 静态库 + ROS2 桥接 | 3 周 |

## 禁止偷懒清单

1. 禁止把所有 UI 堆进 MainWindow
2. 禁止 Domain 层 include Qt
3. 禁止多类挤在同一文件（见 AGENTS.md 行数规范）
4. 禁止不写单测
5. 禁止不更新 SESSION_LOG
6. 禁止跳过 Reviewer
7. 禁止硬编码地图在 C++ 里
8. 禁止跨 Phase 实现功能
9. 禁止 A* 折线不经 Smoother 直接跟踪
10. 禁止 Phase 3 用简单距离判碰糊弄

## Agent 会话 Prompt 模板

见 `AGENTS.md` 会话流程。每次会话开头：

```
读 AGENTS.md + SESSION_LOG.md → Architect mini-plan → Implementer → Reviewer → Scribe
```
