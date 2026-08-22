# ADR 012: Stanley 路径跟踪器

## 状态

已接受（2026-08-23，Phase 5 Session 0 草案；Session 3 实装）

## 背景

Phase 1–4 跟踪器为 `PurePursuitTracker`（后轴 + 前瞻点）。自行车场景需要更直接的横向误差反馈。Phase 5 引入经典 Stanley，与 PP 可切换。

## 决策

### 1. 接口

```cpp
class StanleyTracker : public IPathTracker {
public:
    // k_gain, softening_v (ε), max_steer, wheelbase（前轴投影）
    ControlCommand compute(const Pose& current_pose,
                           const Path& reference_path,
                           double dt) const override;
};
```

- 构造参数默认建议：`k_gain=1.5`，`softening_v=0.1`，`max_steering_rad=0.6`，`wheelbase_m=0.8`，`v_cruise=0.5`
- 实现 **经典公式**（禁止 Pure Pursuit 换皮）：

\[
\delta = \theta_e + \arctan\left(\frac{k\, e}{v + \varepsilon}\right)
\]

其中：
- \(e\)：前轴位置相对路径最近点的**横向误差**（有符号）
- \(\theta_e\)：航向误差（车头相对路径切线）
- \(v\)：期望/当前纵向速度；\(\varepsilon=\) `softening_v` 防止除零

### 2. ControlCommand 填充

- Bicycle：填 `linear_velocity` + `steering_angle`（限幅到 `max_steering_rad`）；`angular_velocity` 可置 0
- DiffDrive：若选用 Stanley，可将 \(\delta\) 经 \(ω ≈ (v/L)\tan\delta\) 填入 `angular_velocity`（Session 3 明确；默认 DiffDrive 仍用 Pure Pursuit）

### 3. 与 Pure Pursuit 并存

| tracker 字段 | 用途 |
|--------------|------|
| `pure_pursuit` | 默认 DiffDrive；bicycle 可选 |
| `stanley` | 推荐 bicycle |

- SimEngine / Vehicle：**按配置注入** tracker（Session 3），禁止永久硬编码仅 PP
- 不删除 `PurePursuitTracker` / `SteeringAdapter`

### 4. 路径朝向

- `Path` 无 θ：切线由相邻 waypoint 差分得到；单点路径返回停车命令
- 前轴位置：`rear + (L cosθ, L sinθ)`

### 5. 单测最低要求

- 直道：横偏 \(|e|\) 随时间下降趋势
- \(v=0\)：有 ε，无 NaN
- 限舵：\(|\delta| \le \delta_{\max}\)
- 禁止与 PP 输出恒等（换皮检测：构造大横偏小航向误差时 Stanley 应含 \(\arctan(ke/v)\) 项）

## 后果

- 正面：可对比实验台；横偏收敛通常快于 PP
- 负面：折点路径可能振荡 → 依赖 Hybrid 稠密输出或限舵

## 禁止

- 禁止 Stanley = PurePursuit 改名或仅改 lookahead
- 禁止忽略前轴投影只用后轴误差

## 参考

- CMU Snider 技术报告（Automatic Steering Methods）
- ADR-009（ControlCommand / Bicycle）
- `docs/PHASE5_GOAL_PROMPT.md` §4.2
