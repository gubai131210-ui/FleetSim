# FleetSim Session Log

## 会话流程

1. 读 `AGENTS.md` + 本文件最新条目
2. Architect → mini-plan
3. Implementer → 代码 + 单测
4. Reviewer → 检查
5. Scribe → 更新本文件
6. 用户本地编译测试

---

## [2026-08-22] Phase 0 — Harness 与工程骨架

**Architect 目标**：模块化 CMake + 核心骨架 + 文档 + SVG 渲染 ADR

**Implementer 完成**：

- [x] 重构 CMake 为 Core / Domain / App / UI / Tests 分层
- [x] FetchContent：Google Test、nlohmann/json、Eigen
- [x] Core：`SimClock`、`EventBus`、基础类型
- [x] Domain：`SimEngine` 骨架；`SchedulingModule`、`CollisionModule` stub
- [x] UI：`MapView`、`VehicleGraphicsItem`（SVG 加载 stub）
- [x] 文档：`AGENTS.md`、ADR 001–004、`learning-path.md`
- [x] 资源：示例 SVG、`map.json`、`scenario.json`
- [x] `.gitignore`、`.cursor/rules/fleetsim.mdc`

**接口变更**：

- 新增 `fleetsim::SimEngine::tick(double dt)`
- 新增 `fleetsim::EventBus::publish/subscribe`
- 新增 `fleetsim::ui::VehicleGraphicsItem`（SVG 车辆图元）

**Reviewer 结果**：PASS（见 Reviewer 子 agent 报告；无 critical/major 问题）

**下次注意（Phase 1）**：

- 从 `OccupancyGrid` + A* 开始
- `MapView` 接 SimEngine EventBus 渲染
- Pure Pursuit + DiffDrive 在 Domain 层实现

**本地验证**（用户执行）：

```
在 Qt Creator 中打开项目，重新配置 CMake，Build → Run Tests
或于 build 目录：cmake --build . && ctest
```
