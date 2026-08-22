# Agent 会话交付模板（强制）

每次 Agent 会话结束前，**Scribe 角色**必须更新 `SESSION_LOG.md`，**完整填写**以下全部章节。缺一 = 会话未完成。

同时，Agent 在**回复用户**时，必须包含同样的「本次交付摘要」，让用户无需读 diff 也能知道做了什么、没做什么。

---

## 会话条目模板（复制到 SESSION_LOG.md）

```markdown
## [YYYY-MM-DD] Phase X — <简短标题>

### 本次 Scope（Architect 定义）
- 目标：
- 允许改动的目录/文件：
- 明确不在本次范围：

### ✅ 已完成（必须逐条列出，禁止笼统写「完成了 Phase X」）
- [ ] 具体文件/类/功能 1
- [ ] 具体文件/类/功能 2
- ...

### ❌ 未完成 / 故意不做（必须写原因）
| 项目 | 原因 | 计划在哪个 Phase/会话 |
|------|------|----------------------|
| 例：MonitorPanel | 不在 Phase 1 scope | Phase 2 |
| 例：地图编辑器 | 依赖 OccupancyGrid 未完成 | Phase 2 会话 2 |

### 🚫 禁止偷懒自检（Reviewer 逐条打勾）
- [ ] 没有把多个类挤进同一文件
- [ ] 没有在 MainWindow 堆业务按钮/控件
- [ ] 新 UI 功能使用了独立 Panel/Dialog 文件
- [ ] Domain 层无 Qt include
- [ ] 新 Domain/Core 类有对应单测
- [ ] 未跨 Phase 实现功能
- [ ] SESSION_LOG 本节已完整填写

### 新增/变更文件清单
| 文件 | 操作 | 说明 |
|------|------|------|
| `src/...` | 新增/修改 | |

### 接口变更
- （无则写「无」）

### Reviewer 结果
- PASS / FAIL + 问题列表

### 用户本地验证
- 操作步骤：
- 预期结果：

### 下次会话建议
- 第一条任务：
- 前置条件：
```

---

## 回复用户时的强制格式

Agent 最终回复必须包含以下四段标题（不可省略）：

### 1. 本次做了什么
逐条 bullet，精确到文件名。

### 2. 本次没做什么
逐条 bullet，每条附带原因和计划时间。

### 3. 为什么这样拆分
1–3 句话说明架构/UI 决策，特别是为何新建 Panel 而非堆控件。

### 4. 请你本地验证
给出 Qt Creator 操作步骤，不代跑编译。

---

## 偷懒行为清单（Reviewer 发现即 FAIL）

| # | 偷懒行为 | 正确做法 |
|---|---------|---------|
| 1 | SESSION_LOG 只写「完成了功能」 | 按模板逐条列出 |
| 2 | 「没做什么」留空 | 必须填写，即使全是「不在 scope」 |
| 3 | 在 MainWindow 加 QPushButton 实现新功能 | 新建 XxxPanel |
| 4 | 用 QToolBar 代替面板 | 用 QDockWidget + Panel |
| 5 | 把监控曲线塞进 ControlPanel | 新建 MonitorPanel |
| 6 | 一个 .cpp 超过 500 行不拆分 | 拆文件 |
| 7 | 不写单测说「Phase X 再补」 | Core/Domain 新类当下就写 |
| 8 | 跨 Phase 提前实现 | 只写 stub/ADR，不写逻辑 |
| 9 | 用户问 UI 时用 placeholder 糊弄 | 新建面板文件，哪怕最小可用 |
| 10 | 不更新 CMakeLists 就加新文件 | 同步更新构建 |

---

## Architect mini-plan 模板（会话开头）

```markdown
**Phase**: X
**目标**: （一句话）
**允许改动**: src/domain/map/, tests/domain/
**禁止改动**: src/ui/, MainWindow
**预计新增文件**:
- src/domain/map/OccupancyGrid.h/cpp
- tests/domain/OccupancyGridTest.cpp
**预计 NOT DO**:
- MonitorPanel（Phase 2）
- 地图编辑器 UI（Phase 2）
```
