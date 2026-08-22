# ADR 004: 车辆 2D 渲染 — SVG + QGraphicsView

## 状态

已接受（2026-08-22）

## 背景

用户希望 Agent 能绘制可交互的小车 2D 模型。需在「Agent 可生成资源」「可缩放」「可交互」「与 Qt Widgets 架构一致」之间权衡。

## 候选方案

| 方案 | Agent 友好 | 可缩放 | 交互 | 推荐 |
|------|-----------|--------|------|------|
| **SVG 文件 + QGraphicsSvgItem** | ⭐⭐⭐⭐⭐ Agent 直接写 SVG | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ QGraphicsScene 原生 | ✅ **主方案** |
| QPainter 代码绘制 | ⭐⭐ 需写 C++ 绘图逻辑 | ⭐⭐⭐ | ⭐⭐⭐ | 仅作 fallback |
| PNG 精灵图 | ⭐⭐⭐ | ⭐⭐ 多分辨率 | ⭐⭐⭐ | ❌ |
| QML Canvas | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ❌ 与 Widgets 主架构分裂 |

## 决策

**主方案：SVG 资源 + QGraphicsView 图元封装**

### 资源目录

```
assets/vehicles/
├── agv_diff.svg      # 差速 AGV（Phase 1 默认）
├── agv_bicycle.svg   # 自行车模型外观（Phase 4）
└── README.md         # SVG 规范：视口 1m×0.6m，原点在车辆中心
```

### 代码结构（一文件一职责）

```
src/ui/graphics/
├── VehicleGraphicsItem.h/cpp   # 单车图元：加载 SVG、位姿变换、选中/悬停
├── ObstacleGraphicsItem.h/cpp  # Phase 2：障碍物图元
├── PathGraphicsItem.h/cpp      # Phase 1：路径折线/样条

src/ui/map/
├── MapScene.h/cpp              # 场景管理：图层、坐标变换
└── MapView.h/cpp               # QGraphicsView 封装：缩放、平移、编辑模式
```

### VehicleGraphicsItem 职责

1. 从 `scenario.json` 中的 `svg` 字段加载路径
2. 使用 `QGraphicsSvgItem` 渲染（需 Qt6 Svg 模块）
3. `setPose(x, y, theta)` 更新 `QTransform`（**不在 Domain 层**）
4. 交互：`ItemIsSelectable`、点击发射 `selected(id)` 信号（Phase 2+）
5. SVG 缺失时 fallback：简单矩形 + 方向箭头（QPainter）

### Agent 绘制 SVG 规范

- viewBox 建议 `"0 0 100 60"`（逻辑单位），代码中按 `vehicle_length_m` 缩放
- 车辆前进方向 = **SVG +X 轴**
- 使用简单 path/rect/circle，避免嵌入 bitmap
- 每辆车可独立 SVG，便于多 AGV 配色区分（Phase 3）

### 与 Domain 层边界

| 层 | 职责 |
|----|------|
| Domain | 只输出 `Pose`、`VehicleId` 等数值状态 |
| App / SimController | 把 Domain 状态同步到 UI |
| UI / VehicleGraphicsItem | SVG 渲染、交互、动画插值（可选） |

**Domain 层不得解析或依赖 SVG。**

## 后果

- Agent 可通过编辑/生成 SVG 文件定制车辆外观，无需改 C++ 绘图代码
- 依赖 Qt6 Svg 模块（`find_package(Qt6 Svg)`）
- 复杂动画（轮子的旋转）可在 SVG 内用 `<animateTransform>` 或 Phase 2+ 在 Item 内细分子图层

## 备选（未采用）

- **Spine / Lottie**：过重，不适合工程仿真工具
- **纯 QML**：与已选 Widgets 主框架不一致
