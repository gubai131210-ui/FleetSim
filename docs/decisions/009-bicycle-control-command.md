# ADR 009: Bicycle 运动学模型与 ControlCommand 扩展

## 状态

已接受（2026-08-23，Phase 4 Session 0 草案；Session 1+ 实装）

## 背景

Phase 1–3 车辆死绑 `DiffDriveModel`（`v` + `ω`）。Phase 4 需要可替换的运动学自行车模型，并保持 DiffDrive 旧路径与单测不破。

## 决策

### 1. ControlCommand 兼容扩展

```cpp
struct ControlCommand {
    double linear_velocity{0.0};
    double angular_velocity{0.0};
    double steering_angle{0.0};  // rad；默认 0
};
```

- DiffDrive：**忽略** `steering_angle`，继续用 `angular_velocity`
- Bicycle：使用 `linear_velocity` + `steering_angle`；可忽略 `angular_velocity`

### 2. IVehicleModel 策略接口

```cpp
class IVehicleModel {
public:
    virtual ~IVehicleModel() = default;
    virtual Pose integrate(const Pose& pose,
                           const ControlCommand& cmd,
                           double dt) const = 0;
};
```

- `DiffDriveModel` / `BicycleModel` 均实现该接口
- `Vehicle`（Session 1）持有 `unique_ptr<IVehicleModel>`，按 scenario `model` 工厂创建

### 3. Bicycle 后轴参考运动学（Session 1 实装）

\[
\dot{x}=v\cos\theta,\quad
\dot{y}=v\sin\theta,\quad
\dot{\theta}=\frac{v}{L}\tan\delta
\]

| 参数 | 默认 | 说明 |
|------|------|------|
| `wheelbase_m` (L) | 0.8–1.0 | 轴距；≤0 防护 |
| `max_steering_rad` | ~0.6 | 舵角限幅 |
| `max_linear_velocity` | 0.5 | 与 DiffDrive 同量级 |

**禁止**：把 Bicycle 写成 DiffDrive 换皮（禁止用 `ω` 代替 `v/L tanδ`）。

### 4. 跟踪适配（Session 2）

Pure Pursuit 已算曲率 `κ=2 sinα / ℓ_d`。Bicycle 路径：

\[
\delta = \arctan(κ L)
\]

再写入 `ControlCommand.steering_angle`（限幅）。Stanley 可选，不阻塞 MVP。

### 5. Scenario

`vehicles[].model`: `"diff_drive"` | `"bicycle"`；bicycle 可带 `wheelbase_m`（Session 2 资产）。

## 后果

- 正面：模型可切换；旧测默认 `steering_angle=0` 不受影响
- 负面：Tracker 需按模型填不同控制字段；须集成测覆盖转弯方向

## 参考

- ArXiv 1803.03758（后轴自行车）
- CMU Snider：Pure Pursuit / Stanley
- `docs/PHASE4_GOAL_PROMPT.md` §4–§5
