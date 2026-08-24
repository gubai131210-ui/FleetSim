# ADR 024: CBS-lite 约束树多车协调

## 状态

草案（2026-08-24，Phase 10 Session 0；Session 5 实装 `CbsLiteCoordinator`）

## 背景

ADR-013 实装 `PriorityPathCoordinator`（按优先级依次规划 + 预约走廊）。CBS（Conflict-Based Search）通过**约束树**在冲突点分支、底层带约束重规划消解多智能体路径冲突。Phase 10 实装 **depth-bounded CBS-lite** 教学 MVP，**非**完整 EECBS。

## 决策

### 1. coordination 字段

```json
"simulation": {
  "coordination": "priority",
  "cbs_max_depth": 10,
  "cbs_time_limit_ms": 100
}
```

| 字段 | 默认 | 说明 |
|------|------|------|
| `coordination` | `priority` | `none` \| `priority` \| **`cbs_lite`** |
| `cbs_max_depth` | 10 | 约束树最大扩展深度 |
| `cbs_time_limit_ms` | 100 | 高层搜索时间上限（ms） |

**默认 `priority`** 保 Phase 5–9 回归。

### 2. 与 Priority / TimeWindow 边界

| 组件 | 职责 |
|------|------|
| `PriorityPathCoordinator` | 按优先级占走廊（plan 前） |
| **`CbsLiteCoordinator`** | 检测路径时空冲突 → 约束 → 底层 A* 重规划（bounded） |
| `TimeWindowCollisionAvoidance` | tick 内速度缩放（**保留**，不删除） |

`coordination=cbs_lite` 时 SimEngine 在 multi-agent plan 阶段调用 CBS-lite；失败可回退 priority 或返回 plan 失败（实现选一种，测例锁死）。

### 3. Domain API（`collision/CbsLiteCoordinator.*`）

最低真实现（Session 5）：

1. **冲突检测**：多 agent 路径在离散时间步上的点冲突或边冲突（时空格）
2. **约束生成**：`agent_i` 禁止在时刻 `t` 占用格 `c`（或边）
3. **底层重规划**：对冲突 agent 在约束下重跑既有 A* / planner
4. **有界搜索**：`cbs_max_depth` / `cbs_time_limit_ms` 上限；超限失败

可观测输出：`constraints_added`、`nodes_expanded`、路径与 Priority 模式可区分。

### 4. UI

- **CoordinationPage** 扩展：`coordination=cbs_lite` + depth/time_limit 绑定
- **禁止**新建「CBS+BT+OSM 全家桶」超级页
- **禁止**在 RoutingPage 出现 `cbs_lite`

### 5. 测试与变异

| 测例 | 最低断言 |
|------|----------|
| `CbsLiteCoordinatorTest` | 对撞场景产生约束并重规划；depth=0 立即失败/回退 |
| M46 | 忽略冲突恒返回原路径 → FAIL |

## 后果

- 正面：教学 MAPF 约束树思想；与 Priority 可切换对比
- 负面：非最优；大规模组合爆炸需 depth 限制

## 禁止

- 禁止 CBS-lite = Priority 换名或仅距离减速
- 禁止静默删除 TimeWindow 且无 ADR/回归
- 禁止完整 EECBS / symmetry reasoning（→ Phase 11）

## 参考

- ADR-013、ADR-003
- Sharon et al., CBS, AI 2015
- `docs/PHASE10_GOAL_PROMPT.md` §4.4
