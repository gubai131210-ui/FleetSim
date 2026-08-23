# ADR 014: 线性 MPC 横向跟踪

## 状态

已接受（2026-08-23，Phase 6 Session 0 草案；Session 1 实装线性化 + Eigen 稠密箱约束 QP；**Session 2 引擎/Dialog 接线 `tracker=mpc`**）

## 背景

Phase 5 已有 `PurePursuitTracker` 与 `StanleyTracker`（几何反馈）。教学上需要**预测时域 + 约束优化**的横向控制，对应 Autoware MPC Lateral 的简化版。Phase 6 引入线性误差动力学 + 有限时域 QP，与 Stanley/PP **可切换对比**。

## 决策

### 1. 接口

```cpp
class MpcLateralTracker : public IPathTracker {
public:
    // horizon, dt, q_lat, q_heading, r_steer, max_steer, wheelbase, cruise_speed
    ControlCommand compute(const Pose& current_pose,
                           const Path& reference_path,
                           double dt) const override;

    void setSpeedProfile(const core::SpeedProfile* profile);  // nullptr = clear
    bool lastSolveOk() const;
};
```

- **保持** `IPathTracker::compute(Pose, Path, dt)` 签名不变（禁止改虚函数签名）。
- 实现类：`domain::control::MpcLateralTracker`。
- **禁止** Stanley/PP 公式换皮；必须有预测时域与非平凡代价/矩阵。

### 2. 误差模型与 QP（Session 1）

- 状态：横向误差 \(e\)、航向误差 \(\theta_e\)（可选扩展舵角 \(\delta\)）。
- 沿参考轨迹将自行车运动学（ADR-009）**线性化** → \(x_{k+1}=A x_k + B u_k\)。
- 有限时域 \(N\) 最小化 \(\sum (e^\top Q e + u^\top R u)\) + 可选终端项；箱约束 \(\delta_{\min}\le u \le \delta_{\max}\)。
- **求解器写死**：**Eigen 稠密 QP**（箱约束投影 / 二次惩罚法）。教学默认 `horizon=10`，硬上限 \(N\le 15\sim20\)；更大 horizon 必须有失败/超时路径。
- **暂不引入 OSQP / osqp-eigen**（避免 EXPORT/FetchContent 重蹈 nlohmann 覆辙）。若未来引入：仅 `BUILD_INTERFACE` + Config `find_dependency`。
- Receding horizon：每周期只施加 \(u_0^*\)。

### 3. ControlCommand 填充

- Bicycle：`linear_velocity` + `steering_angle`（限幅）；`angular_velocity` 可置 0。
- DiffDrive：若显式选 mpc，可将 \(\delta\) 经 \(\omega \approx (v/L)\tan\delta\) 填入（与 Stanley 对称；默认 DiffDrive 仍用 PP）。

### 4. 目标纵向速度（写死）

- **不**扩展 `compute` 入参。
- Tracker 持有 `setSpeedProfile(const SpeedProfile*)`；`compute` 内按路径最近点索引取 `speeds[i]`。
- 无剖面 / 空指针 / 越界 → 使用构造参数 `cruise_speed`。
- SimEngine 在 `speed_planner=st_graph` 接线后调用 `setSpeedProfile`（Session 2/4）；本 ADR 锁定约定，Session 0 不改引擎。

### 5. QP 失败回退（写死）

- QP 失败或 horizon 非法 → **`steering_angle = 0`**，`linear_velocity` 仍取目标速度（剖面或 cruise）；`lastSolveOk() == false`。
- **禁止**静默用 Stanley 公式冒充「求解成功」（假 MPC）。

### 6. Scenario / 默认

| tracker 字段 | 用途 |
|--------------|------|
| `pure_pursuit` | 默认 DiffDrive；bicycle 可选 |
| `stanley` | bicycle 推荐默认（Phase 5） |
| `mpc` | **显式选择**；不默认替换 stanley |

默认保持 Phase 5：`auto` → PP；bicycle 场景可 stanley；**mpc 须显式选**。

### 7. 单测最低要求（Session 1+）

- 直道：横偏 \(|e|\) 下降趋势
- 限舵：\(|\delta| \le \delta_{\max}\)
- 低速：无 NaN
- **断言**预测矩阵 A/B 或代价权重非平凡（权重全 0 → 行为退化须被 M31/M33 捕获）
- QP 失败路径 → 零舵 + `lastSolveOk()==false`
- 与 Stanley 可区分（`MpcVsStanleyCompareTest`，Session 2）

## 后果

- 正面：可对比几何跟踪 vs 预测优化；零新依赖（Eigen 已有）
- 负面：线性化在大横偏/大曲率下近似变差；须限舵与调 R

## 禁止

- 禁止 MPC = Stanley/PP 换皮或投影梯度换皮
- 禁止 horizon>30 却无失败路径
- 禁止 Domain 引入 Qt；禁止削 CMake / 删 Eigen 另起炉灶
- 禁止 EXPORT 直接 PUBLIC 链未导出 FetchContent（若日后 OSQP）

## 参考

- Autoware MPC Lateral / Algorithm 文档
- ADR-009、ADR-012
- `docs/PHASE6_GOAL_PROMPT.md` §4.1 / §5
