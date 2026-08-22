# Vehicle SVG Assets

Agent 可编辑此目录下的 SVG 文件来定制车辆 2D 外观，无需修改 C++ 绘图代码。

## 规范（ADR-004）

| 属性 | 值 |
|------|-----|
| viewBox | `0 0 100 60`（逻辑单位） |
| 前进方向 | **+X 轴**（箭头指向右） |
| 原点 | 车辆中心 ≈ `(50, 30)` |
| 缩放 | 代码按 `vehicle_length_m / 100` 缩放到米制 |

## 文件

| 文件 | 用途 |
|------|------|
| `agv_diff.svg` | 差速 AGV（Phase 1 默认） |
| `agv_bicycle.svg` | 自行车模型外观（Phase 4，待 Agent 绘制） |

## Agent 绘制提示

- 使用 `<rect>`, `<polygon>`, `<circle>`, `<path>` 即可
- 避免嵌入 bitmap
- 多 AGV 时可复制 SVG 并改配色区分 ID
