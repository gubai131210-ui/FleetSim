# ADR 003: 多车避碰与任务调度

## 状态

已实现（2026-08-22，Phase 3）

## 决策

### 避碰：时间窗口预约 + 优先级让行（混合）

```cpp
class ICollisionAvoidance {
    virtual void reservePath(AGVId id, const Path& path, TimeWindow window) = 0;
    virtual Velocity adjustVelocity(AGVId id, const AGVState& state,
                                    const std::vector<AGVState>& nearby) = 0;
};
```

Phase 0–2：`CollisionModule` stub 返回空操作。

### 调度：ITaskAssigner 插件式

- Phase 3 MVP：`GreedyAssigner`（最近车接最近任务）
- Phase 3+：`HungarianAssigner`

## 后果

- Phase 1–2 单车不受接口 stub 影响
- Phase 3 在既有接口上填充实现
