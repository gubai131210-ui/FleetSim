# ADR 020: Behavior Tree MVP + JSON Schema

## 状态

已接受（2026-08-23，Phase 9 Session 0 草案；Session 1–3 实装控制/叶节点/引擎接线；Session 6 终审）

## 背景

Phase 8 完成 LaneGraph + hybrid routing + LaneEditorPanel / RoutingPage。SimEngine 内导航子流程仍由硬编码 `needs_replan` + `AgentPhase` 驱动。工业 Nav2 栈使用 Behavior Tree 编排 **何时 plan / follow / replan / recovery**。Phase 9 在 Domain 层自研**教学 MVP**（零 Qt、零 BehaviorTree.CPP 依赖），与 Phase 5–8 planner/tracker/ST/routing **并存、可切换**。

## 决策

### 1. simulation 字段（scenario.json）

```json
"simulation": {
  "behavior_mode": "legacy",
  "behavior_tree_path": "navigate_replan_recovery.json",
  "replan_hz": 1.0,
  "recovery_wait_ticks": 20
}
```

| 字段 | 默认 | 说明 |
|------|------|------|
| `behavior_mode` | `legacy` | `legacy` 保留 Phase 8 行为；`bt` 启用 BtNavigator |
| `behavior_tree_path` | 空 | 相对场景目录或 `assets/behavior_trees/` |
| `replan_hz` | `1.0` | Rate 装饰器频率，对齐 Nav2 RateController |
| `recovery_wait_ticks` | `20` | BtWaitRecovery 等待 tick 数 |

### 2. Domain 模块（`src/domain/behavior/`）

| 文件 | 职责 |
|------|------|
| `BtTypes.h` | `NodeStatus`（SUCCESS / FAILURE / RUNNING）、`BtNodeType` |
| `BtBlackboard.h` | 键值共享状态（agent_id、goal、path_valid、replan_requested…） |
| `BtNode.h` | 抽象 `tick(BtBlackboard&)` |
| `BtControlNodes.*` | Sequence、Fallback、Recovery（Nav2 语义） |
| `BtDecoratorNodes.*` | RateTick（按 hz 限频 tick 子节点） |
| `BtFleetActionNodes.*` | PlanPath、FollowUntilGoal、WaitRecovery、ReplanIfTimer |
| `BtFleetConditionNodes.*` | IsGoalUpdated、IsPathValid、YieldIfBlocked（可选） |
| `BtTreeLoader.*` | JSON → 内存树 |
| `BtNavigator.*` | 持有树 + blackboard + 每 tick 入口 |

**硬约束**：Domain 禁止 `#include <Qt*>` 与 rclcpp。

### 3. JSON 树 schema（MVP version 1，写死）

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

**节点 type 枚举（MVP）**：`Sequence` | `Fallback` | `Recovery` | `Rate` | `Action` | `Condition`

**Action 名（FleetSim 叶）**：`PlanPath` | `FollowUntilGoal` | `WaitRecovery` | `ReplanIfTimer`

**Condition 名**：`IsGoalUpdated` | `IsPathValid` | `YieldIfBlocked`

### 4. Recovery 语义（对齐 Nav2 RecoveryNode）

1. **第一个 child** = 主行为（Navigation subtree）。
2. **第二个 child** = recovery 行为。
3. 仅当第一个 child 返回 **FAILURE** 时 tick 第二个 child。
4. 第二个 child **SUCCESS** 后重试第一个 child（消耗一次 retry）。
5. 超过 `retries` 返回 **FAILURE**。
6. **禁止**先 tick recovery 再 tick 主行为（M40 变异须 FAIL）。

### 5. 控制节点语义

| 节点 | 行为 |
|------|------|
| Sequence | 顺序 tick 子节点；遇 FAILURE/RUNNING 即返回；全 SUCCESS → SUCCESS |
| Fallback | 顺序 tick；遇 SUCCESS 即返回；遇 RUNNING 即返回；全 FAILURE → FAILURE |
| Rate | 按 `1/hz` 仿真时间限频 tick 子节点；间隔内返回上次状态或 SUCCESS |

### 6. SimEngine 集成（Session 3）

- `behavior_mode=legacy`：现有 `AgentPhase` + `needs_replan` **不变**。
- `behavior_mode=bt`：每 SimEngine tick 对启用 BT 的 agent 调用 `BtNavigator::tick()`。
- BT **编排** plan/follow/replan/recovery **时序**；**不**替代 A*、LaneGraph、MPC、ST。

### 7. 明确不做

- 完整 BehaviorTree.CPP / Nav2 bt_navigator 移植
- BT XML Nav2 全节点兼容
- Spin / BackUp / ClearCostmap 真运动 recovery
- 多车每 agent 独立 BT 森林（MVP 先单车）

## 后果

- 新增 `domain/behavior/` 源文件须登记 `FleetSimDomain` CMake
- 每个新 Domain 类须有 GTest（`BehaviorTreeTest`、后续 `BtNavigationIntegrationTest`）
- `verify_phase9_evidence.py` Session 6 静态审计 ADR-020 与源文件

## 参考

- [Nav2 BT Walkthrough](https://docs.nav2.org/behavior_trees/overview/detailed_behavior_tree_walkthrough.html)
- [Nav2 RecoveryNode](https://docs.nav2.org/configuration/packages/bt-plugins/controls/RecoveryNode.html)
- Phase 9：`docs/PHASE9_GOAL_PROMPT.md` §4–§5
