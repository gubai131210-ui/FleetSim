# ADR 023: BT XML 子集 + 运动 Recovery 叶节点

## 状态

草案（2026-08-24，Phase 10 Session 0；Session 2–3 实装）

## 背景

Phase 9 以 JSON schema 加载行为树（`BtTreeLoader`）。Nav2 / BehaviorTree.CPP 工业栈使用 **XML** 描述树，Behavior Server 提供 Spin / BackUp / ClearCostmap 等运动恢复。Phase 10 扩展 Domain 自研 BT 引擎，**不**引入 BehaviorTree.CPP 依赖。

## 决策

### 1. simulation 字段扩展

```json
"simulation": {
  "behavior_mode": "bt",
  "bt_format": "json",
  "behavior_tree_path": "navigate_spin_backup_recovery.xml",
  "spin_rad": 1.5708,
  "backup_dist_m": 0.3,
  "backup_speed_mps": 0.1
}
```

| 字段 | 默认 | 说明 |
|------|------|------|
| `bt_format` | `json` | `json` 保留 Phase 9；`xml` 启用 `BtXmlLoader` |
| `spin_rad` | π/2 | BtSpinRecovery 目标角（rad，有符号） |
| `backup_dist_m` | 0.3 | BtBackUpRecovery 倒退距离 |
| `backup_speed_mps` | 0.1 | 倒退线速度 |

### 2. BtXmlLoader（`behavior/BtXmlLoader.*`）

- 根：`<root main_tree_to_execute="MainTree">` + `<BehaviorTree ID="MainTree">`
- 白名单控制节点：`Sequence`、`Fallback`、`RecoveryNode`、`RateController`（或 `Rate`）、`RoundRobin`、`ReactiveFallback`
- Action/Condition 映射到既有 FleetSim 叶节点名：`PlanPath`、`FollowUntilGoal`、`WaitRecovery`、`Spin`、`BackUp`、`ClearInflation`、`ReplanIfTimer`、`IsGoalUpdated`、`IsPathValid`
- 未知标签 → 加载失败（禁止空壳恒 SUCCESS）
- 与 `BtTreeLoader` **并存**；由 `bt_format` 切换

### 3. 控制节点扩展（`BtControlNodes.*`）

| 节点 | 语义 |
|------|------|
| `BtRoundRobinNode` | 子节点轮转；任一 SUCCESS → SUCCESS；全 FAILURE → FAILURE |
| `BtReactiveFallbackNode` | 每 tick 从第一个子节点重评估；高优先级条件 SUCCESS 可 **打断** RUNNING 异步子节点（与 Fallback 可测区分） |

### 4. 运动 Recovery 叶节点（`BtMotionRecoveryNodes.*`）

| 节点 | 行为 | 可测副作用 |
|------|------|-----------|
| `BtSpinRecovery` | 原地旋转至目标 yaw（DiffDrive/Bicycle 积分 ω） | `|Δyaw| ≥ ε` |
| `BtBackUpRecovery` | 沿车体后方直线倒退 `backup_dist_m` | 位移沿 -heading ≥ ε |
| `BtClearInflation` | 教学 ClearCostmap：临时清除/重置 inflation 层或标记 replan 前清障碍缓存 | inflation 占用减少或 replan 成功 |

**禁止**：仅用 `BtWaitRecovery` sleep 换皮；禁止空函数声称 ClearCostmap。

### 5. UI 分层

- **BehaviorXmlPage**（Workbench Tab 8）：`bt_format`、xml/json 路径、RoundRobin/Recovery 教学参数
- **禁止**在 BehaviorPage（Tab 6）堆 XML / Spin / BackUp 参数

### 6. 测试与变异

| 测例 | 最低断言 |
|------|----------|
| `BtXmlLoaderTest` | 加载 Nav2 风格 XML；未知标签失败；RoundRobin 轮转可测 |
| `BtMotionRecoveryTest` | Spin/BackUp pose 变化；ClearInflation 副作用 |
| M44 | Spin 不改 yaw → FAIL |
| M45 | RoundRobin 恒 tick 第一个 child → FAIL |

## 后果

- 正面：Nav2 XML 资产可复用（子集）；Recovery 可观测
- 负面：非完整 BT.CPP 兼容；PipelineSequence 可选延后

## 禁止

- 禁止引入 BehaviorTree.CPP / Groot2
- 禁止 ReactiveFallback 与 Fallback 同一实现改名
- 禁止 XML loader 忽略子节点顺序

## 参考

- ADR-020
- Nav2 BT XML / Behavior Server 文档
- `docs/PHASE10_GOAL_PROMPT.md` §4.2–§4.3
