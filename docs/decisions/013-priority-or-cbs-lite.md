# ADR 013: 多车路径协调增强（Priority + TimeWindow）

## 状态

已接受（2026-08-23，Phase 5 Session 0 草案；Session 4 实装 Priority MVP）

## 背景

Phase 3 已有 `PathReservationTable` + `TimeWindowCollisionAvoidance`（预约走廊 + 速度缩放让行）。Phase 4 任务层已有 Greedy/Hungarian。Phase 5 需要**显式路径层优先级协调**，禁止用「距离 &lt; d 停车」冒充 MAPF。

## 决策

### 1. 分层职责（保持）

| 层 | 组件 | 职责 |
|----|------|------|
| 任务分配 | `ITaskAssigner`（Greedy/Hungarian） | 谁做哪单 |
| 单车规划 | A* / Hybrid A* | 几何可行路径 |
| **路径协调（新增）** | `PriorityPathCoordinator` | 按优先级依次规划，低优先级视高优先级已预约轨迹为动态占用 |
| 运行时避碰 | TimeWindow + Reservation | 速度缩放、冲突窗口 |

### 2. Priority MVP（Session 4 必做）

```text
按 priority（车 id 或任务 priority）排序 agents
for each agent in order:
    将更高优先级已占用的时空走廊写入临时障碍 / 扩展 Reservation
    planPathForAgent(...)
    reservePath(...)
```

- 与现有 `reservePath` **协同**：Priority 决定「谁先占走廊」；TimeWindow 仍做 tick 内速度调节
- **不删除** TimeWindow；不改为纯距离判碰

### 3. CBS-lite（可选第二实现）

- 仅当 Priority 不足且时间允许：网格点/边冲突 + **深度限制**的约束树
- Phase 5 **不要求**最优大规模 CBS；可 stub + 本 ADR 指向「未实装」
- 完整 EECBS / 数十车最优：明确范围外

### 4. 替换边界（明确）

| 允许 | 禁止 |
|------|------|
| 在 plan 前按优先级加预约约束 | 用欧氏距离 &lt; d 互相停车代替协调 |
| TimeWindow 参数微调 | 静默删除 Reservation / TimeWindow |
| ADR 更新后迁移策略并加回归测 | 无回归测的「重写避碰」 |

若未来要以 CBS 替换 TimeWindow，必须：新 ADR 修订本文件状态 + `MultiAgvScenarioTest` 绿 + MUTATION M30。

### 5. 配置

- Settings / scenario 可选：`coordination: "priority"|"none"`（默认 `priority` 多车；单车无感）
- 优先级源：任务 `priority` 降序，其次 `vehicle.id` 字典序

## 后果

- 正面：教学规模 3–8 车可解释「谁让谁」；保留 Phase3 质量
- 负面：Priority 次优；饥饿可能 → 后续可加 aging（不在 Phase5 强制）

## 禁止

- 禁止多车协调 = 距离判碰
- 禁止破坏 Phase3 时间窗口质量且无 ADR/回归

## 参考

- Sharon et al., CBS, AI Journal 2015
- ADR-003、ADR-008
- `docs/PHASE5_GOAL_PROMPT.md` §4.3 / §5
