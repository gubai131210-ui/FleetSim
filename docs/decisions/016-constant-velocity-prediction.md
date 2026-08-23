# ADR 016: 常速轨迹预测 + ST-Graph 增强

## 状态

已接受（2026-08-23，Phase 7 Session 0 草案；Session 1–2 实装 CV + SimEngine 接线）

## 背景

Phase 6 的 `StGraphSpeedPlanner` 假设他车 `reference_path` **静态已知**（ADR-015）。工业 ST 图通常先对他车做短 horizon 轨迹预测，再投影到 \((s,t)\) 占用块。Phase 7 引入教学向 **Constant-Velocity (CV)** 预测，闭合「预测 → ST」闭环，**不**引入神经网络或感知栈。

## 决策

### 1. 预测接口（写死）

```cpp
namespace fleetsim::domain::prediction {

class IPeerPredictor {
public:
    virtual ~IPeerPredictor() = default;
    virtual core::Path predictPath(const core::Pose& current,
                                   double nominal_speed_mps,
                                   double horizon_s,
                                   double sample_dt_s) const = 0;
};

class ConstantVelocityPredictor : public IPeerPredictor {
public:
    core::Path predictPath(const core::Pose& current,
                           double nominal_speed_mps,
                           double horizon_s,
                           double sample_dt_s) const override;
};

}  // namespace fleetsim::domain::prediction
```

**外推公式（写死）**：沿当前航向 \(\theta\) 以名义速度 \(v\) 前进，采样间隔 \(\Delta t\)：

\[
x_k = x_0 + v \cos\theta \cdot k \Delta t,\quad
y_k = y_0 + v \sin\theta \cdot k \Delta t,\quad
k = 0 \ldots \lfloor T / \Delta t \rfloor
\]

- 默认 `horizon_s = 3.0`，`sample_dt_s = 0.1`（与 ST `dt_grid` 对齐，见 ADR-015）。
- 输出 `core::Path` 稠密折线，供 `StGraphSpeedPlanner::buildOccupancies` 投影。
- **禁止**：把静态 `reference_path` 原样复制并改名 `predicted_path`。
- **禁止**：神经网络 / 多模态 / 感知输入。

### 2. `PeerTrajectory` 扩展

```cpp
struct PeerTrajectory {
    core::Path path;              // ST 投影用几何路径
    double nominal_speed{0.5};
    bool from_prediction{false};  // 诊断/测试：区分静态 vs 预测来源
};
```

- `prediction=none`：`path = peer.reference_path`（Phase 6 行为，默认）。
- `prediction=constant_velocity`：`path = predictor.predictPath(peer.pose(), nominal_speed, …)`；**禁止**仍读 `reference_path` 却标 prediction 开。

### 3. SimEngine / scenario 字段

| 字段 | 值 | 行为 |
|------|-----|------|
| `simulation.prediction` | `none`（默认） | Phase 6 回归：`collectPeersFor` 读静态 Path |
| | `constant_velocity` | `collectPeersFor` 用 `ConstantVelocityPredictor` 生成 peer.path |

- 与 `speed_planner=st_graph` 组合时才有 ST 效果；prediction 单独开但 ST 关 → 仅记录字段，不影响剖面（可接受）。
- `setPredictionKind(const std::string&)` / `predictionKind()` 与 scenario 序列化对齐（Session 2+）。

### 4. 单测合同（Session 0 红灯 → Session 1–2 转绿）

| 测试 | 断言 |
|------|------|
| `ConstantVelocityPredictorTest` | 直线 CV：路径长度 \(\approx v \cdot T\)；点数 \(\approx T/\Delta t + 1\) |
| `StGraphWithPredictionTest` | 同一 peer：关 prediction（静态短 path）vs 开 CV（外推 path）→ ST 剖面 **可区分** |
| `StGraphSimEngineWiringTest` 扩展 | `prediction=constant_velocity` 时 `collectPeers` 的 path 来自外推，非 reference_path 拷贝 |

### 5. 默认参数

| 参数 | 默认 |
|------|------|
| prediction horizon | 3.0 s |
| prediction sample_dt | 0.1 s |
| simulation.prediction | none |

### 6. 明确不做

- 神经网络 / 行为预测网络
- 激光 / 相机 / 占用栅格感知
- 多模态轨迹集合
- LaneGraph 路由预测
- 完整 Apollo EM Planner 栈

## 后果

- ST 动态障碍教学闭环；默认 `none` 保 Phase 6 104 测回归。
- Domain 新增 `prediction/` 子目录；零 Qt / 零 rclcpp。

## 参考

- ADR-015 ST-Graph
- Phase 7 `docs/PHASE7_GOAL_PROMPT.md` §4.2、§5
- Apollo EM Planner ST 动态障碍 CV 投影（教学简化）
