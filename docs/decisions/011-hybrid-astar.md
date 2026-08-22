# ADR 011: Hybrid A* 非完整约束规划

## 状态

草案（2026-08-23，Phase 5 Session 0；Session 1–2 实装）

## 背景

Phase 1–4 使用栅格 `AStarPlanner`（状态仅 `(x,y)`）。自行车模型有最小转弯半径 \(R_{\min}=L/\tan\delta_{\max}\)，栅格折线路径常违反运动学，窄弯场景易「切角」或不可行。Phase 5 需要与 A* 并存的 Hybrid A*。

## 决策

### 1. 接口保持 `IPathPlanner`

```cpp
core::Path plan(const map::OccupancyGrid& grid,
                const core::Pose& start,
                const core::Pose& goal) const;
```

- 新增 `HybridAStarPlanner : IPathPlanner`
- 运动学参数经**构造注入**：`wheelbase_m`、`max_steering_rad`、可选 `motion_resolution_m`、`analytic_expand_every_n`
- **不**为 Hybrid 单独改虚函数签名，避免破坏现有调用点

### 2. 内部状态与扩展（真 Hybrid，禁止 A* 换皮）

- 搜索状态：`(x, y, θ)`（连续位姿 + 离散化哈希用于 visited）
- 后继：离散舵角集合（默认 `[-δmax, 0, +δmax]`，可加密）+ **仅前进**（Dubins MVP）自行车积分一步
- 碰撞：沿积分轨迹采样查 `OccupancyGrid`
- 解析扩展：每 N 次扩展尝试 **Dubins** 直连目标；无碰则提前结束
- Reeds-Shepp（可倒车）：Phase 5 **可选**，默认关闭

### 3. 输出与平滑

- 输出仍为 `core::Path`（`Waypoint` 仅 `x,y`）；稠密采样保留曲率信息
- 朝向：由相邻 waypoint 切线或起点/终点 `Pose.theta` 在 tracker 侧恢复
- **Hybrid 路径默认跳过 Douglas-Peucker**（已较平滑；强制 DP 会削弯并抹掉稠密度）
- A* 路径仍走现有 smoother（行为不变）

### 4. Scenario / 默认策略

| 条件 | 默认 planner |
|------|----------------|
| `model == bicycle` | `hybrid_astar`（可配置覆盖） |
| `model == diff_drive` | `astar` |

字段建议（Session 2）：

```json
"simulation": {
  "planner": "astar" | "hybrid_astar",
  "tracker": "pure_pursuit" | "stanley"
}
```

车辆级覆盖可选：`vehicles[].planner`。

### 5. SimEngine 接线（Session 2）

- 将硬编码 `AStarPlanner planner_` 改为按配置选择（`unique_ptr<IPathPlanner>` 或分支）
- Hybrid 分支：`plan` 后**不**调用 `smoother_.smooth`（或 identity）
- DiffDrive / `planner=astar`：保持 A* + DP

### 6. 失败与超时

- 开集上限 / 时间预算耗尽 → 返回空 `Path`（与 A* 一致）
- 单测覆盖：直道、最小 R 弯、运动学阻塞、窄通道对比（Hybrid vs 栅格 A*）

## 后果

- 正面：bicycle 可规划可行弯道；与 A* 可切换对比
- 负面：搜索成本高于栅格 A*；须限开集与解析扩展
- 不改：`Waypoint` 暂不加 `theta`（避免 Core 大面积回归）；若 Session 6 前仍不够，另开 ADR 扩展

## 禁止

- 禁止「只给 A* 节点加 θ 字段」而不做运动学扩展
- 禁止把栅格中心折线当作 Hybrid 输出冒充验收

## 参考

- MATLAB `plannerHybridAStar`
- Apollo Hybrid A* 文档
- `docs/PHASE5_GOAL_PROMPT.md` §4.1 / §5
