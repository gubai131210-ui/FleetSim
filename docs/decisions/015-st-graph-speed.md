# ADR 015: ST-Graph 速度规划 + SpeedProfile

## 状态

草案（2026-08-23，Phase 6 Session 0；Session 3 实装 ST；Session 4 引擎强制接线）

## 背景

FleetSim 已有空间路径（A*/Hybrid）与运行时避碰（TimeWindow + Reservation）及路径层 Priority（ADR-013）。缺的是**沿参考线的显式纵向速度规划**：把他车已知轨迹投影到自车路径的 \((s,t)\) 图上，输出可执行速度剖面。Phase 6 做教学规模 ST-Graph MVP。

## 决策

### 1. 输出类型（写死 — 推荐选型）

新增并列于 `Path` 的 Core 类型（**不**改 `Waypoint` 字段）：

```cpp
namespace fleetsim::core {
struct SpeedProfile {
    std::vector<double> speeds;         // m/s，与 Path waypoints 等长
    std::vector<double> arrival_times;  // s，自路径起点累计到达时间，等长
};
}
```

| 约定 | 规则 |
|------|------|
| 等长 | `speeds.size() == arrival_times.size() == path.waypoints().size()` |
| 空 Path | 空剖面（两向量皆空） |
| 语义 | `speeds[i]` = 在 waypoint \(i\) 处的期望纵向速度；`arrival_times[i]` = 到达该点的计划时刻（`arrival_times[0]=0`） |
| 禁止 | 仅改 `agent.speed_scale` 冒充 ST 交付物 |
| 禁止 | 给 `Waypoint` 加速度/时间字段却不更新序列化与所有消费者 |
| 不做 | `TimedPath` 包装（本阶段不选备选） |

### 2. `StGraphSpeedPlanner` 接口

```cpp
class StGraphSpeedPlanner {
public:
    // v_max, a_max, dt_grid, ...
    core::SpeedProfile plan(const core::Path& ego_path,
                            const std::vector<PeerTrajectory>& peers) const;
};
```

- 输入：自车 `Path`；他车已知 `Path` + 名义速度（和/或可从 `PathReservationTable` 投影的占用），经 `PeerTrajectory`（或等价结构）传入。
- 内部：沿自车路径累积弧长 \(s\)；建 \((s,t)\) 栅格/采样；他车轨迹与自车路径求交 → ST 占用块；在可行走廊内规划满足 \(v,a\) 限幅的速度剖面。
- **禁止**：欧氏距离 \(< d\) 就停车冒充 ST。

### 3. SimEngine 接线合同（写死；Session 4 强制实现）

| `speed_planner` | 行为 |
|-----------------|------|
| `none`（默认） | 不建 ST；跟踪器用 cruise / 既有逻辑 |
| `st_graph` | **`plan()` 成功后、首次跟踪前至少调用一次** ST；把 `SpeedProfile` 交给 tracker（`setSpeedProfile`，见 ADR-014） |

- 若他车 Path/预约在仿真中更新：**每 N tick 重算**，默认 **N=10**；与仿真 `dt` 一并在 Settings/scenario 可配（Session 4 落地字段）。
- **禁止**：ST 类已实现但 SimEngine 从不调用。
- **禁止**：仅用 `speed_scale` 作为 Phase 6 ST 验收交付物。
- TimeWindow 速度缩放可**叠加**在 ST 剖面之上；不可用 TimeWindow **替代** ST。
- Priority 仍负责几何走廊顺序（ADR-013）；ST 负责沿路径纵向。

### 4. 最低接线测（Session 4）

- 两车场景：ST 开启时后车在冲突段减速/等待。
- **人为清空他车障碍输入** → 剖面应变「更激进」；或测试在「忽略他车」配置下 **FAIL**（证明接线读了他车）。
- 单元：空障碍 → 近匀速 cruise；他车横穿 → 减速段非距离停车；`ClearingPeerObstaclesChangesProfile`。

### 5. 默认参数

| 参数 | 默认 |
|------|------|
| ST \(\Delta t\) | 0.1～0.2 s |
| \(v_{\max}\) | 0.5 m/s |
| \(a_{\max}\) | 0.5～1.0 m/s² |
| 重算周期 N | 10 ticks |

### 6. 明确不做

- CBS-lite / EECBS 最优 MAPF（Priority 已够教学）
- 完整感知预测网络（他车用**已知仿真轨迹**）
- 联合横纵向大系统 MPC

## 后果

- 正面：路径/速度分层清晰；可与 TimeWindow/Priority 叠加教学
- 负面：ST 栅格分辨率与算力权衡；双重减速可能过猛 → 单测锁速度地板

## 禁止

- 禁止 ST = 距离停车
- 禁止只写类不接线 / 只用 `speed_scale` 冒充交付
- 禁止「写 ADR 说不做」规避读他车的最低接线合同
- 禁止破坏 TimeWindow / Priority 无本 ADR 修订 + 回归测

## 参考

- CMU s–T / temporal optimization；MPQP + ST graph（arXiv 2401.06305）
- ADR-013、ADR-003
- `docs/PHASE6_GOAL_PROMPT.md` §4.2 / §5
